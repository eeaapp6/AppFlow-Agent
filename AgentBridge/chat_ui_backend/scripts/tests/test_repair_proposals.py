import json
import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.models import TaskContext
from scripts.simagent_core.repair import current_repair_action, repair_action_for_gate_review
from scripts.simagent_core.state.task_store import TaskStore


def make_task(root: Path) -> TaskContext:
    task_dir = root / "task_repair"
    return TaskContext(
        version=1,
        task_id="task_repair",
        status="planned",
        user_requirement="test",
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
        plan={"solver_family": "openfoam"},
    )


class RepairProposalTests(unittest.TestCase):
    def test_passed_gate_does_not_get_repair_action(self) -> None:
        action = repair_action_for_gate_review({"gate": "generation", "status": "passed", "issues": []})

        self.assertEqual({}, action)

    def test_execution_runtime_failure_maps_to_openfoam_runtime_action(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "execution",
                "status": "failed",
                "issues": [{"code": "execution.not_ready", "message": "WM_PROJECT_DIR is not set."}],
            }
        )

        self.assertEqual("configure_openfoam_runtime", action["id"])
        self.assertFalse(action["automatic"])

    def test_result_missing_directory_maps_to_log_review_action(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "result_review",
                "status": "failed",
                "issues": [{"code": "result.missing_latest_dir", "message": "missing"}],
            }
        )

        self.assertEqual("review_solver_logs", action["id"])

    def test_unstable_residual_maps_to_solver_numerics_action(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "result_review",
                "status": "failed",
                "issues": [{"code": "result.residual_nan", "message": "nan"}],
            }
        )

        self.assertEqual("inspect_solver_numerics", action["id"])

    def test_pressure_residual_metrics_refine_solver_numerics_action(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "result_review",
                "status": "failed",
                "issues": [{"code": "result.residual_high", "message": "high residual"}],
                "diagnostics": {
                    "metrics": {
                        "max_final_residual": 2.5,
                        "worst_residual_field": "p",
                    }
                },
            }
        )

        self.assertEqual("inspect_solver_numerics", action["id"])
        self.assertEqual("Inspect pressure numerics", action["label"])
        self.assertIn("Pressure residual", action["description"])
        self.assertIn("2.5", action["description"])

    def test_velocity_residual_metrics_refine_solver_numerics_action(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "result_review",
                "status": "failed",
                "issues": [{"code": "result.residual_high", "message": "high residual"}],
                "diagnostics": {
                    "metrics": {
                        "max_final_residual": 4.0,
                        "worst_residual_field": "U",
                    }
                },
            }
        )

        self.assertEqual("inspect_solver_numerics", action["id"])
        self.assertEqual("Inspect velocity numerics", action["label"])
        self.assertIn("Courant", action["description"])

    def test_mesh_quality_issue_maps_to_mesh_review_action(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "result_review",
                "status": "failed",
                "issues": [{"code": "result.mesh_quality_failed", "message": "mesh failed"}],
            }
        )

        self.assertEqual("review_mesh_quality", action["id"])

    def test_failed_mesh_check_metrics_refine_mesh_review_action(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "result_review",
                "status": "failed",
                "issues": [{"code": "result.mesh_quality_failed", "message": "mesh failed"}],
                "diagnostics": {"metrics": {"failed_mesh_checks": 2}},
            }
        )

        self.assertEqual("review_mesh_quality", action["id"])
        self.assertEqual("Review failed mesh checks", action["label"])
        self.assertIn("2 failed mesh check", action["description"])

    def test_non_orthogonal_mesh_metrics_refine_mesh_review_action(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "result_review",
                "status": "warning",
                "issues": [{"code": "result.mesh_quality_warning", "message": "mesh warning"}],
                "diagnostics": {"metrics": {"max_non_orthogonality": 73.5}},
            }
        )

        self.assertEqual("review_mesh_quality", action["id"])
        self.assertEqual("Review non-orthogonal mesh", action["label"])
        self.assertIn("73.5", action["description"])

    def test_unsupported_capability_decision_maps_to_scope_revision(self) -> None:
        action = repair_action_for_gate_review(
            {
                "gate": "capability",
                "status": "unsupported",
                "issues": [],
                "decision": {"mode": "unsupported"},
            }
        )

        self.assertEqual("revise_request_scope", action["id"])

    def test_task_store_persists_repair_action_without_mutating_input_review(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            Path(task.task_dir).mkdir(parents=True)
            review = {
                "gate": "result_review",
                "status": "failed",
                "issues": [{"code": "result.log_fatal", "message": "fatal"}],
            }

            TaskStore().attach_gate_review(task, review)
            saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))

        self.assertNotIn("next_repair_action", review)
        self.assertEqual("inspect_fatal_log", saved["gate_reviews"][0]["next_repair_action"]["id"])
        self.assertEqual(saved["gate_reviews"], saved["plan"]["gate_reviews"])

    def test_task_store_replaces_existing_gate_review(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            Path(task.task_dir).mkdir(parents=True)

            store = TaskStore()
            store.attach_gate_review(
                task,
                {
                    "gate": "static_validation",
                    "status": "failed",
                    "issues": [{"code": "validation.missing_file", "message": "Missing required file: case/0/p."}],
                },
            )
            store.attach_gate_review(
                task,
                {"gate": "static_validation", "status": "passed", "issues": []},
            )
            saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))

        self.assertEqual(1, len(saved["gate_reviews"]))
        self.assertEqual("passed", saved["gate_reviews"][0]["status"])
        self.assertNotIn("next_repair_action", saved["gate_reviews"][0])
        self.assertEqual({}, current_repair_action(task))

    def test_task_store_records_repair_history_with_source_gate(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            Path(task.task_dir).mkdir(parents=True)
            store = TaskStore()
            store.attach_gate_review(
                task,
                {
                    "gate": "static_validation",
                    "status": "failed",
                    "issues": [{"code": "validation.missing_file", "message": "Missing required file: case/0/p."}],
                },
            )

            store.record_repair_action(
                task,
                {
                    "id": "regenerate_case",
                    "label": "Regenerate case",
                },
            )
            saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))

        self.assertEqual("regenerate_case", saved["repair_history"][0]["action_id"])
        self.assertEqual("Regenerate case", saved["repair_history"][0]["label"])
        self.assertEqual("accepted", saved["repair_history"][0]["status"])
        self.assertEqual("static_validation", saved["repair_history"][0]["source_gate"])
        self.assertEqual(saved["repair_history"], saved["plan"]["repair_history"])

    def test_validation_result_closes_latest_open_repair_history_item(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            Path(task.task_dir).mkdir(parents=True)
            store = TaskStore()
            store.record_repair_action(task, {"id": "regenerate_case", "label": "Regenerate case"})

            store.attach_validation_result(task, {"status": "validated", "missing_files": []})
            saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))

        follow_up = saved["repair_history"][0]["follow_up"]
        self.assertEqual("validated", follow_up["validation_status"])
        self.assertIn("checked_at", follow_up)
        self.assertEqual(saved["repair_history"], saved["plan"]["repair_history"])

    def test_current_repair_action_returns_latest_failed_gate_action(self) -> None:
        task = make_task(Path("C:/tmp"))
        task.gate_reviews = [
            {"gate": "validation", "status": "failed", "next_repair_action": {"id": "old"}},
            {"gate": "generation", "status": "passed"},
            {"gate": "result_review", "status": "failed", "next_repair_action": {"id": "inspect_fatal_log"}},
        ]

        self.assertEqual({"id": "inspect_fatal_log"}, current_repair_action(task))


if __name__ == "__main__":
    unittest.main()
