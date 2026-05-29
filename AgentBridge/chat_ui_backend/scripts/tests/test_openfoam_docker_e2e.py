import os
import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.manifest.validator import validate_appflow_manifest
from scripts.simagent_core.manifest.writer import ManifestWriter
from scripts.simagent_core.models import TaskContext
from scripts.simagent_core.solvers.openfoam.generated import create_rect_channel_plan
from scripts.simagent_core.solvers.openfoam.input_writer import generate_openfoam_files
from scripts.simagent_core.solvers.openfoam.run_local import run_openfoam_case
from scripts.simagent_core.solvers.openfoam.runtime import detect_openfoam_runtime, selected_openfoam_backend
from scripts.simagent_core.solvers.openfoam.validate import validate_openfoam_case


RECT_CHANNEL_PROMPT = "Generate a small laminar rectangular channel with length=1, height=0.2, velocity=0.1, endTime=0.02, deltaT=0.01"
DOCKER_INFRASTRUCTURE_MARKERS = [
    "cannot connect to the docker daemon",
    "error during connect",
    "permission denied while trying to connect",
    "unable to find image",
    "pull access denied",
    "manifest unknown",
    "no matching manifest",
    "repository does not exist",
    "not found: manifest",
]


def make_task(root: Path) -> TaskContext:
    task_dir = root / "task_docker_e2e"
    return TaskContext(
        version=1,
        task_id="task_docker_e2e",
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


class OpenFOAMDockerE2ETests(unittest.TestCase):
    def setUp(self) -> None:
        if os.environ.get("SIMAGENT_OPENFOAM_E2E", "").strip() != "1":
            self.skipTest("Set SIMAGENT_OPENFOAM_E2E=1 to run Docker OpenFOAM E2E smoke test.")
        if selected_openfoam_backend() != "docker":
            self.skipTest("Set SIMAGENT_OPENFOAM_BACKEND=docker to run Docker OpenFOAM E2E smoke test.")

    def test_generated_rect_channel_runs_through_docker_backend(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            task = make_task(root)
            plan = create_rect_channel_plan(RECT_CHANNEL_PROMPT)
            generate_openfoam_files(task, plan)
            validation = validate_openfoam_case(task, plan)
            self.assertEqual("validated", validation["status"])

            runtime_info = detect_openfoam_runtime(required_commands=["blockMesh", plan.solver_name])
            if not runtime_info.get("available", False):
                self.skipTest(runtime_info.get("reason", "Docker OpenFOAM runtime is unavailable."))

            run_result = run_openfoam_case(task, plan, timeout_seconds=120, runtime_info=runtime_info)
            if _is_skippable_docker_failure(task, run_result):
                self.skipTest(run_result.get("reason", "Docker image or daemon is unavailable."))

            self.assertEqual("run_completed", run_result["status"])
            self.assertEqual("docker", run_result["runtime_info"]["backend"])
            self.assertTrue((Path(task.logs_dir) / "blockMesh.log").is_file())
            self.assertTrue((Path(task.logs_dir) / f"{plan.solver_name}.log").is_file())

            manifest = ManifestWriter().write_run_result(task, plan, run_result)
            manifest_validation = validate_appflow_manifest(manifest, task)

            self.assertEqual("valid", manifest_validation["status"])
            self.assertEqual("docker", manifest["workflow"]["run"]["runtime_info"]["backend"])
            self.assertEqual("docker", manifest["appflow_hints"]["runtime_backend"])
            self.assertTrue(manifest["appflow_hints"]["runtime_available"])


def _is_skippable_docker_failure(task: TaskContext, run_result: dict) -> bool:
    if run_result.get("status") == "run_blocked":
        return True
    if run_result.get("status") != "run_failed":
        return False
    content = _combined_log_content(task, run_result).lower()
    return any(marker in content for marker in DOCKER_INFRASTRUCTURE_MARKERS)


def _combined_log_content(task: TaskContext, run_result: dict) -> str:
    chunks = [str(run_result.get("reason", ""))]
    for item in run_result.get("logs", []):
        if not isinstance(item, dict):
            continue
        rel_path = str(item.get("path", "")).strip()
        if not rel_path:
            continue
        path = Path(task.task_dir) / rel_path
        if path.is_file():
            chunks.append(path.read_text(encoding="utf-8", errors="replace"))
    return "\n".join(chunks)


if __name__ == "__main__":
    unittest.main()
