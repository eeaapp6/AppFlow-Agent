from ..models import SimulationPlan, TaskContext
from ..solvers.openfoam.validate import validate_openfoam_case


def validate_case(task: TaskContext, plan: SimulationPlan | None = None) -> dict:
    if task.solver_family == "openfoam":
        return validate_openfoam_case(task, plan)
    if task.solver_family == "phenglei":
        raise NotImplementedError("PHengLEI validator is reserved but not implemented yet.")

    raise ValueError(f"Unsupported solver_family: {task.solver_family}")

__all__ = ["validate_case"]
