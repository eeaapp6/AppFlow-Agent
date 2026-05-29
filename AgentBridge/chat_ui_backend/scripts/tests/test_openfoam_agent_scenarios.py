import tempfile
import unittest
from pathlib import Path

import agent_service
from scripts.simagent_core.gates.result_review_gate import review_run_results
from scripts.simagent_core.manifest.writer import ManifestWriter
from scripts.simagent_core.models import SimulationPlan, TaskContext
from scripts.simagent_core.repair import current_repair_action
from scripts.simagent_core.solvers.openfoam.generated import create_rect_channel_plan
from scripts.simagent_core.solvers.openfoam.input_writer import generate_openfoam_files
from scripts.simagent_core.solvers.openfoam.run_local import run_openfoam_case
from scripts.simagent_core.solvers.openfoam.validate import validate_openfoam_case
from scripts.simagent_core.state.task_store import TaskStore


RECT_CHANNEL_PROMPT = "Generate a laminar rectangular channel length=3 height=0.5 velocity=1"


def make_task(root: Path, task_id: str = "task_scenario") -> TaskContext:
    task_dir = root / task_id
    return TaskContext(
        version=1,
        task_id=task_id,
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


def available_runtime_info() -> dict:
    return {
        "mode": "local",
        "backend": "local",
        "available": True,
        "reason": "Local OpenFOAM runtime is available.",
        "wm_project_dir": "/opt/openfoam",
        "missing_commands": [],
        "commands": {
            "blockMesh": {"available": True},
            "icoFoam": {"available": True},
        },
    }


def write_reviewed_run_manifest(
    task: TaskContext,
    store: TaskStore,
    plan: SimulationPlan,
    run_result: dict,
) -> dict:
    review = review_run_results(task, run_result)
    store.attach_gate_review(task, review.to_dict(), save=False)
    store.attach_run_result(task, run_result)
    return ManifestWriter().write_run_result(task, plan, run_result)


class ScenarioRunner:
    def __init__(self, solver_log: str = "End\n", create_results: bool = True) -> None:
        self.solver_log = solver_log
        self.create_results = create_results

    def run_step(self, command: list[str], case_dir: Path, log_path: Path, timeout_seconds: int) -> dict:
        log_path.parent.mkdir(parents=True, exist_ok=True)
        command_name = command[0]
        if command_name == "blockMesh":
            log_path.write_text("blockMesh OK\n", encoding="utf-8")
        else:
            log_path.write_text(self.solver_log, encoding="utf-8")
            if self.create_results:
                result_dir = case_dir / "0.5"
                result_dir.mkdir(parents=True, exist_ok=True)
                (result_dir / "U").write_text("result U\n", encoding="utf-8")
                (result_dir / "p").write_text("result p\n", encoding="utf-8")
        return {
            "command": " ".join(command),
            "return_code": 0,
            "log": log_path.name,
            "log_path": str(log_path),
        }


class OpenFOAMAgentScenarioTests(unittest.TestCase):
    def test_generated_rect_channel_happy_path_reaches_showable_result_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root, "scenario_rect_channel")
            store = TaskStore()
            plan = create_rect_channel_plan(RECT_CHANNEL_PROMPT)
            store.attach_plan(task, plan)

            generated = generate_openfoam_files(task, plan)
            validation = validate_openfoam_case(task, plan)
            run_result = run_openfoam_case(
                task,
                plan,
                runtime_info=available_runtime_info(),
                runner=ScenarioRunner(),
            )
            manifest = write_reviewed_run_manifest(task, store, plan, run_result)

        self.assertIn("case/system/blockMeshDict", {item.path for item in generated})
        self.assertEqual("validated", validation["status"])
        self.assertEqual("run_completed", run_result["status"])
        self.assertEqual("passed", manifest["workflow"]["run"]["result_review"]["status"])
        self.assertTrue(manifest["workflow"]["run"]["result_review"]["can_show_results"])
        self.assertEqual("local", manifest["workflow"]["run"]["runtime_info"]["backend"])
        self.assertTrue(manifest["appflow_hints"]["show_results"])
        self.assertTrue(manifest["appflow_hints"]["run_foam_to_vtk"])

    def test_missing_boundary_field_repair_then_rerun_marks_followup_resolved(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root, "scenario_missing_boundary")
            store = TaskStore()
            plan = create_rect_channel_plan(RECT_CHANNEL_PROMPT)
            task.plan = plan.to_dict()
            field_path = Path(task.task_dir) / "case/0/U"
            field_path.parent.mkdir(parents=True)
            field_path.write_text("boundaryField\n{\n}\n", encoding="utf-8")

            store.attach_gate_review(
                task,
                {
                    "gate": "result_review",
                    "status": "failed",
                    "issues": [{"code": "result.missing_boundary_field", "message": "U missing inlet"}],
                    "diagnostics": {
                        "categories": ["boundary"],
                        "metrics": {"missing_boundary_fields": {"U": ["inlet"]}},
                    },
                },
            )
            action = current_repair_action(task)
            applied_action = agent_service.apply_and_record_repair_action(task, action, store)
            rerun_result = {
                "status": "run_completed",
                "diagnostics": {"severity": "passed", "items": [], "metrics": {}},
                "outputs": {"results": {"latest_path": ""}},
            }
            store.attach_run_result(task, rerun_result)
            manifest = ManifestWriter().write_run_result(task, plan, rerun_result)
            updated_field = field_path.read_text(encoding="utf-8")

        self.assertEqual("repair_case_dictionaries", applied_action["id"])
        self.assertIn("inlet", updated_field)
        history = task.repair_history[0]
        self.assertEqual("applied", history["status"])
        self.assertEqual(["result.missing_boundary_field"], history["related_diagnostic_codes"])
        self.assertEqual("resolved", history["repair_followup"]["status"])
        self.assertEqual(["result.missing_boundary_field"], manifest["workflow"]["repair_followup"]["resolved_codes"])
        self.assertTrue(manifest["workflow"]["last_repair_action"]["should_rerun"])

    def test_nan_divergence_repair_suggestion_then_rerun_marks_followup_unresolved(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root, "scenario_nan_divergence")
            store = TaskStore()
            plan = create_rect_channel_plan(RECT_CHANNEL_PROMPT)
            store.attach_plan(task, plan)
            generate_openfoam_files(task, plan)

            first_run = run_openfoam_case(
                task,
                plan,
                runtime_info=available_runtime_info(),
                runner=ScenarioRunner("solution diverged\nExecutionTime = nan s\n", create_results=True),
            )
            review = review_run_results(task, first_run)
            store.attach_gate_review(task, review.to_dict(), save=False)
            store.attach_run_result(task, first_run)
            action = current_repair_action(task)

            self.assertEqual("run_failed", first_run["status"])
            self.assertEqual("failed", review.status)
            self.assertTrue(review.metadata["result_review"]["should_offer_repair"])
            self.assertTrue(review.metadata["result_review"]["should_offer_rerun"])
            self.assertEqual([], task.repair_history)

            applied_action = agent_service.apply_and_record_repair_action(task, action, store)
            rerun_result = {
                "status": "run_failed",
                "diagnostics": {
                    "severity": "failed",
                    "items": [{"code": "result.residual_nan", "severity": "error"}],
                    "metrics": {"has_nan_or_inf": True},
                },
            }
            store.attach_run_result(task, rerun_result)
            manifest = ManifestWriter().write_run_result(task, plan, rerun_result)

        self.assertEqual("inspect_solver_numerics", applied_action["id"])
        self.assertEqual("applied", task.repair_history[0]["status"])
        self.assertEqual("unresolved", task.repair_history[0]["repair_followup"]["status"])
        self.assertIn("result.residual_nan", task.repair_history[0]["repair_followup"]["unresolved_codes"])
        self.assertEqual("unresolved", manifest["workflow"]["repair_followup"]["status"])
        self.assertTrue(manifest["appflow_hints"]["offer_rerun"])


if __name__ == "__main__":
    unittest.main()
