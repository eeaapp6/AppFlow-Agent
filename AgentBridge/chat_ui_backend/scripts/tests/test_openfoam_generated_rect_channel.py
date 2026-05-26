import json
import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.models import TaskContext
from scripts.simagent_core.state.task_store import TaskStore
from scripts.simagent_core.solvers.openfoam.generated import create_rect_channel_plan, is_rect_channel_request
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
    def test_rect_channel_request_detection_accepts_chinese_channel_prompt(self) -> None:
        self.assertTrue(is_rect_channel_request(SIZED_RECT_CHANNEL_PROMPT))

    def test_plain_pipe_flow_does_not_force_rect_channel_generation(self) -> None:
        self.assertFalse(is_rect_channel_request("\u751f\u6210\u4e00\u4e2a\u7ba1\u9053\u6d41"))

    def test_rect_channel_plan_uses_generated_case_mode(self) -> None:
        plan = create_rect_channel_plan(SIZED_RECT_CHANNEL_PROMPT)

        self.assertEqual("generated_case", plan.generation_mode)
        self.assertEqual("rect_channel", plan.case_name)
        self.assertEqual("rect_channel", plan.parameters["simulation_spec"]["geometry"]["type"])
        self.assertEqual("blockMesh", plan.parameters["simulation_spec"]["mesh"]["type"])

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
