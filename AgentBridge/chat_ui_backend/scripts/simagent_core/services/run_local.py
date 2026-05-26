from ..models import SimulationPlan, TaskContext
from ..solvers.openfoam.run_local import run_openfoam_case


def run_case(task: TaskContext, plan: SimulationPlan | None = None) -> dict:
    if task.solver_family == "openfoam":
        return run_openfoam_case(task, plan)
    if task.solver_family == "phenglei":
        raise NotImplementedError("PHengLEI runner is reserved but not implemented yet.")

    raise ValueError(f"Unsupported solver_family: {task.solver_family}")

__all__ = ["run_case"]
