from ..config import Config
from ..models import SimulationPlan
from ..router_func import select_solver_family
from ..solvers.openfoam import OpenFOAMAdapter
from ..solvers.phenglei import PHengLEIAdapter


def create_plan(user_requirement: str, solver_family: str | None = None, config: Config | None = None) -> SimulationPlan:
    cfg = config or Config.from_environment()
    family = (solver_family or select_solver_family(user_requirement, cfg)).strip().lower()

    if family == "openfoam":
        return OpenFOAMAdapter().create_plan(user_requirement)
    if family == "phenglei":
        return PHengLEIAdapter().create_plan(user_requirement)

    raise ValueError(f"Unsupported solver_family: {family}")

__all__ = ["create_plan"]
