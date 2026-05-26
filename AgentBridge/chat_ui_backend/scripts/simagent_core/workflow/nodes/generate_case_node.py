from ...manifest.writer import ManifestWriter
from ...models import SimulationPlan, TaskContext
from ...state.task_store import TaskStore
from .input_writer_node import input_writer_node
from .reference_loader_node import reference_loader_node


def generate_case_node(
    task: TaskContext,
    task_store: TaskStore | None = None,
    manifest_writer: ManifestWriter | None = None,
) -> tuple[list[dict], list[dict]]:
    if not task.plan:
        raise ValueError("Task has no plan. Run planner before case generation.")

    plan = SimulationPlan.from_dict(task.plan)
    store = task_store or TaskStore()
    writer = manifest_writer or ManifestWriter()

    reference_files = []
    if plan.generation_mode in {"reference_copy", "reference_modify"}:
        reference_files = reference_loader_node(task, store)

    generated_files = input_writer_node(task, store, writer)
    return reference_files, generated_files
