from ..models import PlannedFile, SimulationPlan, TaskContext
from ..solvers.openfoam.input_writer import generate_openfoam_files


def generate_files(task: TaskContext, plan: SimulationPlan) -> list[PlannedFile]:
    if plan.solver_family == "openfoam":
        return generate_openfoam_files(task, plan)
    if plan.solver_family == "phenglei":
        raise NotImplementedError("PHengLEI input writer is reserved but not implemented yet.")

    raise ValueError(f"Unsupported solver_family: {plan.solver_family}")

__all__ = ["generate_files"]
