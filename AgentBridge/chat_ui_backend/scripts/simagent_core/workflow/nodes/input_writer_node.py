from ...manifest.writer import ManifestWriter
from ...gates.generation_gate import review_generated_files
from ...models import SimulationPlan, TaskContext
from ...state.task_store import TaskStore
from ...services.input_writer import generate_files
from pathlib import Path


def input_writer_node(
    task: TaskContext,
    task_store: TaskStore | None = None,
    manifest_writer: ManifestWriter | None = None,
) -> list[dict]:
    if not task.plan:
        raise ValueError("Task has no plan. Run planner before input_writer.")

    store = task_store or TaskStore()
    writer = manifest_writer or ManifestWriter()
    plan = SimulationPlan.from_dict(task.plan)

    store.update_status(task, "generating_files")
    generated = generate_files(task, plan)
    generated_dicts = [item.to_dict() for item in generated]
    store.attach_gate_review(task, review_generated_files(generated).to_dict(), save=False)
    store.attach_generated_files(task, generated_dicts)
    _remove_reference_files_if_present(task)
    store.mark_reference_files_removed(task, True)
    writer.write_generated(task, plan, generated_dicts)
    return generated_dicts


def _remove_reference_files_if_present(task: TaskContext) -> None:
    reference_files_path = task.effective_reference_files_path()
    if not reference_files_path:
        return
    path = Path(reference_files_path)
    if path.exists():
        path.unlink()
