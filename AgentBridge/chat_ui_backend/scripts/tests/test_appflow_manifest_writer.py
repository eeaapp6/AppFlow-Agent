import json
import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.manifest.validator import validate_appflow_manifest
from scripts.simagent_core.manifest.writer import ManifestWriter
from scripts.simagent_core.models import SimulationPlan, TaskContext
from scripts.simagent_core.workflow.nodes.local_runner_node import _manifest_gate_review


def make_task(root: Path) -> TaskContext:
    task_id = "demo_case_001"
    task_dir = root / task_id
    return TaskContext(
        version=1,
        task_id=task_id,
        status="context_created",
        user_requirement="test case",
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
        created_at="2026-05-16T14:30:00+08:00",
        updated_at="2026-05-16T14:30:00+08:00",
    )


def make_plan() -> SimulationPlan:
    return SimulationPlan(
        solver_family="openfoam",
        solver_name="icoFoam",
        physics_domain="incompressible",
        case_category="cavity",
        case_name="cavity",
        description="test plan",
        requested_changes={
            "end_time": 0.5,
            "delta_t": 0.005,
            "kinematic_viscosity": 0.01,
        },
    )


def read_written_manifest(task: TaskContext) -> dict:
    return json.loads(Path(task.manifest_path).read_text(encoding="utf-8"))


