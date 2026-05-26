import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.gates.result_review_gate import review_run_results
from scripts.simagent_core.models import TaskContext


def make_task(root: Path) -> TaskContext:
    task_dir = root / "task_result_review"
    return TaskContext(
        version=1,
        task_id="task_result_review",
        status="run_completed",
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
    )


class ResultReviewGateTests(unittest.TestCase):
    def test_passes_when_run_completed_with_latest_result_dir_and_clean_logs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "icoFoam.log").write_text("End\n", encoding="utf-8")

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [{"path": "logs/icoFoam.log"}],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("passed", result.status)
        self.assertEqual([], result.issues)
        self.assertEqual("passed", result.metadata["diagnostics"]["severity"])
        self.assertEqual("Result review passed.", result.metadata["diagnostics"]["summary"])

    def test_fails_when_run_did_not_complete(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            result = review_run_results(task, {"status": "run_blocked", "reason": "WM_PROJECT_DIR is not set."})

        self.assertEqual("failed", result.status)
        self.assertEqual(["result.run_incomplete"], [issue.code for issue in result.issues])

    def test_fails_when_latest_result_dir_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("failed", result.status)
        self.assertEqual(["result.missing_latest_dir"], [issue.code for issue in result.issues])

    def test_fails_when_log_contains_fatal_marker(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "simpleFoam.log").write_text("FOAM FATAL ERROR\n", encoding="utf-8")

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [{"path": "logs/simpleFoam.log"}],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("failed", result.status)
        self.assertEqual(["result.log_fatal"], [issue.code for issue in result.issues])
        self.assertEqual(
            "result.log_fatal",
            result.metadata["diagnostics"]["primary_issue"]["code"],
        )
        self.assertEqual(["runtime"], result.metadata["diagnostics"]["categories"])
        evidence = result.metadata["diagnostics"]["evidence"]
        self.assertEqual("logs/simpleFoam.log", evidence["log_path"])
        self.assertEqual("simpleFoam", evidence["command"])
        self.assertEqual(1, evidence["line"])
        self.assertEqual("FOAM FATAL ERROR", evidence["marker"])
        self.assertEqual("FOAM FATAL ERROR", evidence["excerpt"])

    def test_fails_when_log_contains_nan_residual(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "simpleFoam.log").write_text(
                "DILUPBiCGStab:  Solving for Ux, Initial residual = nan, Final residual = nan, No Iterations 1\n"
                "ExecutionTime = 1 s\n",
                encoding="utf-8",
            )

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [{"path": "logs/simpleFoam.log"}],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("failed", result.status)
        self.assertIn("result.residual_nan", [issue.code for issue in result.issues])

    def test_fails_when_final_residual_is_high(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "simpleFoam.log").write_text(
                "smoothSolver:  Solving for p, Initial residual = 0.1, Final residual = 2.5, No Iterations 100\n"
                "smoothSolver:  Solving for U, Initial residual = 0.2, Final residual = 0.01, No Iterations 2\n"
                "ExecutionTime = 1 s\n",
                encoding="utf-8",
            )

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [{"path": "logs/simpleFoam.log"}],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("failed", result.status)
        self.assertIn("result.residual_high", [issue.code for issue in result.issues])
        evidence = result.metadata["diagnostics"]["evidence"]
        self.assertEqual("logs/simpleFoam.log", evidence["log_path"])
        self.assertEqual("p", evidence["field"])
        self.assertEqual(2.5, evidence["final_residual"])
        self.assertIn("Final residual = 2.5", evidence["excerpt"])
        metrics = result.metadata["diagnostics"]["metrics"]
        self.assertEqual(2, metrics["residual_count"])
        self.assertEqual(["U", "p"], metrics["residual_fields"])
        self.assertEqual(0.2, metrics["max_initial_residual"])
        self.assertEqual(2.5, metrics["max_final_residual"])
        self.assertEqual("p", metrics["worst_residual_field"])

    def test_warns_when_residual_log_has_no_execution_time_marker(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "simpleFoam.log").write_text(
                "smoothSolver:  Solving for p, Initial residual = 0.1, Final residual = 0.001, No Iterations 2\n",
                encoding="utf-8",
            )

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [{"path": "logs/simpleFoam.log"}],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("warning", result.status)
        self.assertEqual(["result.no_execution_time"], [issue.code for issue in result.issues])

    def test_fails_when_mesh_log_contains_quality_failure(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "checkMesh.log").write_text(
                "Mesh non-orthogonality Max: 73.5 average: 8\n"
                "Max skewness = 4.2 OK.\n"
                "Max aspect ratio = 180\n"
                "Min volume = -0.001\n"
                "Failed 1 mesh checks\n***Error in mesh quality\n",
                encoding="utf-8",
            )

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [{"path": "logs/checkMesh.log", "role": "checkMesh"}],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("failed", result.status)
        self.assertEqual(["result.mesh_quality_failed"], [issue.code for issue in result.issues])
        metrics = result.metadata["diagnostics"]["metrics"]
        self.assertEqual(1, metrics["failed_mesh_checks"])
        self.assertEqual(73.5, metrics["max_non_orthogonality"])
        self.assertEqual(4.2, metrics["max_skewness"])
        self.assertEqual(180, metrics["max_aspect_ratio"])
        self.assertEqual(-0.001, metrics["min_volume"])

    def test_warns_when_mesh_log_contains_quality_warning(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "checkMesh.log").write_text(
                "Mesh has severely non-orthogonal faces but no failed checks\n",
                encoding="utf-8",
            )

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [{"path": "logs/checkMesh.log", "role": "checkMesh"}],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("warning", result.status)
        self.assertEqual(["result.mesh_quality_warning"], [issue.code for issue in result.issues])

    def test_clean_mesh_log_does_not_add_issue(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "checkMesh.log").write_text("Mesh OK.\nEnd\n", encoding="utf-8")

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [{"path": "logs/checkMesh.log", "role": "checkMesh"}],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        self.assertEqual("passed", result.status)
        self.assertEqual([], result.issues)

    def test_diagnostics_groups_categories_and_prefers_error_primary_issue(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            Path(task.logs_dir).mkdir(parents=True)
            (Path(task.logs_dir) / "simpleFoam.log").write_text(
                "smoothSolver:  Solving for p, Initial residual = 0.1, Final residual = 2.5, No Iterations 100\n",
                encoding="utf-8",
            )
            (Path(task.logs_dir) / "checkMesh.log").write_text(
                "Mesh has severely non-orthogonal faces but no failed checks\n",
                encoding="utf-8",
            )

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [
                        {"path": "logs/simpleFoam.log"},
                        {"path": "logs/checkMesh.log", "role": "checkMesh"},
                    ],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

        diagnostics = result.metadata["diagnostics"]
        self.assertEqual("failed", diagnostics["severity"])
        self.assertEqual(["mesh", "numerics", "runtime"], diagnostics["categories"])
        self.assertEqual("result.residual_high", diagnostics["primary_issue"]["code"])
        self.assertEqual("logs/simpleFoam.log", diagnostics["evidence"]["log_path"])
        self.assertEqual(2.5, diagnostics["metrics"]["max_final_residual"])


if __name__ == "__main__":
    unittest.main()
