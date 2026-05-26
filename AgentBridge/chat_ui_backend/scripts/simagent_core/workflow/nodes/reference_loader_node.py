from ...models import SimulationPlan, TaskContext
from ...solvers.openfoam.knowledge import TutorialDetailsDatabase
from ...state.task_store import TaskStore
from ...utils import atomic_write_json
from pathlib import Path


def reference_loader_node(
    task: TaskContext,
    task_store: TaskStore | None = None,
) -> list[dict]:
    if not task.plan:
        raise ValueError("Task has no plan. Run planner before reference loading.")

    store = task_store or TaskStore()
    plan = SimulationPlan.from_dict(task.plan)
    if plan.solver_family != "openfoam":
        raise NotImplementedError("Reference loading is currently implemented for OpenFOAM only.")
    if not plan.reference_case:
        raise ValueError("Plan has no reference_case. Run planner before reference loading.")

    store.update_status(task, "loading_reference")
    reference_file_set = TutorialDetailsDatabase.from_default_data().load_reference_files(plan.reference_case)
    if not task.reference_files_path:
        task.reference_files_path = str(Path(task.task_dir) / "reference_files.json")
    atomic_write_json(Path(task.reference_files_path), reference_file_set.to_dict())
    reference_files = [
        {
            "path": item.path,
            "role": item.role,
            "format": item.format,
            "content_length": len(item.content),
        }
        for item in reference_file_set.files
    ]
    store.attach_reference_files(task, reference_files)
    return reference_files
