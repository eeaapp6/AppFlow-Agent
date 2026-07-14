import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.simagent_core.models import TaskContext
from scripts.simagent_core.state.task_store import TaskStore
from scripts.simagent_core.solvers.openfoam.generated import create_rect_channel_plan, is_rect_channel_request
from scripts.simagent_core.solvers.openfoam.generated.rect_channel import (
    RectChannelGeometryError,
    rect_channel_spec_from_text,
)
from scripts.simagent_core.solvers.openfoam.input_writer import generate_openfoam_files
from scripts.simagent_core.solvers.openfoam.plan import create_openfoam_plan
from scripts.simagent_core.solvers.openfoam.run_pipeline import build_basic_run_pipeline
from scripts.simagent_core.solvers.openfoam.validate import validate_openfoam_case
from scripts.simagent_core.workflow.nodes.input_writer_node import input_writer_node
from scripts.simagent_core.workflow.nodes.validator_node import validator_node


RECT_CHANNEL_PROMPT = "\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u7ba1\u9053\u6d41"
SIZED_RECT_CHANNEL_PROMPT = (
    "\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u7ba1\u9053\u6d41\uff0c"
    "\u957f\u5ea6 5m\uff0c\u9ad8\u5ea6 1m\uff0c\u5165\u53e3\u901f\u5ea6 1m/s"
)
EXACT_RECT_CHANNEL_PROMPT = (
    "\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u901a\u9053\uff0c\u957f\u5ea65m\uff0c\u9ad8\u5ea61m\uff0c"
    "\u5165\u53e3\u901f\u5ea61m/s\uff0c\u51fa\u53e3\u538b\u529b0\uff0c\u8fd0\u52a8\u7c98\u5ea60.01"
)


class CountingPlanningLLM:
    def __init__(self) -> None:
        self.call_count = 0

    def chat_json(self, messages: list[dict]) -> dict:
        self.call_count += 1
        return {
            "solver_family": "openfoam",
            "solver_name": "simpleFoam",
            "physics_domain": "incompressible",
            "case_category": "externalAerodynamics",
            "case_name": "airFoil2D",
            "description": "must not be used",
            "generation_mode": "reference_modify",
            "requested_changes": {},
            "parameters": {},
            "planned_files": [],
        }


