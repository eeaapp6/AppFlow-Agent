from ...models import SimulationPlan
from ..base import SolverAdapter


class PHengLEIAdapter(SolverAdapter):
    solver_family = "phenglei"

    def create_plan(self, user_requirement: str) -> SimulationPlan:
        raise NotImplementedError("PHengLEI adapter is reserved but not implemented yet.")
