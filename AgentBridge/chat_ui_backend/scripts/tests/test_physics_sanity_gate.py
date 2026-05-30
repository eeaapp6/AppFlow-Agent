import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.simagent_core.gates.physics_sanity_gate import review_physics_sanity
from scripts.simagent_core.manifest.writer import ManifestWriter
from scripts.simagent_core.models import SimulationPlan, TaskContext
from scripts.simagent_core.repair.proposals import repair_action_for_gate_review
from scripts.simagent_core.solvers.openfoam.generated import create_rect_channel_plan
from scripts.simagent_core.spec import SimulationSpec
from scripts.simagent_core.state.task_store import TaskStore
from scripts.simagent_core.workflow.nodes.local_runner_node import local_runner_node


def make_task(root: Path) -> TaskContext:
    task_dir = root / "task_physics_sanity"
    for path in [
        task_dir,
        task_dir / "case",
        task_dir / "mesh",
        task_dir / "results",
        task_dir / "logs",
    ]:
        path.mkdir(parents=True, exist_ok=True)
    return TaskContext(
        version=1,
        task_id="task_physics_sanity",
        status="planned",
        user_requirement="rect channel",
        solver_family="openfoam",
        output_root=str(root),
        host_output_root=str(root),
        task_dir=str(task_dir),
        case_dir=str(task_dir / "case"),
        mesh_dir=str(task_dir / "mesh"),
        results_dir=str(task_dir / "results"),
        logs_dir=str(task_dir / "logs"),
        context_path=str(task_dir / "task_context.json"),
        manifest_path=str(task_dir / "agent_manifest_v1.json"),
    )


def rect_plan() -> SimulationPlan:
    return create_rect_channel_plan("Generate a rectangular channel")


def spec_from_plan(plan: SimulationPlan) -> SimulationSpec:
    return SimulationSpec.from_dict(plan.parameters["simulation_spec"])


def put_spec(plan: SimulationPlan, spec: SimulationSpec) -> SimulationPlan:
    plan.parameters["simulation_spec"] = spec.to_dict()
    return plan