def make_task(root: Path) -> TaskContext:
    task_dir = root / "task_rect_channel"
    return TaskContext(
        version=1,
        task_id="task_rect_channel",
        status="planned",
        user_requirement=RECT_CHANNEL_PROMPT,
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


class OpenFOAMGeneratedRectChannelTests(unittest.TestCase):
    def assert_geometry_error(
        self,
        prompt: str,
        expected_code: str,
        *message_parts: str,
    ) -> None:
        with self.assertRaises(RectChannelGeometryError) as raised:
            rect_channel_spec_from_text(prompt)

        self.assertEqual(expected_code, raised.exception.code)
        for part in message_parts:
            self.assertIn(part, str(raised.exception))

    def test_rect_channel_request_detection_accepts_chinese_channel_prompt(self) -> None:
        self.assertTrue(is_rect_channel_request(SIZED_RECT_CHANNEL_PROMPT))

    def test_rect_channel_request_detection_accepts_chinese_rectangular_channel_prompt(self) -> None:
        self.assertTrue(is_rect_channel_request("\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u901a\u9053\u6d41"))

    def test_rect_channel_request_detection_accepts_english_prompt(self) -> None:
        self.assertTrue(is_rect_channel_request("Generate a laminar incompressible rectangular channel case"))

    def test_plain_pipe_flow_does_not_force_rect_channel_generation(self) -> None:
        self.assertFalse(is_rect_channel_request("\u751f\u6210\u4e00\u4e2a\u7ba1\u9053\u6d41"))

    def test_rect_channel_spec_uses_stable_defaults(self) -> None:
        spec = rect_channel_spec_from_text(RECT_CHANNEL_PROMPT)

        self.assertEqual({"length": 5.0, "height": 1.0, "depth": 0.1}, spec.geometry.parameters)
        self.assertEqual([100, 20, 1], spec.mesh.parameters["cells"])
        self.assertEqual(20, spec.mesh.parameters["mesh_density_per_meter"])
        self.assertEqual("icoFoam", spec.solver.name)
        self.assertEqual(0.005, spec.numerics.time["delta_t"])
        self.assertEqual(0.5, spec.numerics.controls["max_co"])

    def test_rect_channel_spec_extracts_chinese_parameters(self) -> None:
        prompt = (
            "\u751f\u6210\u4e8c\u7ef4\u77e9\u5f62\u901a\u9053\uff0c\u957f\u5ea6 6m\uff0c\u5bbd\u5ea6 0.5m\uff0c"
            "\u5165\u53e3\u901f\u5ea6 2m/s\uff0c\u8fd0\u52a8\u7c98\u5ea6 0.02\uff0c"
            "\u7f51\u683c\u5bc6\u5ea6 40\uff0c\u7ed3\u675f\u65f6\u95f4 2s\uff0c\u65f6\u95f4\u6b65 0.001"
        )

        spec = rect_channel_spec_from_text(prompt)

        self.assertEqual(6.0, spec.geometry.parameters["length"])
        self.assertEqual(0.5, spec.geometry.parameters["height"])
        self.assertEqual([240, 20, 1], spec.mesh.parameters["cells"])
        self.assertEqual([2.0, 0.0, 0.0], spec.boundaries[0].value["velocity"])
        self.assertEqual(0.02, spec.physics.properties["kinematic_viscosity"])
        self.assertEqual(2.0, spec.numerics.time["end_time"])
        self.assertEqual(0.001, spec.numerics.time["delta_t"])

    def test_rect_channel_spec_extracts_english_parameters(self) -> None:
        prompt = (
            "Generate a rectangular channel with L=4m, height=0.25m, U=1.5m/s, "
            "nu=1e-3, mesh density=30, endTime=3, deltaT=0.002"
        )

        spec = rect_channel_spec_from_text(prompt)

        self.assertEqual(4.0, spec.geometry.parameters["length"])
        self.assertEqual(0.25, spec.geometry.parameters["height"])
        self.assertEqual([120, 8, 1], spec.mesh.parameters["cells"])
        self.assertEqual([1.5, 0.0, 0.0], spec.boundaries[0].value["velocity"])
        self.assertEqual(1e-3, spec.physics.properties["kinematic_viscosity"])
        self.assertEqual(3.0, spec.solver.parameters["end_time"])
        self.assertEqual(0.002, spec.solver.parameters["delta_t"])

    def test_rect_channel_spec_uses_default_when_height_is_missing(self) -> None:
        spec = rect_channel_spec_from_text("rectangular channel length=5")

        self.assertEqual(1.0, spec.geometry.parameters["height"])

    def test_rect_channel_spec_rejects_negative_height(self) -> None:
        self.assert_geometry_error(
            "rectangular channel height=-1",
            "geometry.non_positive_height",
            "height",
            "-1",
        )

    def test_rect_channel_spec_rejects_zero_height(self) -> None:
        self.assert_geometry_error(
            "rectangular channel height=0",
            "geometry.non_positive_height",
            "height",
            "0",
        )

    def test_rect_channel_spec_rejects_negative_length(self) -> None:
        self.assert_geometry_error(
            "rectangular channel length=-5",
            "geometry.non_positive_length",
            "length",
            "-5",
        )

    def test_rect_channel_spec_rejects_zero_depth(self) -> None:
        self.assert_geometry_error(
            "rectangular channel depth=0",
            "geometry.non_positive_depth",
            "depth",
            "0",
        )

    def test_rect_channel_spec_rejects_negative_width_as_height(self) -> None:
        self.assert_geometry_error(
            "rectangular channel width=-1",
            "geometry.non_positive_height",
            "height",
            "width",
            "-1",
        )

    def test_rect_channel_spec_preserves_positive_fractional_height(self) -> None:
        spec = rect_channel_spec_from_text("rectangular channel height=0.5")

        self.assertEqual(0.5, spec.geometry.parameters["height"])

    def test_openfoam_plan_rejects_invalid_rect_channel_before_llm_or_reference(self) -> None:
        llm = CountingPlanningLLM()
        prompt = (
            "\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u901a\u9053\uff0c\u957f\u5ea6 5m\uff0c\u9ad8\u5ea6 -1m\uff0c"
            "\u5165\u53e3\u901f\u5ea6 1m/s\uff0c\u51fa\u53e3\u538b\u529b 0\uff0c\u8fd0\u52a8\u9ecf\u5ea6 0.01"
        )

        with patch("scripts.simagent_core.solvers.openfoam.plan.select_reference") as select_reference:
            with self.assertRaises(RectChannelGeometryError) as raised:
                create_openfoam_plan(prompt, llm=llm)

        self.assertEqual("geometry.non_positive_height", raised.exception.code)
        self.assertEqual(0, llm.call_count)
        select_reference.assert_not_called()

    def test_real_openfoam_plan_entry_uses_authoritative_generated_route(self) -> None:
        llm = CountingPlanningLLM()

        with patch("scripts.simagent_core.solvers.openfoam.plan.select_reference") as select_reference:
            plan = create_openfoam_plan(EXACT_RECT_CHANNEL_PROMPT, llm=llm)

        route = plan.parameters["route_decision"]
        decision = plan.parameters["capability_decision"]
        capability_gate = next(
            item for item in plan.parameters["gate_reviews"] if item["gate"] == "capability"
        )

        self.assertEqual(0, llm.call_count)
        select_reference.assert_not_called()
        self.assertEqual("generated_case", route["selected_mode"])
        self.assertEqual("generated_case", plan.generation_mode)
        self.assertEqual("rect_channel", route["geometry_type"])
        self.assertEqual("rect_channel", plan.case_name)
        self.assertEqual("icoFoam", plan.solver_name)
        self.assertEqual("rect_channel", plan.parameters["simulation_spec"]["geometry"]["type"])
        self.assertEqual("passed", capability_gate["status"])
        self.assertEqual({}, decision["unsupported_changes"])
        self.assertNotEqual("airFoil2D", plan.case_name)

    def test_unsupported_created_geometry_fails_before_llm_or_reference(self) -> None:
        llm = CountingPlanningLLM()

        with patch("scripts.simagent_core.solvers.openfoam.plan.select_reference") as select_reference:
            with self.assertRaisesRegex(ValueError, "not supported"):
                create_openfoam_plan("Generate a 3D turbine blade flow field", llm=llm)

        self.assertEqual(0, llm.call_count)
        select_reference.assert_not_called()

    def test_rect_channel_plan_uses_generated_case_mode(self) -> None:
        plan = create_rect_channel_plan(SIZED_RECT_CHANNEL_PROMPT)

        self.assertEqual("generated_case", plan.generation_mode)
        self.assertEqual("rect_channel", plan.case_name)
        self.assertEqual("rect_channel", plan.parameters["simulation_spec"]["geometry"]["type"])
        self.assertEqual("blockMesh", plan.parameters["simulation_spec"]["mesh"]["type"])
        self.assertEqual("incompressible", plan.parameters["simulation_spec"]["physics"]["domain"])
        self.assertEqual("PISO", plan.parameters["simulation_spec"]["numerics"]["algorithm"])
        self.assertEqual(["U", "p"], plan.parameters["simulation_spec"]["outputs"]["fields"])

    def test_generated_rect_channel_case_validates(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            plan = create_rect_channel_plan(SIZED_RECT_CHANNEL_PROMPT)

            generated = generate_openfoam_files(task, plan)
            validation = validate_openfoam_case(task, plan)

            generated_paths = {item.path for item in generated}
            self.assertIn("case/system/blockMeshDict", generated_paths)
            self.assertIn("case/0/U", generated_paths)
            self.assertIn("case/0/p", generated_paths)
            self.assertEqual("validated", validation["status"])
            self.assertEqual(["frontAndBack", "inlet", "outlet", "walls"], sorted(validation["mesh_patches"]))
            self.assertEqual(
                {"frontAndBack": "empty", "inlet": "fixedValue", "outlet": "zeroGradient", "walls": "noSlip"},
                validation["boundary_field_types"]["U"],
            )
            self.assertEqual(
                {"frontAndBack": "empty", "inlet": "zeroGradient", "outlet": "fixedValue", "walls": "zeroGradient"},
                validation["boundary_field_types"]["p"],
            )

    def test_generated_rect_channel_writes_requested_cfd_controls(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            plan = create_rect_channel_plan(
                "Generate a rectangular channel with length=3, height=0.5, velocity=2, nu=0.02, "
                "mesh density=10, endTime=4, deltaT=0.01"
            )

            generate_openfoam_files(task, plan)

            block_mesh = (Path(task.task_dir) / "case/system/blockMeshDict").read_text(encoding="utf-8")
            control = (Path(task.task_dir) / "case/system/controlDict").read_text(encoding="utf-8")
            physical = (Path(task.task_dir) / "case/constant/physicalProperties").read_text(encoding="utf-8")
            u_field = (Path(task.task_dir) / "case/0/U").read_text(encoding="utf-8")
            self.assertIn("hex (0 1 2 3 4 5 6 7) (30 5 1)", block_mesh)
            self.assertIn("endTime         4;", control)
            self.assertIn("deltaT          0.01;", control)
            self.assertIn("maxCo           0.5;", control)
            self.assertIn("nu              [0 2 -1 0 0 0 0] 0.02;", physical)
            self.assertIn("value           uniform (2 0 0);", u_field)

    def test_generated_rect_channel_pipeline_uses_blockmesh_then_solver(self) -> None:
        plan = create_rect_channel_plan(RECT_CHANNEL_PROMPT)

        pipeline = build_basic_run_pipeline(plan)

        self.assertEqual(["blockMesh", "icoFoam"], [item["command"] for item in pipeline])

    def test_generation_and_validation_nodes_persist_gate_reviews(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            store = TaskStore()
            plan = create_openfoam_plan(
                "\u6570\u636e\u5e93\u6ca1\u6709\u4e5f\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u7ba1\u9053"
            )
            store.attach_plan(task, plan)

            input_writer_node(task, store)
            validator_node(task, store)
            reloaded = store.load_task(task.task_dir)

        gates = [item["gate"] for item in reloaded.gate_reviews]
        self.assertIn("capability", gates)
        self.assertIn("spec", gates)
        self.assertIn("generation", gates)
        self.assertIn("static_validation", gates)
        self.assertEqual(reloaded.gate_reviews, reloaded.plan["gate_reviews"])

    def test_validation_failure_persists_repair_action_in_task_and_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            store = TaskStore()
            plan = create_openfoam_plan(
                "\u6570\u636e\u5e93\u6ca1\u6709\u4e5f\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u7ba1\u9053"
            )
            store.attach_plan(task, plan)
            input_writer_node(task, store)
            (Path(task.task_dir) / "case" / "0" / "p").unlink()

            validation = validator_node(task, store)
            reloaded = store.load_task(task.task_dir)
            manifest = json.loads(Path(task.manifest_path).read_text(encoding="utf-8"))

        self.assertEqual("validation_failed", validation["status"])
        static_validation = next(item for item in reloaded.gate_reviews if item["gate"] == "static_validation")
        self.assertEqual("failed", static_validation["status"])
        self.assertEqual("regenerate_case", static_validation["next_repair_action"]["id"])

        manifest_reviews = manifest["workflow"]["gates"]["reviews"]
        manifest_validation = next(item for item in manifest_reviews if item["gate"] == "static_validation")
        self.assertEqual("regenerate_case", manifest_validation["next_repair_action"]["id"])


if __name__ == "__main__":
    unittest.main()
