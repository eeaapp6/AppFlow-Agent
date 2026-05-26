import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / "scripts"
if str(SCRIPTS) not in sys.path:
    sys.path.insert(0, str(SCRIPTS))

from agent_service import apply_replan, parse_replan_changes
from simagent_core.models import ReferenceCase, SimulationPlan
from simagent_core.state.task_store import TaskStore


class AgentServiceReplanTests(unittest.TestCase):
    def test_parse_replan_changes_normalizes_supported_aliases(self) -> None:
        changes, unrecognized = parse_replan_changes("把 end_time 改成 1.0, set nu to 0.005")

        self.assertEqual("1.0", changes["end_time"])
        self.assertEqual("0.005", changes["kinematic_viscosity"])
        self.assertEqual({}, unrecognized)

    def test_apply_replan_updates_plan_requested_changes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            store = TaskStore()
            task = store.create_task("cavity", tmp, "openfoam", tmp)
            plan = SimulationPlan(
                solver_family="openfoam",
                solver_name="icoFoam",
                physics_domain="incompressible",
                case_category="cavity",
                case_name="cavity",
                description="test",
                reference_case=ReferenceCase(
                    case_name="cavity",
                    case_domain="incompressible",
                    case_category="cavity",
                    case_solver="icoFoam",
                ),
                requested_changes={"delta_t": "0.005"},
                parameters={"requested_changes": {"delta_t": "0.005"}},
            )
            store.attach_plan(task, plan)

            updated_plan, replan = apply_replan(task, "把 end_time 改成 1.0", store)

            reloaded = store.load_task(task.task_dir)

        self.assertEqual("updated", replan["status"])
        self.assertEqual({"end_time": "1.0"}, replan["applied_changes"])
        self.assertEqual("1.0", updated_plan["requested_changes"]["end_time"])
        self.assertEqual("1.0", reloaded.plan["requested_changes"]["end_time"])
        self.assertEqual("1.0", reloaded.plan["parameters"]["requested_changes"]["end_time"])


if __name__ == "__main__":
    unittest.main()
