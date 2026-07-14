from copy import deepcopy
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / "scripts"
if str(SCRIPTS) not in sys.path:
    sys.path.insert(0, str(SCRIPTS))

from agent_service import apply_replan, handle_replan, parse_replan_changes
from simagent_core.manifest.writer import ManifestWriter
from simagent_core.models import ReferenceCase, SimulationPlan
from simagent_core.solvers.openfoam.plan import create_openfoam_plan
from simagent_core.state.task_store import TaskStore
from simagent_core.workflow.nodes.input_writer_node import input_writer_node
from simagent_core.workflow.nodes.validator_node import validator_node
from simagent_core.workflow.state import allowed_workflow_actions


RECT_CHANNEL_PROMPT = (
    "\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u901a\u9053\uff0c\u957f\u5ea65m\uff0c\u9ad8\u5ea61m\uff0c"
    "\u5165\u53e3\u901f\u5ea61m/s\uff0c\u51fa\u53e3\u538b\u529b0\uff0c\u8fd0\u52a8\u7c98\u5ea60.01"
)


class CountingTaskStore(TaskStore):
    def __init__(self):
        super().__init__()
        self.save_calls = 0

    def save_task(self, task) -> None:
        self.save_calls += 1
        super().save_task(task)


class AgentServiceReplanTests(unittest.TestCase):
    def make_generated_task(self, root: Path, store: TaskStore):
        task = store.create_task(RECT_CHANNEL_PROMPT, str(root), "openfoam", str(root))
        plan = create_openfoam_plan(RECT_CHANNEL_PROMPT)
        store.attach_plan(task, plan)
        task.status = "run_completed"
        task.generated_files = [{"path": "case/system/controlDict"}]
        task.validation = {"status": "validated", "missing_files": []}
        task.run = {"status": "run_completed"}
        task.error_message = "stale run error"
        task.gate_reviews.extend(
            {"gate": gate, "status": "passed", "issues": []}
            for gate in (
                "generation",
                "static_validation",
                "validation",
                "physics_sanity",
                "execution",
                "result_review",
                "manifest",
            )
        )
        task.repair_history = [{"action_id": "inspect_solver_numerics", "status": "recorded"}]
        task.plan["generated_files"] = task.generated_files
        task.plan["validation"] = task.validation
        task.plan["run"] = task.run
        task.plan["gate_reviews"] = task.gate_reviews
        task.plan["repair_history"] = task.repair_history
        task.plan["parameters"]["gate_reviews"] = task.gate_reviews
        store.save_task(task)
        sentinel = Path(task.case_dir) / "generated_replan_sentinel.foam"
        sentinel.write_text("keep\n", encoding="utf-8")
        return task, sentinel

    def make_completed_task(self, root: Path, store: TaskStore):
        task = store.create_task("cavity", str(root), "openfoam", str(root))
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

        task.status = "run_completed"
        task.generated_files = [{"path": "case/system/controlDict"}]
        task.validation = {"status": "validated", "missing_files": []}
        task.run = {"status": "run_completed", "pipeline_info": {"selected": "basic"}}
        task.error_message = "stale run error"
        task.gate_reviews = [
            {
                "gate": "capability",
                "status": "warning",
                "issues": [],
                "metadata": {"marker": "preserve"},
            },
            {
                "gate": "spec",
                "status": "failed",
                "issues": [{"code": "spec.stale", "message": "stale review"}],
            },
        ] + [
            {"gate": gate, "status": "passed", "issues": []}
            for gate in (
                "generation",
                "static_validation",
                "validation",
                "physics_sanity",
                "execution",
                "result_review",
                "manifest",
            )
        ]
        task.repair_history = [
            {
                "action_id": "inspect_solver_numerics",
                "status": "recorded",
                "created_at": "2026-05-23T00:00:00+08:00",
            }
        ]
        task.plan["generated_files"] = task.generated_files
        task.plan["validation"] = task.validation
        task.plan["run"] = task.run
        task.plan["gate_reviews"] = task.gate_reviews
        task.plan["repair_history"] = task.repair_history
        task.plan["parameters"]["gate_reviews"] = task.gate_reviews
        task.plan["parameters"]["simulation_spec"] = {
            "marker": "stale",
            "numerics": {"time": {"end_time": "1.0", "delta_t": "0.005"}},
            "solver": {"parameters": {"end_time": "1.0", "delta_t": "0.005"}},
        }
        store.save_task(task)

        sentinel = Path(task.case_dir) / "replan_sentinel.foam"
        sentinel.write_text("keep\n", encoding="utf-8")
        return task, sentinel

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

    def test_successful_replan_invalidates_stale_downstream_state(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            store = CountingTaskStore()
            task, sentinel = self.make_completed_task(Path(tmp), store)
            repair_history = task.to_dict()["repair_history"]
            capability_gate = task.to_dict()["gate_reviews"][0]
            store.save_calls = 0

            response = handle_replan(
                task,
                {"message": "set end_time to 2.0"},
                store,
                ManifestWriter(),
            )
            updated_plan = response["plan"]
            replan = response["replan"]

            reloaded = store.load_task(task.task_dir)
            save_calls = store.save_calls
            case_dir_exists = Path(reloaded.case_dir).is_dir()
            sentinel_exists = sentinel.is_file()

        self.assertEqual("updated", replan["status"])
        self.assertEqual("generate", response["next_action"]["id"])
        self.assertEqual(
            ["generate", "replan", "repair_action"],
            response["allowed_actions"],
        )
        self.assertEqual({"end_time": "2.0"}, replan["applied_changes"])
        self.assertEqual("2.0", updated_plan["requested_changes"]["end_time"])
        self.assertEqual("2.0", reloaded.plan["requested_changes"]["end_time"])
        self.assertEqual("2.0", reloaded.plan["parameters"]["requested_changes"]["end_time"])
        self.assertEqual("planned", reloaded.status)
        self.assertEqual([], reloaded.generated_files)
        self.assertEqual({}, reloaded.validation)
        self.assertEqual({}, reloaded.run)
        self.assertEqual("", reloaded.error_message)
        self.assertNotIn("generated_files", reloaded.plan)
        self.assertNotIn("validation", reloaded.plan)
        self.assertNotIn("run", reloaded.plan)
        simulation_spec = reloaded.plan["parameters"]["simulation_spec"]
        self.assertEqual("2.0", simulation_spec["numerics"]["time"]["end_time"])
        self.assertEqual("2.0", simulation_spec["solver"]["parameters"]["end_time"])
        self.assertEqual("0.005", simulation_spec["numerics"]["time"]["delta_t"])
        self.assertEqual("0.005", simulation_spec["solver"]["parameters"]["delta_t"])
        self.assertNotIn("marker", simulation_spec)
        gate_names = [review["gate"] for review in reloaded.gate_reviews]
        self.assertEqual(1, gate_names.count("capability"))
        self.assertEqual(1, gate_names.count("spec"))
        self.assertEqual(capability_gate, reloaded.gate_reviews[gate_names.index("capability")])
        spec_gate = reloaded.gate_reviews[gate_names.index("spec")]
        self.assertEqual("passed", spec_gate["status"])
        self.assertEqual([], spec_gate["issues"])
        self.assertEqual({"capability", "spec"}, set(gate_names))
        self.assertEqual(reloaded.gate_reviews, reloaded.plan["gate_reviews"])
        self.assertEqual(reloaded.gate_reviews, reloaded.plan["parameters"]["gate_reviews"])
        self.assertEqual(reloaded.gate_reviews, updated_plan["parameters"]["gate_reviews"])
        self.assertEqual(repair_history, reloaded.repair_history)
        self.assertEqual(repair_history, reloaded.plan["repair_history"])
        self.assertEqual(1, save_calls)
        self.assertTrue(case_dir_exists)
        self.assertTrue(sentinel_exists)
        self.assertEqual(
            ["generate", "replan", "repair_action"],
            allowed_workflow_actions(reloaded),
        )

    def test_replan_without_supported_changes_has_no_side_effects(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            store = CountingTaskStore()
            task, sentinel = self.make_completed_task(Path(tmp), store)
            manifest_path = Path(task.manifest_path)
            manifest_path.write_bytes(b'{"sentinel": true}\n')
            context_path = Path(task.context_path)
            context_before = context_path.read_bytes()
            manifest_before = manifest_path.read_bytes()
            task_before = task.to_dict()
            updated_at_before = task.updated_at
            store.save_calls = 0

            with (
                patch("simagent_core.agent.replan.simulation_spec_from_openfoam_plan") as spec_builder,
                patch("simagent_core.agent.replan.review_simulation_spec") as spec_reviewer,
            ):
                response = handle_replan(
                    task,
                    {"message": "set mesh_density to 5"},
                    store,
                    ManifestWriter(),
                )

            self.assertEqual("no_supported_changes", response["replan"]["status"])
            self.assertEqual({}, response["next_action"])
            self.assertEqual(
                ["generate", "validate", "run", "replan", "repair_action"],
                response["allowed_actions"],
            )
            self.assertEqual(task_before, task.to_dict())
            self.assertEqual(updated_at_before, task.updated_at)
            self.assertEqual(context_before, context_path.read_bytes())
            self.assertEqual(manifest_before, manifest_path.read_bytes())
            self.assertEqual(0, store.save_calls)
            spec_builder.assert_not_called()
            spec_reviewer.assert_not_called()
            self.assertTrue(sentinel.is_file())

    def test_generated_rect_channel_replan_preserves_route_and_invalidates_downstream(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            store = CountingTaskStore()
            task, sentinel = self.make_generated_task(Path(tmp), store)
            route_before = deepcopy(task.plan["parameters"]["route_decision"])
            repair_history = deepcopy(task.repair_history)
            store.save_calls = 0

            response = handle_replan(
                task,
                {"message": "end_time=2.0"},
                store,
                ManifestWriter(),
            )
            reloaded = store.load_task(task.task_dir)
            manifest = Path(reloaded.manifest_path).read_text(encoding="utf-8")
            sentinel_exists = sentinel.is_file()

        spec = reloaded.plan["parameters"]["simulation_spec"]
        self.assertEqual("updated", response["replan"]["status"])
        self.assertEqual({"end_time": "2.0"}, response["replan"]["applied_changes"])
        self.assertEqual("generate", response["next_action"]["id"])
        self.assertEqual(["generate", "replan", "repair_action"], response["allowed_actions"])
        self.assertEqual("planned", reloaded.status)
        self.assertEqual("generated_case", reloaded.plan["generation_mode"])
        self.assertEqual("rect_channel", reloaded.plan["case_name"])
        self.assertEqual("icoFoam", reloaded.plan["solver_name"])
        self.assertIsNone(reloaded.plan["reference_case"])
        self.assertEqual(route_before, reloaded.plan["parameters"]["route_decision"])
        self.assertEqual("rect_channel", spec["geometry"]["type"])
        self.assertEqual(2.0, spec["numerics"]["time"]["end_time"])
        self.assertEqual(2.0, spec["solver"]["parameters"]["end_time"])
        self.assertEqual([], reloaded.generated_files)
        self.assertEqual({}, reloaded.validation)
        self.assertEqual({}, reloaded.run)
        self.assertEqual("", reloaded.error_message)
        self.assertEqual(repair_history, reloaded.repair_history)
        self.assertEqual({"capability", "spec"}, {item["gate"] for item in reloaded.gate_reviews})
        self.assertEqual(1, store.save_calls)
        self.assertTrue(sentinel_exists)
        self.assertIn('"status": "partial"', manifest)
        self.assertIn('"end_time": "2.0"', manifest)

    def test_generated_rect_channel_replan_updates_rendered_parameters_without_resetting_case(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            store = CountingTaskStore()
            task = store.create_task(RECT_CHANNEL_PROMPT, tmp, "openfoam", tmp)
            store.attach_plan(task, create_openfoam_plan(RECT_CHANNEL_PROMPT))
            input_writer_node(task, store, ManifestWriter())
            first_validation = validator_node(task, store, ManifestWriter())
            self.assertEqual("validated", first_validation["status"])
            case_dir = Path(task.case_dir)
            unchanged_paths = [
                case_dir / "system/blockMeshDict",
                case_dir / "0/U",
                case_dir / "0/p",
            ]
            unchanged_contents = {path: path.read_bytes() for path in unchanged_paths}
            initial_spec = deepcopy(task.plan["parameters"]["simulation_spec"])
            store.save_calls = 0

            response = handle_replan(
                task,
                {"message": "end_time=2.0, delta_t=0.01, write_interval=25, nu=0.02"},
                store,
                ManifestWriter(),
            )
            reloaded = store.load_task(task.task_dir)
            save_calls = store.save_calls
            generated = input_writer_node(reloaded, store, ManifestWriter())
            validation = validator_node(reloaded, store, ManifestWriter())
            control_dict = (Path(reloaded.case_dir) / "system/controlDict").read_text(encoding="utf-8")
            properties = (Path(reloaded.case_dir) / "constant/physicalProperties").read_text(encoding="utf-8")
            unchanged_after = {path: path.read_bytes() for path in unchanged_paths}

        spec = response["plan"]["parameters"]["simulation_spec"]
        self.assertEqual("updated", response["replan"]["status"])
        self.assertEqual(1, save_calls)
        self.assertEqual(0.01, spec["numerics"]["time"]["delta_t"])
        self.assertEqual(0.01, spec["solver"]["parameters"]["delta_t"])
        self.assertEqual(0.01, spec["numerics"]["controls"]["max_delta_t"])
        self.assertEqual(25, spec["outputs"]["controls"]["write_interval"])
        self.assertEqual(25, spec["solver"]["parameters"]["write_interval"])
        self.assertEqual(0.02, spec["physics"]["properties"]["kinematic_viscosity"])
        self.assertEqual(0.02, spec["solver"]["parameters"]["nu"])
        self.assertEqual(initial_spec["geometry"], spec["geometry"])
        self.assertEqual(initial_spec["mesh"], spec["mesh"])
        self.assertEqual(initial_spec["boundaries"], spec["boundaries"])
        self.assertEqual("generated_case", response["plan"]["generation_mode"])
        self.assertIn("endTime         2;", control_dict)
        self.assertIn("deltaT          0.01;", control_dict)
        self.assertIn("writeInterval   25;", control_dict)
        self.assertIn("0.02", properties)
        for path, content in unchanged_contents.items():
            self.assertEqual(content, unchanged_after[path])
        self.assertTrue(generated)
        generation_gate = next(item for item in reloaded.gate_reviews if item["gate"] == "generation")
        self.assertEqual("passed", generation_gate["status"])
        self.assertEqual("validated", validation["status"])
        self.assertEqual([], validation["missing_files"])

    def test_generated_rect_channel_unsupported_replan_has_no_side_effects(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            store = CountingTaskStore()
            task, _ = self.make_generated_task(Path(tmp), store)
            manifest_path = Path(task.manifest_path)
            manifest_path.write_bytes(b'{"sentinel": true}\n')
            context_before = Path(task.context_path).read_bytes()
            manifest_before = manifest_path.read_bytes()
            task_before = task.to_dict()
            updated_at_before = task.updated_at
            store.save_calls = 0

            response = handle_replan(
                task,
                {"message": "mesh_density=20"},
                store,
                ManifestWriter(),
            )
            context_after = Path(task.context_path).read_bytes()
            manifest_after = manifest_path.read_bytes()

        self.assertEqual("no_supported_changes", response["replan"]["status"])
        self.assertEqual({}, response["next_action"])
        self.assertEqual(task_before, task.to_dict())
        self.assertEqual(updated_at_before, task.updated_at)
        self.assertEqual(context_before, context_after)
        self.assertEqual(manifest_before, manifest_after)
        self.assertEqual(0, store.save_calls)

    def test_generated_rect_channel_invalid_values_have_no_side_effects(self) -> None:
        invalid_messages = (
            "end_time=0",
            "delta_t=-0.01",
            "write_interval=0",
            "nu=-0.01",
        )
        with tempfile.TemporaryDirectory() as tmp:
            for message in invalid_messages:
                with self.subTest(message=message):
                    store = CountingTaskStore()
                    task, _ = self.make_generated_task(Path(tmp), store)
                    manifest_path = Path(task.manifest_path)
                    manifest_path.write_bytes(b'{"sentinel": true}\n')
                    context_before = Path(task.context_path).read_bytes()
                    manifest_before = manifest_path.read_bytes()
                    task_before = task.to_dict()
                    updated_at_before = task.updated_at
                    store.save_calls = 0

                    response = handle_replan(task, {"message": message}, store, ManifestWriter())

                    self.assertEqual("invalid_changes", response["replan"]["status"])
                    self.assertTrue(response["replan"]["error"])
                    self.assertEqual({}, response["next_action"])
                    self.assertEqual(task_before, task.to_dict())
                    self.assertEqual(updated_at_before, task.updated_at)
                    self.assertEqual(context_before, Path(task.context_path).read_bytes())
                    self.assertEqual(manifest_before, manifest_path.read_bytes())
                    self.assertEqual(0, store.save_calls)

    def test_generated_rect_channel_invalid_simulation_spec_has_no_side_effects(self) -> None:
        invalid_specs = (
            None,
            "not-an-object",
            {"geometry": {"type": "cavity"}},
        )
        with tempfile.TemporaryDirectory() as tmp:
            for invalid_spec in invalid_specs:
                with self.subTest(invalid_spec=invalid_spec):
                    store = CountingTaskStore()
                    task, _ = self.make_generated_task(Path(tmp), store)
                    if invalid_spec is None:
                        del task.plan["parameters"]["simulation_spec"]
                    else:
                        task.plan["parameters"]["simulation_spec"] = invalid_spec
                    store.save_task(task)
                    manifest_path = Path(task.manifest_path)
                    manifest_path.write_bytes(b'{"sentinel": true}\n')
                    context_before = Path(task.context_path).read_bytes()
                    manifest_before = manifest_path.read_bytes()
                    task_before = task.to_dict()
                    updated_at_before = task.updated_at
                    store.save_calls = 0

                    response = handle_replan(
                        task,
                        {"message": "end_time=2.0"},
                        store,
                        ManifestWriter(),
                    )

                    self.assertEqual("invalid_simulation_spec", response["replan"]["status"])
                    self.assertTrue(response["replan"]["error"])
                    self.assertEqual({}, response["next_action"])
                    self.assertEqual(task_before, task.to_dict())
                    self.assertEqual(updated_at_before, task.updated_at)
                    self.assertEqual(context_before, Path(task.context_path).read_bytes())
                    self.assertEqual(manifest_before, manifest_path.read_bytes())
                    self.assertEqual(0, store.save_calls)


if __name__ == "__main__":
    unittest.main()
