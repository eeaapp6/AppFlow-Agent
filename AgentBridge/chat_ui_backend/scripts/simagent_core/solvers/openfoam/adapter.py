from ...models import SimulationPlan
from ..base import SolverAdapter
from .plan import create_openfoam_plan


class OpenFOAMAdapter(SolverAdapter):
    solver_family = "openfoam"

    def create_plan(self, user_requirement: str) -> SimulationPlan:
        return create_openfoam_plan(user_requirement)
