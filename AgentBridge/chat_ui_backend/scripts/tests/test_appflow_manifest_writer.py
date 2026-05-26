import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.manifest.validator import validate_appflow_manifest
from scripts.simagent_core.manifest.writer import ManifestWriter
from scripts.simagent_core.models import SimulationPlan, TaskContext


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

    def test_manifest_gate_summary_is_pending_when_no_reviews_exist(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))

            manifest = ManifestWriter().write_pending(task)
            gates = manifest["workflow"]["gates"]

            self.assertEqual("pending", gates["status"])
            self.assertEqual(0, gates["total"])
            self.assertEqual([], gates["reviews"])

    def test_manifest_includes_repair_history(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            task.repair_history = [
                {
                    "action_id": "regenerate_case",
                    "label": "Regenerate case",
                    "status": "accepted",
                    "source_gate": "static_validation",
                    "created_at": "2026-05-23T00:00:00+08:00",
                }
            ]

            manifest = ManifestWriter().write_pending(task)

        self.assertEqual(task.repair_history, manifest["workflow"]["repair_history"])

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


if __name__ == "__main__":
    unittest.main()
