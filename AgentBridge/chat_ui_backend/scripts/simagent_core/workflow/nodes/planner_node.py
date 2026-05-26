from ...config import Config
from ...manifest.writer import ManifestWriter
from ...models import SimulationPlan, TaskContext
from ...state.task_store import TaskStore
from ...services.plan import create_plan


def planner_node(
    task: TaskContext,
    task_store: TaskStore | None = None,
    manifest_writer: ManifestWriter | None = None,
    config: Config | None = None,
) -> SimulationPlan:
    store = task_store or TaskStore(config)
    writer = manifest_writer or ManifestWriter()

    store.update_status(task, "planning")
    plan = create_plan(task.user_requirement, task.solver_family, config)
    store.attach_plan(task, plan)
    writer.write_planned(task, plan)
    return plan
