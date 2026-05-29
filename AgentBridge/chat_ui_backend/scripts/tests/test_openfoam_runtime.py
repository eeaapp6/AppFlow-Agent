import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.simagent_core.models import PlannedFile, SimulationPlan, TaskContext
from scripts.simagent_core.solvers.openfoam.run_local import run_openfoam_case
from scripts.simagent_core.solvers.openfoam.runtime import DockerCommandExecutor, detect_openfoam_runtime, selected_openfoam_backend


class FakeProbe:
    def __init__(self, available: set[str] | None = None) -> None:
        self.available = available or set()

    def exists(self, command: str) -> bool:
        return command in self.available


class FakeRunner:
    def __init__(self, results: list[dict] | None = None, timeout: bool = False) -> None:
        self.results = results or []
        self.timeout = timeout
        self.commands: list[list[str]] = []

    def run_step(self, command: list[str], case_dir: Path, log_path: Path, timeout_seconds: int) -> dict:
        self.commands.append(command)
        log_path.parent.mkdir(parents=True, exist_ok=True)
        log_path.write_text(f"{command[0]} log\n", encoding="utf-8")
        if self.timeout:
            raise subprocess.TimeoutExpired(command, timeout_seconds)
        if self.results:
            return self.results.pop(0)
        return {
            "command": " ".join(command),
            "return_code": 0,
            "log": log_path.name,
            "log_path": str(log_path),
        }


