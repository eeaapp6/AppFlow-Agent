from abc import ABC, abstractmethod

from ..models import SimulationPlan


class SolverAdapter(ABC):
    solver_family: str

    @abstractmethod
    def create_plan(self, user_requirement: str) -> SimulationPlan:
        raise NotImplementedError
