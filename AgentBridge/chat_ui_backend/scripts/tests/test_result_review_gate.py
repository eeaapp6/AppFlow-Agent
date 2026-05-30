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
        review = result.metadata["result_review"]
        self.assertEqual("passed", review["status"])
        self.assertTrue(review["can_show_results"])
        self.assertFalse(review["should_offer_repair"])
        self.assertFalse(review["should_offer_rerun"])
        self.assertTrue(review["should_offer_vtk"])
        self.assertTrue(review["should_offer_paraview"])

    def test_fails_when_run_did_not_complete(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            result = review_run_results(task, {"status": "run_blocked", "reason": "WM_PROJECT_DIR is not set."})

        self.assertEqual("failed", result.status)
        self.assertEqual(["result.run_incomplete"], [issue.code for issue in result.issues])
        review = result.metadata["result_review"]
        self.assertEqual("failed", review["status"])
        self.assertFalse(review["can_show_results"])
        self.assertTrue(review["should_offer_repair"])
        self.assertTrue(review["should_offer_rerun"])

    def test_failed_run_uses_run_diagnostics_before_generic_incomplete_issue(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            result = review_run_results(
                task,
                {
                    "status": "run_failed",
                    "reason": "solver failed",
                    "diagnostics": {
                        "severity": "failed",
                        "summary": "Solver log contains nan or inf.",
                        "items": [
                            {
                                "code": "result.residual_nan",
                                "severity": "error",
                                "category": "numerics",
                                "message": "Solver log contains nan or inf.",
                                "source": "solver",
                                "log_file": "logs/icoFoam.log",
                                "matched_line": "ExecutionTime = nan s",
                            }
                        ],
                        "metrics": {"has_nan_or_inf": True},
                    },
                },
            )

        self.assertEqual("failed", result.status)
        self.assertEqual(["result.residual_nan"], [issue.code for issue in result.issues])
        self.assertTrue(result.metadata["diagnostics"]["metrics"]["has_nan_or_inf"])
        review = result.metadata["result_review"]
        self.assertEqual("failed", review["status"])
        self.assertTrue(review["should_offer_repair"])
        self.assertEqual("inspect_solver_numerics", review["repair_action_id"])
        self.assertEqual("executable", review["repair_mode"])

    def test_run_diagnostics_warning_marks_result_review_warning(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                    "diagnostics": {
                        "severity": "warning",
                        "summary": "OpenFOAM log contains a warning.",
                        "items": [
                            {
                                "code": "result.log_warning",
                                "severity": "warning",
                                "category": "runtime",
                                "message": "OpenFOAM log contains a warning.",
                                "source": "solver",
                                "log_file": "logs/icoFoam.log",
                                "matched_line": "--> FOAM Warning",
                            }
                        ],
                        "metrics": {},
                    },
                },
            )

        self.assertEqual("warning", result.status)
        self.assertEqual(["result.log_warning"], [issue.code for issue in result.issues])
        review = result.metadata["result_review"]
        self.assertEqual("warning", review["status"])
        self.assertTrue(review["can_show_results"])
        self.assertFalse(review["should_offer_repair"])
        self.assertTrue(review["should_offer_rerun"])

    def test_failed_diagnostics_without_items_still_fails_review(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            (Path(task.case_dir) / "0.5").mkdir(parents=True)

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                    "diagnostics": {
                        "severity": "failed",
                        "summary": "Diagnostics failed without detailed items.",
                        "items": [],
                    },
                },
            )

        self.assertEqual("failed", result.status)
        self.assertEqual(["result.diagnostics_failed"], [issue.code for issue in result.issues])
        self.assertFalse(result.metadata["result_review"]["can_show_results"])

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
        review = result.metadata["result_review"]
        self.assertFalse(review["can_show_results"])
        self.assertTrue(review["should_offer_repair"])
        self.assertTrue(review["should_offer_rerun"])

    def test_completed_run_without_result_time_dir_cannot_show_results(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "logs": [],
                    "outputs": {"results": {"latest_path": ""}},
                },
            )

        self.assertEqual("failed", result.status)
        review = result.metadata["result_review"]
        self.assertFalse(review["can_show_results"])
        self.assertFalse(review["should_offer_vtk"])
        self.assertFalse(review["should_offer_paraview"])

    def test_warning_only_without_result_dir_cannot_show_results(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            result = review_run_results(
                task,
                {
                    "status": "run_completed",
                    "diagnostics": {
                        "severity": "warning",
                        "summary": "OpenFOAM log contains a warning.",
                        "items": [
                            {
                                "code": "result.log_warning",
                                "severity": "warning",
                                "message": "OpenFOAM log contains a warning.",
                            }
                        ],
                    },
                    "outputs": {"results": {"latest_path": ""}},
                },
            )

        self.assertEqual("failed", result.status)
        review = result.metadata["result_review"]
        self.assertFalse(review["can_show_results"])
        self.assertTrue(review["should_offer_repair"])

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

    def test_run_diagnostics_groups_new_configuration_boundary_and_numerics_codes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            result = review_run_results(
                task,
                {
                    "status": "run_failed",
                    "diagnostics": {
                        "severity": "failed",
                        "summary": "configuration and boundary errors",
                        "items": [
                            {
                                "code": "result.pressure_reference_missing",
                                "severity": "error",
                                "category": "configuration",
                                "message": "Pressure reference is missing.",
                                "source": "solver",
                                "log_file": "logs/simpleFoam.log",
                                "matched_line": "Unable to set reference cell for field p",
                            },
                            {
                                "code": "result.patch_type_inconsistent",
                                "severity": "error",
                                "category": "boundary",
                                "message": "Patch type mismatch.",
                                "source": "solver",
                                "log_file": "logs/simpleFoam.log",
                                "matched_line": "patch type wall and patchField type fixedValue",
                            },
                            {
                                "code": "result.continuity_abnormal",
                                "severity": "warning",
                                "category": "numerics",
                                "message": "Continuity error is abnormal.",
                                "source": "solver",
                                "log_file": "logs/simpleFoam.log",
                                "matched_line": "time step continuity errors : sum local = 0.02, global = 0, cumulative = 0.02",
                            },
                        ],
                    },
                },
            )

        diagnostics = result.metadata["diagnostics"]
        self.assertEqual("failed", result.status)
        self.assertEqual(["boundary", "configuration", "numerics"], diagnostics["categories"])


if __name__ == "__main__":
    unittest.main()