def make_task(root: Path) -> TaskContext:
    task_dir = root / "task_runtime"
    return TaskContext(
        version=1,
        task_id="task_runtime",
        status="validated",
        user_requirement="run openfoam",
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


def make_plan() -> SimulationPlan:
    return SimulationPlan(
        solver_family="openfoam",
        solver_name="icoFoam",
        physics_domain="incompressible",
        case_category="cavity",
        case_name="cavity",
        description="runtime test",
        planned_files=[
            PlannedFile("mesh", "case/system/blockMeshDict", "openfoam-dict"),
            PlannedFile("control", "case/system/controlDict", "openfoam-dict"),
            PlannedFile("numerics", "case/system/fvSchemes", "openfoam-dict"),
            PlannedFile("linear_solver", "case/system/fvSolution", "openfoam-dict"),
            PlannedFile("physical_properties", "case/constant/physicalProperties", "openfoam-dict"),
            PlannedFile("initial_condition", "case/0/U", "openfoam-dict"),
            PlannedFile("initial_condition", "case/0/p", "openfoam-dict"),
        ],
    )


def write_required_case_files(task: TaskContext, plan: SimulationPlan) -> None:
    for item in plan.planned_files:
        path = Path(task.task_dir) / item.path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("FoamFile{}\n", encoding="utf-8")


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


def available_docker_runtime_info() -> dict:
    return {
        "mode": "docker",
        "backend": "docker",
        "available": True,
        "reason": "Docker OpenFOAM backend available with image foam:test.",
        "wm_project_dir": "",
        "missing_commands": [],
        "docker_available": True,
        "docker_image": "foam:test",
        "docker_case_dir": "/case",
        "commands": {
            "blockMesh": {"available": True, "source": "docker_image"},
            "icoFoam": {"available": True, "source": "docker_image"},
        },
    }


class FakeCompletedProcess:
    def __init__(self, returncode: int) -> None:
        self.returncode = returncode


class OpenFOAMRuntimeTests(unittest.TestCase):
    def test_local_backend_is_default(self) -> None:
        self.assertEqual("local", selected_openfoam_backend({}))

    def test_runtime_detection_unavailable_without_wm_project_dir_and_commands(self) -> None:
        runtime = detect_openfoam_runtime(
            required_commands=["blockMesh", "icoFoam"],
            env={},
            probe=FakeProbe(),
        )

        self.assertEqual("unavailable", runtime["mode"])
        self.assertFalse(runtime["available"])
        self.assertEqual(["blockMesh", "icoFoam"], runtime["missing_commands"])
        self.assertIn("WM_PROJECT_DIR", runtime["reason"])

    def test_local_runtime_detection_reports_available_required_commands(self) -> None:
        runtime = detect_openfoam_runtime(
            required_commands=["blockMesh", "icoFoam"],
            env={"WM_PROJECT_DIR": "/opt/openfoam"},
            probe=FakeProbe({"blockMesh", "icoFoam"}),
        )

        self.assertEqual("local", runtime["mode"])
        self.assertEqual("local", runtime["backend"])
        self.assertTrue(runtime["available"])
        self.assertEqual([], runtime["missing_commands"])
        self.assertTrue(runtime["commands"]["blockMesh"]["available"])
        self.assertTrue(runtime["commands"]["icoFoam"]["available"])

    def test_run_blocks_when_required_command_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)
            runtime = detect_openfoam_runtime(
                required_commands=["blockMesh", "icoFoam"],
                env={"WM_PROJECT_DIR": "/opt/openfoam"},
                probe=FakeProbe({"icoFoam"}),
            )

            result = run_openfoam_case(task, plan, runtime_info=runtime)

        self.assertEqual("run_blocked", result["status"])
        self.assertIn("blockMesh", result["reason"])
        self.assertEqual(["blockMesh"], result["runtime_info"]["missing_commands"])

    def test_pipeline_gate_failure_takes_precedence_over_runtime_unavailable(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)
            runtime = {
                "mode": "unavailable",
                "backend": "local",
                "available": False,
                "reason": "OpenFOAM runtime is not configured. WM_PROJECT_DIR is not set.",
                "missing_commands": ["blockMesh"],
                "wm_project_dir": "",
                "commands": {"blockMesh": {"available": False}},
            }

            with patch(
                "scripts.simagent_core.solvers.openfoam.run_local._select_run_pipeline",
                return_value=(
                    [{"command": "rm", "args": [], "stage": "bad", "source": "test"}],
                    {"mode": "basic", "selected": "basic", "fallback_used": False, "warnings": []},
                ),
            ):
                result = run_openfoam_case(task, plan, runtime_info=runtime)

        self.assertEqual("run_failed", result["status"])
        self.assertEqual("Run pipeline failed execution gate.", result["reason"])
        self.assertEqual("failed", result["pipeline_info"]["execution_gate"]["status"])
        self.assertEqual(runtime, result["runtime_info"])

    def test_run_failed_when_command_returns_nonzero_with_log_info(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)
            runner = FakeRunner([
                {
                    "command": "blockMesh",
                    "return_code": 2,
                    "log": "blockMesh.log",
                    "log_path": str(Path(task.logs_dir) / "blockMesh.log"),
                }
            ])

            result = run_openfoam_case(task, plan, runtime_info=available_runtime_info(), runner=runner)

        self.assertEqual("run_failed", result["status"])
        self.assertIn("return code 2", result["reason"])
        self.assertEqual("blockMesh.log", result["logs"][0]["path"].split("/")[-1])
        self.assertEqual("blockMesh", result["steps"][0]["command"])

    def test_run_failed_when_command_times_out(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)

            result = run_openfoam_case(
                task,
                plan,
                timeout_seconds=3,
                runtime_info=available_runtime_info(),
                runner=FakeRunner(timeout=True),
            )

        self.assertEqual("run_failed", result["status"])
        self.assertIn("timed out after 3 seconds", result["reason"])
        self.assertEqual("logs/blockMesh.log", result["logs"][0]["path"])

    def test_docker_runtime_detection_reports_available_without_wm_project_dir(self) -> None:
        runtime = detect_openfoam_runtime(
            required_commands=["blockMesh"],
            env={
                "SIMAGENT_OPENFOAM_BACKEND": "docker",
                "SIMAGENT_OPENFOAM_DOCKER_IMAGE": "foam:test",
                "SIMAGENT_OPENFOAM_DOCKER_CASE_DIR": "/workspace",
            },
            probe=FakeProbe({"docker"}),
        )

        self.assertEqual("docker", runtime["mode"])
        self.assertEqual("docker", runtime["backend"])
        self.assertTrue(runtime["available"])
        self.assertTrue(runtime["docker_available"])
        self.assertEqual("foam:test", runtime["docker_image"])
        self.assertEqual("/workspace", runtime["docker_case_dir"])
        self.assertEqual([], runtime["missing_commands"])
        self.assertTrue(runtime["commands"]["blockMesh"]["available"])
        self.assertIn("Docker OpenFOAM backend available", runtime["reason"])

    def test_docker_runtime_detection_blocks_when_docker_is_missing(self) -> None:
        runtime = detect_openfoam_runtime(
            required_commands=["blockMesh"],
            backend="docker",
            env={},
            probe=FakeProbe(),
        )

        self.assertEqual("docker", runtime["backend"])
        self.assertFalse(runtime["available"])
        self.assertFalse(runtime["docker_available"])
        self.assertEqual(["docker"], runtime["missing_commands"])
        self.assertIn("docker command is not available", runtime["reason"])

    def test_docker_command_executor_builds_docker_run_without_shell(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            case_dir = Path(tmp) / "case"
            case_dir.mkdir()
            log_path = Path(tmp) / "logs" / "blockMesh.log"
            captured = []

            def fake_run(command, **kwargs):
                captured.append((command, kwargs))
                return FakeCompletedProcess(0)

            executor = DockerCommandExecutor(image="foam:test", container_case_dir="/case")
            with patch("scripts.simagent_core.solvers.openfoam.runtime.subprocess.run", side_effect=fake_run):
                result = executor.run(["blockMesh"], case_dir, log_path, 30)

        docker_command, kwargs = captured[0]
        self.assertIsInstance(docker_command, list)
        self.assertEqual(["docker", "run", "--rm"], docker_command[:3])
        self.assertEqual(["-w", "/case", "foam:test", "blockMesh"], docker_command[-4:])
        self.assertIn(f"{case_dir.resolve()}:/case", docker_command)
        self.assertNotIn("shell", kwargs)
        self.assertEqual("blockMesh", result["command"])
        self.assertEqual(docker_command, result["docker_command"])

    def test_run_openfoam_case_uses_docker_runner_successfully(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)
            calls = []

            def fake_run(command, **kwargs):
                calls.append(command)
                return FakeCompletedProcess(0)

            with patch("scripts.simagent_core.solvers.openfoam.runtime.subprocess.run", side_effect=fake_run):
                result = run_openfoam_case(task, plan, runtime_info=available_docker_runtime_info())

        self.assertEqual("run_completed", result["status"])
        self.assertEqual("docker", result["runtime_info"]["backend"])
        self.assertEqual(["blockMesh", "icoFoam"], [step["command"] for step in result["steps"]])
        self.assertEqual("foam:test", result["steps"][0]["docker_image"])
        self.assertEqual("docker", calls[0][0])
        self.assertIn("blockMesh", calls[0])
        self.assertIn("icoFoam", calls[1])

    def test_run_openfoam_case_blocks_when_docker_backend_is_unavailable(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)
            runtime = detect_openfoam_runtime(backend="docker", env={}, probe=FakeProbe())

            result = run_openfoam_case(task, plan, runtime_info=runtime)

        self.assertEqual("run_blocked", result["status"])
        self.assertIn("docker command is not available", result["reason"])
        self.assertEqual(["docker"], result["runtime_info"]["missing_commands"])

    def test_run_openfoam_case_reports_docker_nonzero_as_run_failed(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)

            with patch(
                "scripts.simagent_core.solvers.openfoam.runtime.subprocess.run",
                return_value=FakeCompletedProcess(4),
            ):
                result = run_openfoam_case(task, plan, runtime_info=available_docker_runtime_info())

        self.assertEqual("run_failed", result["status"])
        self.assertIn("return code 4", result["reason"])
        self.assertEqual("logs/blockMesh.log", result["logs"][0]["path"])
        self.assertEqual("docker", result["steps"][0]["backend"])

    def test_run_openfoam_case_reports_docker_timeout_as_run_failed(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)

            with patch(
                "scripts.simagent_core.solvers.openfoam.runtime.subprocess.run",
                side_effect=subprocess.TimeoutExpired(["docker", "run"], 7),
            ):
                result = run_openfoam_case(
                    task,
                    plan,
                    timeout_seconds=7,
                    runtime_info=available_docker_runtime_info(),
                )

        self.assertEqual("run_failed", result["status"])
        self.assertIn("timed out after 7 seconds", result["reason"])
        self.assertEqual("logs/blockMesh.log", result["logs"][0]["path"])

    def test_wsl_backend_remains_blocked_even_if_detected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp))
            plan = make_plan()
            write_required_case_files(task, plan)
            runtime = {
                "mode": "wsl",
                "backend": "wsl",
                "available": True,
                "reason": "WSL detected.",
                "wm_project_dir": "",
                "missing_commands": [],
                "commands": {},
                "wsl_available": True,
            }

            result = run_openfoam_case(task, plan, runtime_info=runtime)

        self.assertEqual("run_blocked", result["status"])
        self.assertIn("WSL", result["reason"])
        self.assertFalse(result["runtime_info"]["available"])


if __name__ == "__main__":
    unittest.main()