class AppFlowManifestWriterTests(unittest.TestCase):
    def test_manifest_includes_normalized_gate_summary(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            task.gate_reviews = [
                {
                    "gate": "capability",
                    "status": "passed",
                    "issues": [],
                    "decision": {"mode": "reference_modify", "reason": "test"},
                },
                {
                    "gate": "static_validation",
                    "status": "failed",
                    "next_repair_action": {
                        "id": "regenerate_case",
                        "label": "Regenerate case",
                        "description": "test",
                        "automatic": False,
                    },
                    "issues": [
                        {
                            "code": "missing_file",
                            "message": "case/0/p is missing.",
                            "severity": "error",
                        },
                        {
                            "code": "style",
                            "message": "Optional warning.",
                            "severity": "warning",
                        },
                    ],
                    "diagnostics": {
                        "severity": "failed",
                        "categories": ["output"],
                        "summary": "output failed",
                        "primary_issue": {"code": "missing_file", "message": "case/0/p is missing."},
                    },
                },
            ]

            manifest = ManifestWriter().write_planned(task, make_plan())
            written = read_written_manifest(task)
            gates = manifest["workflow"]["gates"]

            self.assertEqual("failed", gates["status"])
            self.assertEqual(2, gates["total"])
            self.assertEqual(1, gates["passed"])
            self.assertEqual(1, gates["failed"])
            self.assertEqual("capability", gates["reviews"][0]["gate"])
            self.assertEqual({"mode": "reference_modify", "reason": "test"}, gates["reviews"][0]["decision"])
            self.assertEqual(2, gates["reviews"][1]["issue_count"])
            self.assertEqual(1, gates["reviews"][1]["error_count"])
            self.assertEqual(1, gates["reviews"][1]["warning_count"])
            self.assertEqual("regenerate_case", gates["reviews"][1]["next_repair_action"]["id"])
            self.assertEqual("output failed", gates["reviews"][1]["diagnostics"]["summary"])
            self.assertFalse(written["appflow_hints"]["import_ready"])
            self.assertTrue(written["appflow_hints"]["import_blockers"])

    def test_manifest_gate_summary_is_pending_when_no_reviews_exist(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            manifest = ManifestWriter().write_pending(task)
            written = read_written_manifest(task)
            gates = manifest["workflow"]["gates"]

            self.assertEqual("pending", gates["status"])
            self.assertEqual(0, gates["total"])
            self.assertEqual([], gates["reviews"])
            self.assertFalse(written["appflow_hints"]["import_ready"])
            self.assertTrue(written["appflow_hints"]["import_blockers"])

    def test_manifest_includes_repair_history(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            task.repair_history = [
                {
                    "repair_action_id": "regenerate_case",
                    "action_id": "regenerate_case",
                    "label": "Regenerate case",
                    "status": "recorded",
                    "repair_mode": "manual",
                    "source_gate": "static_validation",
                    "related_diagnostic_codes": ["validation.missing_file"],
                    "related_diagnostic_categories": ["validation"],
                    "created_at": "2026-05-23T00:00:00+08:00",
                }
            ]

            manifest = ManifestWriter().write_pending(task)

        self.assertEqual(task.repair_history, manifest["workflow"]["repair_history"])
        self.assertEqual("regenerate_case", manifest["workflow"]["last_repair_action"]["repair_action_id"])
        self.assertEqual("recorded", manifest["workflow"]["last_repair_action"]["status"])
        self.assertEqual("manual", manifest["workflow"]["last_repair_action"]["repair_mode"])
        self.assertEqual(["validation.missing_file"], manifest["workflow"]["last_repair_action"]["related_diagnostic_codes"])
        self.assertEqual("unknown", manifest["workflow"]["repair_followup"]["status"])
        self.assertTrue(manifest["workflow"]["last_repair_action"]["should_rerun"])
        self.assertTrue(manifest["appflow_hints"]["offer_rerun"])

    def test_manifest_last_repair_action_summarizes_patch_result(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            task.repair_history = [
                {
                    "repair_action_id": "stabilize_time_step",
                    "action_id": "stabilize_time_step",
                    "label": "Stabilize time step",
                    "status": "applied",
                    "repair_mode": "executable",
                    "source_gate": "result_review",
                    "related_diagnostic_codes": ["result.courant_high"],
                    "related_diagnostic_categories": ["numerics"],
                    "patch_result": {
                        "status": "applied",
                        "applied": [{"op": "adjust_time_step"}],
                        "skipped": [],
                    },
                    "created_at": "2026-05-23T00:00:00+08:00",
                }
            ]

            manifest = ManifestWriter().write_pending(task)

        summary = manifest["workflow"]["last_repair_action"]
        self.assertEqual("stabilize_time_step", summary["repair_action_id"])
        self.assertEqual("executable", summary["repair_mode"])
        self.assertEqual({"status": "applied", "applied_count": 1, "skipped_count": 0}, summary["patch_result"])

    def test_manifest_repair_followup_states(self) -> None:
        cases = [
            ("resolved", ["result.courant_high"], []),
            ("unresolved", [], ["result.courant_high"]),
            ("unknown", [], []),
        ]
        for status, resolved_codes, unresolved_codes in cases:
            with self.subTest(status=status):
                with tempfile.TemporaryDirectory() as tmp:
                    task = make_task(Path(tmp))
                    task.repair_history = [
                        {
                            "repair_action_id": "stabilize_time_step",
                            "action_id": "stabilize_time_step",
                            "status": "applied",
                            "repair_mode": "executable",
                            "related_diagnostic_codes": ["result.courant_high"],
                            "repair_followup": {
                                "status": status,
                                "resolved_codes": resolved_codes,
                                "unresolved_codes": unresolved_codes,
                                "checked_at": "2026-05-23T01:00:00+08:00",
                            },
                        }
                    ]

                    manifest = ManifestWriter().write_pending(task)

                followup = manifest["workflow"]["repair_followup"]
                self.assertEqual(status, followup["status"])
                self.assertEqual(resolved_codes, followup["resolved_codes"])
                self.assertEqual(unresolved_codes, followup["unresolved_codes"])

    def test_manifest_repair_followup_unknown_without_last_repair(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            manifest = ManifestWriter().write_pending(task)

        self.assertEqual("unknown", manifest["workflow"]["repair_followup"]["status"])
        self.assertEqual({}, manifest["workflow"]["last_repair_action"])

    def test_run_manifest_requests_docker_foam_to_vtk_for_raw_openfoam_results(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            case_dir = Path(task.case_dir)
            (case_dir / "constant" / "polyMesh").mkdir(parents=True)
            (case_dir / "0.5").mkdir(parents=True)
            logs_dir = Path(task.logs_dir)
            logs_dir.mkdir(parents=True)
            (logs_dir / "icoFoam.log").write_text("solver log\n", encoding="utf-8")

            manifest = ManifestWriter().write_run_result(
                task,
                make_plan(),
                {
                    "status": "run_completed",
                    "reason": "completed",
                    "logs": [{"path": "logs/icoFoam.log", "role": "icoFoam", "format": "text"}],
                    "outputs": {
                        "results": {
                            "latest_path": "case/0.5",
                            "latest_time": "0.5",
                        }
                    },
                },
            )
            written = read_written_manifest(task)

            self.assertEqual("succeeded", manifest["workflow"]["status"])
            self.assertEqual("openfoam", manifest["solver"]["family"])
            self.assertEqual("icoFoam", manifest["solver"]["command"])
            self.assertTrue(manifest["appflow_hints"]["run_foam_to_vtk"])
            self.assertTrue(manifest["appflow_hints"]["open_paraview"])
            self.assertEqual("docker", manifest["appflow_hints"]["foam_to_vtk_backend"])
            self.assertEqual("first_vtk", manifest["appflow_hints"]["paraview_open_mode"])
            self.assertEqual("icoFoam", manifest["artifacts"]["logs"][0]["type"])
            self.assertEqual("icoFoam", manifest["artifacts"]["logs"][0]["name"])
            self.assertEqual(0.5, manifest["solver_settings"]["time"]["end_time"])
            self.assertEqual(0.01, manifest["solver_settings"]["physics"]["nu"])

            validation = validate_appflow_manifest(manifest, task)

            self.assertEqual("valid", validation["status"])
            self.assertTrue(validation["import_ready"])
            self.assertEqual([], validation["import_blockers"])
            self.assertTrue(written["appflow_hints"]["import_ready"])
            self.assertEqual([], written["appflow_hints"]["import_blockers"])
            self.assertEqual("passed", _manifest_gate_review(validation)["status"])

    def test_run_manifest_uses_existing_vtk_without_requesting_export(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            vtk_dir = Path(task.case_dir) / "VTK"
            vtk_dir.mkdir(parents=True)
            (vtk_dir / "case_0.vtk").write_text("# vtk\n", encoding="utf-8")

            manifest = ManifestWriter().write_run_result(
                task,
                make_plan(),
                {
                    "status": "run_completed",
                    "reason": "completed",
                    "logs": [],
                    "outputs": {"results": {"latest_path": "case/0.5"}},
                },
            )

            self.assertFalse(manifest["appflow_hints"]["run_foam_to_vtk"])
            self.assertFalse(manifest["appflow_hints"]["export_vtk"])
            self.assertEqual("first_vtk", manifest["appflow_hints"]["paraview_open_mode"])

    def test_run_manifest_keeps_runtime_info_without_result_hints_on_blocked_run(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            Path(task.case_dir).mkdir(parents=True)
            (Path(task.case_dir) / "constant" / "polyMesh").mkdir(parents=True)

            manifest = ManifestWriter().write_run_result(
                task,
                make_plan(),
                {
                    "status": "run_blocked",
                    "reason": "Required OpenFOAM command is missing: blockMesh.",
                    "logs": [],
                    "runtime_info": {
                        "mode": "unavailable",
                        "backend": "local",
                        "available": False,
                        "reason": "Required OpenFOAM command is missing: blockMesh.",
                        "missing_commands": ["blockMesh"],
                        "wm_project_dir": "",
                        "commands": {
                            "blockMesh": {"available": False},
                            "icoFoam": {"available": True},
                        },
                    },
                },
            )
            written = read_written_manifest(task)

            self.assertEqual("failed", manifest["workflow"]["status"])
            self.assertEqual("run_blocked", manifest["workflow"]["run"]["status"])
            self.assertEqual("local", manifest["workflow"]["run"]["runtime_info"]["backend"])
            self.assertEqual(["blockMesh"], manifest["workflow"]["run"]["runtime_info"]["missing_commands"])
            self.assertFalse(manifest["appflow_hints"]["show_results"])
            self.assertFalse(manifest["appflow_hints"]["open_paraview"])
            self.assertFalse(manifest["appflow_hints"]["run_foam_to_vtk"])
            self.assertEqual("local", manifest["appflow_hints"]["runtime_backend"])
            self.assertFalse(manifest["appflow_hints"]["runtime_available"])

            validation = validate_appflow_manifest(manifest, task)

            self.assertEqual("valid", validation["status"])
            self.assertFalse(validation["import_ready"])
            blocker_codes = {item["code"] for item in validation["import_blockers"]}
            self.assertIn("manifest.workflow_not_succeeded", blocker_codes)
            self.assertIn("manifest.run_not_completed", blocker_codes)
            self.assertFalse(written["appflow_hints"]["import_ready"])
            manifest_gate = _manifest_gate_review(validation)
            self.assertEqual("failed", manifest_gate["status"])
            self.assertIn("manifest.run_not_completed", {item["code"] for item in manifest_gate["issues"]})

    def test_run_manifest_rejects_unsupported_solver_and_missing_command(self) -> None:
        cases = [
            ("calculix", "icoFoam", "manifest.unsupported_solver"),
            ("openfoam", "", "manifest.missing_solver_command"),
        ]
        for solver_family, solver_command, expected_code in cases:
            with self.subTest(expected_code=expected_code), tempfile.TemporaryDirectory() as tmp:
                task = make_task(Path(tmp))
                task.solver_family = solver_family
                (Path(task.case_dir) / "constant" / "polyMesh").mkdir(parents=True)
                plan = make_plan()
                plan.solver_name = solver_command

                ManifestWriter().write_run_result(
                    task,
                    plan,
                    {
                        "status": "run_completed",
                        "reason": "completed",
                        "logs": [],
                        "outputs": {"results": {}},
                    },
                )
                written = read_written_manifest(task)
                validation = validate_appflow_manifest(written, task)

                self.assertEqual("valid", validation["status"])
                self.assertFalse(validation["import_ready"])
                self.assertIn(expected_code, {item["code"] for item in validation["import_blockers"]})
                self.assertFalse(written["appflow_hints"]["import_ready"])

    def test_run_manifest_with_missing_artifact_paths_is_invalid_and_not_ready(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            ManifestWriter().write_run_result(
                task,
                make_plan(),
                {
                    "status": "run_completed",
                    "reason": "completed",
                    "logs": [],
                    "outputs": {"results": {}},
                },
            )
            written = read_written_manifest(task)
            validation = validate_appflow_manifest(written, task)

            self.assertEqual("invalid", validation["status"])
            self.assertFalse(validation["import_ready"])
            self.assertIn(
                "manifest.invalid_structure",
                {item["code"] for item in validation["import_blockers"]},
            )
            self.assertFalse(written["appflow_hints"]["import_ready"])

    def test_run_manifest_preserves_docker_runtime_hints(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            case_dir = Path(task.case_dir)
            (case_dir / "constant" / "polyMesh").mkdir(parents=True)
            (case_dir / "0.5").mkdir(parents=True)

            manifest = ManifestWriter().write_run_result(
                task,
                make_plan(),
                {
                    "status": "run_completed",
                    "reason": "completed",
                    "logs": [],
                    "outputs": {"results": {"latest_path": "case/0.5", "latest_time": "0.5"}},
                    "runtime_info": {
                        "mode": "docker",
                        "backend": "docker",
                        "available": True,
                        "reason": "Docker OpenFOAM backend available with image foam:test.",
                        "missing_commands": [],
                        "wm_project_dir": "",
                        "docker_image": "foam:test",
                        "docker_case_dir": "/case",
                        "commands": {"blockMesh": {"available": True}},
                    },
                },
            )

            self.assertEqual("docker", manifest["workflow"]["run"]["runtime_info"]["backend"])
            self.assertEqual("foam:test", manifest["workflow"]["run"]["runtime_info"]["docker_image"])
            self.assertEqual("/case", manifest["workflow"]["run"]["runtime_info"]["docker_case_dir"])
            self.assertEqual("docker", manifest["appflow_hints"]["runtime_backend"])
            self.assertTrue(manifest["appflow_hints"]["runtime_available"])
            self.assertEqual("foam:test", manifest["appflow_hints"]["docker_image"])
            self.assertEqual("/case", manifest["appflow_hints"]["docker_case_dir"])
            self.assertTrue(manifest["appflow_hints"]["show_results"])

    def test_run_manifest_includes_run_diagnostics_and_repair_hint_flag(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            task.gate_reviews = [
                {
                    "gate": "result_review",
                    "status": "failed",
                    "issues": [{"code": "result.courant_high", "message": "max Co high", "severity": "error"}],
                    "result_review": {
                        "status": "failed",
                        "summary": "Maximum Courant number 3.2 is above 1.",
                        "items": [{"code": "result.courant_high", "message": "max Co high", "severity": "error"}],
                        "can_show_results": False,
                        "should_offer_repair": True,
                        "should_offer_rerun": True,
                        "should_offer_vtk": False,
                        "should_offer_paraview": False,
                    },
                    "next_repair_action": {
                        "id": "stabilize_time_step",
                        "label": "Stabilize time step",
                        "description": "test",
                        "automatic": False,
                    },
                }
            ]

            manifest = ManifestWriter().write_run_result(
                task,
                make_plan(),
                {
                    "status": "run_failed",
                    "reason": "OpenFOAM run completed with error diagnostics.",
                    "logs": [{"path": "logs/icoFoam.log", "role": "icoFoam", "format": "text"}],
                    "diagnostics": {
                        "severity": "failed",
                        "summary": "Maximum Courant number 3.2 is above 1.",
                        "items": [
                            {
                                "code": "result.courant_high",
                                "severity": "error",
                                "category": "numerics",
                                "message": "Maximum Courant number 3.2 is above 1.",
                                "source": "solver",
                                "log_file": "logs/icoFoam.log",
                                "matched_line": "Courant Number mean: 0.2 max: 3.2",
                                "repair_hint": "Enable adjustTimeStep or reduce deltaT before rerunning.",
                            }
                        ],
                        "metrics": {"max_courant": 3.2},
                    },
                },
            )
            written = read_written_manifest(task)

        run = manifest["workflow"]["run"]
        self.assertEqual("failed", run["result_review"]["status"])
        self.assertEqual("failed", run["diagnostics"]["severity"])
        self.assertEqual("result.courant_high", run["diagnostics"]["items"][0]["code"])
        self.assertEqual(3.2, run["diagnostics"]["metrics"]["max_courant"])
        self.assertEqual("failed", manifest["appflow_hints"]["run_diagnostics_severity"])
        self.assertTrue(manifest["appflow_hints"]["has_run_diagnostics"])
        self.assertTrue(manifest["appflow_hints"]["offer_repair"])
        self.assertTrue(manifest["appflow_hints"]["offer_rerun"])
        self.assertTrue(manifest["appflow_hints"]["has_suggested_repair_actions"])
        self.assertFalse(written["appflow_hints"]["import_ready"])
        blocker_codes = {item["code"] for item in written["appflow_hints"]["import_blockers"]}
        self.assertIn("manifest.run_not_completed", blocker_codes)
        self.assertIn("manifest.repair_required", blocker_codes)

    def test_run_manifest_result_hints_are_driven_by_result_review(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            (Path(task.case_dir) / "0.5").mkdir(parents=True)
            task.gate_reviews = [
                {
                    "gate": "result_review",
                    "status": "failed",
                    "issues": [{"code": "result.residual_nan", "message": "nan", "severity": "error"}],
                    "result_review": {
                        "status": "failed",
                        "summary": "numerics failed",
                        "items": [{"code": "result.residual_nan", "message": "nan", "severity": "error"}],
                        "can_show_results": False,
                        "should_offer_repair": True,
                        "should_offer_rerun": True,
                        "should_offer_vtk": False,
                        "should_offer_paraview": False,
                        "repair_action_id": "inspect_solver_numerics",
                        "repair_mode": "executable",
                    },
                    "next_repair_action": {
                        "id": "inspect_solver_numerics",
                        "label": "Inspect unstable numerics",
                        "description": "test",
                        "automatic": False,
                    },
                }
            ]

            manifest = ManifestWriter().write_run_result(
                task,
                make_plan(),
                {
                    "status": "run_completed",
                    "reason": "completed",
                    "logs": [],
                    "outputs": {"results": {"latest_path": "case/0.5", "latest_time": "0.5"}},
                },
            )
            written = read_written_manifest(task)

        self.assertEqual("failed", manifest["workflow"]["status"])
        self.assertFalse(manifest["appflow_hints"]["show_results"])
        self.assertFalse(manifest["appflow_hints"]["open_paraview"])
        self.assertFalse(manifest["appflow_hints"]["run_foam_to_vtk"])
        self.assertTrue(manifest["appflow_hints"]["offer_repair"])
        self.assertTrue(manifest["appflow_hints"]["offer_rerun"])
        self.assertFalse(written["appflow_hints"]["import_ready"])
        blocker_codes = {item["code"] for item in written["appflow_hints"]["import_blockers"]}
        self.assertIn("manifest.workflow_not_succeeded", blocker_codes)
        self.assertIn("manifest.repair_required", blocker_codes)


if __name__ == "__main__":
    unittest.main()