class PhysicsSanityGateTests(unittest.TestCase):
    def test_rect_channel_defaults_pass(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = rect_plan()

            result = review_physics_sanity(task, plan)

        physics = result.metadata["physics_sanity"]
        self.assertEqual("passed", result.status)
        self.assertTrue(physics["supported"])
        self.assertEqual("rect_channel", physics["case_name"])
        self.assertAlmostEqual(100.0, physics["metrics"]["reynolds_number"])
        self.assertAlmostEqual(0.1, physics["metrics"]["estimated_courant"])
        self.assertEqual(
            [
                "Uses channel height as characteristic length.",
                "Uses minimum blockMesh cell size for estimated Courant number.",
            ],
            physics["assumptions"],
        )

    def test_large_delta_t_warns_for_courant_risk(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = rect_plan()
            spec = spec_from_plan(plan)
            spec.numerics.time["delta_t"] = 0.1
            spec.solver.parameters["delta_t"] = 0.1
            put_spec(plan, spec)

            result = review_physics_sanity(task, plan)

        self.assertEqual("warning", result.status)
        self.assertIn("physics.courant_risk", [issue.code for issue in result.issues])
        self.assertGreater(result.metadata["physics_sanity"]["metrics"]["estimated_courant"], 1.0)

    def test_non_positive_delta_t_or_nu_fails(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = rect_plan()
            spec = spec_from_plan(plan)
            spec.numerics.time["delta_t"] = 0
            spec.physics.properties["kinematic_viscosity"] = -1e-3
            put_spec(plan, spec)

            result = review_physics_sanity(task, plan)

        codes = [issue.code for issue in result.issues]
        self.assertEqual("failed", result.status)
        self.assertIn("physics.invalid_time_step", codes)
        self.assertIn("physics.invalid_viscosity", codes)

    def test_high_reynolds_with_icofoam_warns(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = rect_plan()
            spec = spec_from_plan(plan)
            spec.physics.properties["kinematic_viscosity"] = 1e-5
            put_spec(plan, spec)

            result = review_physics_sanity(task, plan)

        self.assertEqual("warning", result.status)
        self.assertIn("physics.reynolds_solver_risk", [issue.code for issue in result.issues])
        self.assertGreater(result.metadata["physics_sanity"]["metrics"]["reynolds_number"], 1000.0)

    def test_coarse_mesh_warns(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = rect_plan()
            spec = spec_from_plan(plan)
            spec.mesh.parameters["cells"] = [4, 2, 1]
            put_spec(plan, spec)

            result = review_physics_sanity(task, plan)

        self.assertEqual("warning", result.status)
        self.assertIn("physics.mesh_too_coarse", [issue.code for issue in result.issues])

    def test_zero_velocity_warns_without_dropping_metrics(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = rect_plan()
            spec = spec_from_plan(plan)
            spec.boundaries[0].value["velocity"] = [0.0, 0.0, 0.0]
            put_spec(plan, spec)

            result = review_physics_sanity(task, plan)

        physics = result.metadata["physics_sanity"]
        self.assertEqual("warning", result.status)
        self.assertIn("physics.zero_velocity", [issue.code for issue in result.issues])
        self.assertEqual(0.0, physics["metrics"]["velocity_magnitude"])
        self.assertEqual(0.0, physics["metrics"]["estimated_courant"])

    def test_reference_case_is_unsupported_without_failure(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = SimulationPlan(
                solver_family="openfoam",
                solver_name="icoFoam",
                physics_domain="incompressible",
                case_category="tutorial",
                case_name="cavity",
                description="reference",
                generation_mode="reference_modify",
            )

            result = review_physics_sanity(task, plan)

        physics = result.metadata["physics_sanity"]
        self.assertEqual("passed", result.status)
        self.assertFalse(physics["supported"])
        self.assertEqual("cavity", physics["case_name"])

    def test_failed_physics_sanity_blocks_run(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            plan = rect_plan()
            spec = spec_from_plan(plan)
            spec.numerics.time["delta_t"] = -1
            put_spec(plan, spec)
            task.plan = plan.to_dict()
            store = TaskStore()
            store.save_task(task)

            with patch("scripts.simagent_core.workflow.nodes.local_runner_node.run_case") as run_case:
                result = local_runner_node(task, store, ManifestWriter())

            run_case.assert_not_called()
            reloaded = store.load_task(task.task_dir)

        self.assertEqual("run_blocked", result["status"])
        self.assertIn("physics.invalid_time_step", [item["code"] for item in result["diagnostics"]["items"]])
        physics_review = next(item for item in reloaded.gate_reviews if item["gate"] == "physics_sanity")
        self.assertEqual("failed", physics_review["status"])
        self.assertEqual("revise_simulation_spec", physics_review["next_repair_action"]["id"])

    def test_warning_physics_sanity_does_not_block_run(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            plan = rect_plan()
            spec = spec_from_plan(plan)
            spec.numerics.time["delta_t"] = 0.1
            put_spec(plan, spec)
            task.plan = plan.to_dict()
            store = TaskStore()
            store.save_task(task)

            fake_run = {
                "status": "run_completed",
                "reason": "mocked run",
                "steps": [],
                "pipeline": [{"command": "blockMesh"}, {"command": "icoFoam"}],
                "pipeline_info": {},
                "logs": [],
                "outputs": {"results": {"latest_path": "case/0.5", "latest_time": "0.5"}},
                "diagnostics": {"severity": "passed", "summary": "No OpenFOAM run diagnostics.", "items": [], "metrics": {}},
            }
            with patch("scripts.simagent_core.workflow.nodes.local_runner_node.run_case", return_value=fake_run) as run_case:
                result = local_runner_node(task, store, ManifestWriter())

            run_case.assert_called_once()
            reloaded = store.load_task(task.task_dir)

        self.assertEqual("run_completed", result["status"])
        physics_review = next(item for item in reloaded.gate_reviews if item["gate"] == "physics_sanity")
        self.assertEqual("warning", physics_review["status"])

    def test_manifest_gates_include_physics_sanity(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = rect_plan()
            physics_review = review_physics_sanity(task, plan).to_dict()
            task.gate_reviews = [physics_review]

            manifest = ManifestWriter().write_planned(task, plan)

        review = manifest["workflow"]["gates"]["reviews"][0]
        self.assertEqual("physics_sanity", review["gate"])
        self.assertEqual("passed", review["physics_sanity"]["status"])
        self.assertEqual("passed", manifest["appflow_hints"]["physics_sanity_status"])

    def test_repair_proposal_for_physics_codes_is_manual(self) -> None:
        invalid_action = repair_action_for_gate_review(
            {
                "gate": "physics_sanity",
                "status": "failed",
                "issues": [{"code": "physics.invalid_time_step", "message": "bad deltaT"}],
            }
        )
        courant_action = repair_action_for_gate_review(
            {
                "gate": "physics_sanity",
                "status": "warning",
                "issues": [{"code": "physics.courant_risk", "message": "high Co"}],
            }
        )

        self.assertEqual("revise_simulation_spec", invalid_action["id"])
        self.assertEqual("inspect_solver_numerics", courant_action["id"])
        self.assertNotIn("patches", invalid_action)
        self.assertNotIn("patches", courant_action)


if __name__ == "__main__":
    unittest.main()
