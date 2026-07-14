from pathlib import Path
import json

from ...models import PlannedFile, ReferenceFile, ReferenceFileSet, SimulationPlan, TaskContext
from ...gates.generation_gate import review_generated_files
from ...spec import SimulationSpec
from .generated.rect_channel import render_rect_channel_case
from .knowledge import TutorialDetailsDatabase
from .knowledge.tutorial_details import is_reference_script_path, reference_file_format
from .mesh import (
    GMSH_MESH_CASE_PATH,
    copy_external_gmsh_mesh,
    external_gmsh_source_path,
    is_block_mesh_dict,
    plan_uses_external_gmsh_mesh,
)
from .modifiers import apply_reference_modifications, merged_requested_changes
from .templates.icofoam_cavity import render_case_files


def generate_openfoam_files(task: TaskContext, plan: SimulationPlan) -> list[PlannedFile]:
    if plan.solver_family != "openfoam":
        raise ValueError(f"OpenFOAM input writer received unsupported solver_family: {plan.solver_family}")

    if plan.generation_mode == "generated_case":
        generated = _generate_from_simulation_spec(task, plan)
        _enforce_generation_gate(generated, plan.planned_files)
        return generated

    if plan.generation_mode in {"reference_copy", "reference_modify"} and plan.reference_case:
        generated = _generate_reference_based_case(task, plan)
        if generated:
            _enforce_generation_gate(generated, plan.planned_files)
            return generated

    generated = _generate_from_cavity_template(task, plan)
    _enforce_generation_gate(generated, plan.planned_files)
    return generated


def _generate_from_simulation_spec(task: TaskContext, plan: SimulationPlan) -> list[PlannedFile]:
    spec_data = plan.parameters.get("simulation_spec", {})
    if not isinstance(spec_data, dict):
        raise ValueError("Generated case plan has no simulation_spec.")

    spec = SimulationSpec.from_dict(spec_data)
    if spec.geometry.type != "rect_channel":
        raise ValueError(f"Unsupported generated geometry type: {spec.geometry.type}")

    generated: list[PlannedFile] = []
    task_dir = Path(task.task_dir)
    for planned_file, content in render_rect_channel_case(spec):
        file_path = task_dir / planned_file.path
        file_path.parent.mkdir(parents=True, exist_ok=True)
        file_path.write_text(content.strip() + "\n", encoding="utf-8")
        generated.append(planned_file)
    return generated


def _generate_reference_based_case(task: TaskContext, plan: SimulationPlan) -> list[PlannedFile]:
    if not plan.reference_case:
        return []

    reference_files = _load_reference_file_set(task, plan)
    generated: list[PlannedFile] = []
    task_dir = Path(task.task_dir)
    changes = _changes_for_reference_generation(plan)
    for reference_file in reference_files.files:
        if _should_skip_planned_path(plan, reference_file.path):
            continue
        generated.append(_write_reference_file(task_dir, reference_file, changes))

    _append_external_gmsh_mesh_if_needed(generated, task_dir, plan)
    return generated


def _changes_for_reference_generation(plan: SimulationPlan) -> dict:
    if plan.generation_mode != "reference_modify":
        return {}
    return merged_requested_changes(plan.parameters.get("requested_changes", {}), plan.requested_changes)


def _load_reference_file_set(task: TaskContext, plan: SimulationPlan) -> ReferenceFileSet:
    reference_files_path = task.effective_reference_files_path()
    if reference_files_path:
        path = Path(reference_files_path)
        if path.exists():
            data = json.loads(path.read_text(encoding="utf-8"))
            return ReferenceFileSet.from_dict(data)

    return TutorialDetailsDatabase.from_default_data().load_reference_files(plan.reference_case)


def _planned_file_from_reference_file(reference_file: ReferenceFile, modifications: list[str]) -> PlannedFile:
    planned_file = PlannedFile(
        role=reference_file.role,
        path=reference_file.path,
        format=reference_file_format(reference_file.path),
        required=True,
    )
    planned_file.source = "reference_modify" if modifications else "reference_copy"
    planned_file.modifications = modifications
    return planned_file


def _write_reference_file(
    task_dir: Path,
    reference_file: ReferenceFile,
    changes: dict,
) -> PlannedFile:
    if is_reference_script_path(reference_file.path):
        content = reference_file.content
        modifications = []
        if not content.strip():
            raise ValueError(f"Reference script content is empty: {reference_file.path}")
    else:
        content, modifications = apply_reference_modifications(reference_file, changes)
    file_path = task_dir / reference_file.path
    file_path.parent.mkdir(parents=True, exist_ok=True)
    file_path.write_text(content if content.endswith("\n") else content + "\n", encoding="utf-8")
    return _planned_file_from_reference_file(reference_file, modifications)


def _generate_from_cavity_template(task: TaskContext, plan: SimulationPlan) -> list[PlannedFile]:
    generated: list[PlannedFile] = []
    task_dir = Path(task.task_dir)
    for planned_file, content in render_case_files(plan.parameters):
        if _should_skip_planned_path(plan, planned_file.path):
            continue
        file_path = task_dir / planned_file.path
        file_path.parent.mkdir(parents=True, exist_ok=True)
        file_path.write_text(content.strip() + "\n", encoding="utf-8")
        generated.append(planned_file)

    _append_external_gmsh_mesh_if_needed(generated, task_dir, plan)
    return generated


def _should_skip_planned_path(plan: SimulationPlan, path: str) -> bool:
    return plan_uses_external_gmsh_mesh(plan.parameters) and is_block_mesh_dict(path)


def _append_external_gmsh_mesh_if_needed(
    generated: list[PlannedFile],
    task_dir: Path,
    plan: SimulationPlan,
) -> None:
    if not plan_uses_external_gmsh_mesh(plan.parameters):
        return

    source_path = external_gmsh_source_path(plan.parameters)
    copy_external_gmsh_mesh(task_dir, source_path)
    generated.append(_external_gmsh_planned_file(source_path))


def _external_gmsh_planned_file(source_path: str) -> PlannedFile:
    return PlannedFile(
        role="mesh_source",
        path=GMSH_MESH_CASE_PATH,
        format="gmsh-msh",
        source=source_path,
        modifications=[],
        required=True,
    )


def _enforce_generation_gate(
    generated: list[PlannedFile],
    planned_files: list[PlannedFile],
) -> None:
    result = review_generated_files(generated, planned_files)
    if result.status == "failed":
        messages = "; ".join(item.message for item in result.issues if item.severity == "error")
        raise ValueError(f"Generated files failed generation gate: {messages}")
