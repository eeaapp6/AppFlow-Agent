from dataclasses import dataclass
import os
import shutil
import subprocess
from pathlib import Path
from typing import Any, Protocol


OPENFOAM_COMMANDS = ["blockMesh", "checkMesh", "icoFoam", "simpleFoam", "gmshToFoam", "foamToVTK"]
BACKEND_ENV_VAR = "SIMAGENT_OPENFOAM_BACKEND"
DOCKER_IMAGE_ENV_VAR = "SIMAGENT_OPENFOAM_DOCKER_IMAGE"
DOCKER_CASE_DIR_ENV_VAR = "SIMAGENT_OPENFOAM_DOCKER_CASE_DIR"
DEFAULT_DOCKER_IMAGE = "leoyue123/foamagent:latest"
DEFAULT_DOCKER_CASE_DIR = "/case"
SUPPORTED_BACKENDS = {"local", "docker", "wsl"}


class CommandProbe(Protocol):
    def exists(self, command: str) -> bool:
        ...


class CommandExecutor(Protocol):
    def run(self, command: list[str], cwd: Path, log_path: Path, timeout_seconds: int) -> dict[str, Any]:
        ...


@dataclass
class ShutilCommandProbe:
    def exists(self, command: str) -> bool:
        return shutil.which(command) is not None


@dataclass
class SubprocessCommandExecutor:
    def run(self, command: list[str], cwd: Path, log_path: Path, timeout_seconds: int) -> dict[str, Any]:
        log_path.parent.mkdir(parents=True, exist_ok=True)
        with log_path.open("w", encoding="utf-8", errors="replace") as log:
            process = subprocess.run(
                command,
                cwd=str(cwd),
                stdout=log,
                stderr=subprocess.STDOUT,
                text=True,
                timeout=timeout_seconds,
            )

        return {
            "command": " ".join(command),
            "return_code": process.returncode,
            "log": log_path.name,
            "log_path": str(log_path),
        }


def selected_openfoam_backend(env: dict[str, str] | None = None) -> str:
    env = env or os.environ
    backend = str(env.get(BACKEND_ENV_VAR, "local")).strip().lower() or "local"
    return backend if backend in SUPPORTED_BACKENDS else "local"


def detect_openfoam_runtime(
    *,
    required_commands: list[str] | None = None,
    backend: str | None = None,
    env: dict[str, str] | None = None,
    probe: CommandProbe | None = None,
) -> dict[str, Any]:
    env = env or os.environ
    probe = probe or ShutilCommandProbe()
    selected_backend = (backend or selected_openfoam_backend(env)).strip().lower()
    if selected_backend not in SUPPORTED_BACKENDS:
        selected_backend = "local"

    wm_project_dir = str(env.get("WM_PROJECT_DIR", "")).strip()

    if selected_backend == "local":
        required = sorted(set(required_commands or []))
        command_names = sorted({*OPENFOAM_COMMANDS, *required})
        commands = {command: {"available": probe.exists(command)} for command in command_names}
        missing_required = [command for command in required if not commands[command]["available"]]
        return _local_runtime_info(commands, missing_required, wm_project_dir)
    if selected_backend == "docker":
        return _docker_runtime_info(
            docker_available=probe.exists("docker"),
            required_commands=required_commands or [],
            env=env,
        )
    commands = {command: {"available": probe.exists(command)} for command in OPENFOAM_COMMANDS}
    return _blocked_backend_info(
        backend="wsl",
        tool_available=probe.exists("wsl"),
        tool_name="wsl",
        commands=commands,
        missing_required=[],
        wm_project_dir=wm_project_dir,
    )


def _local_runtime_info(
    commands: dict[str, dict[str, bool]],
    missing_required: list[str],
    wm_project_dir: str,
) -> dict[str, Any]:
    missing = list(missing_required)
    if not wm_project_dir:
        reason = "OpenFOAM runtime is not configured. WM_PROJECT_DIR is not set."
        mode = "unavailable"
        available = False
    elif missing:
        reason = f"Required OpenFOAM command is missing: {missing[0]}."
        mode = "unavailable"
        available = False
    else:
        reason = "Local OpenFOAM runtime is available."
        mode = "local"
        available = True

    return {
        "mode": mode,
        "backend": "local",
        "available": available,
        "reason": reason,
        "wm_project_dir": wm_project_dir,
        "commands": commands,
        "missing_commands": missing,
    }


def _docker_runtime_info(
    *,
    docker_available: bool,
    required_commands: list[str],
    env: dict[str, str],
) -> dict[str, Any]:
    image = str(env.get(DOCKER_IMAGE_ENV_VAR, DEFAULT_DOCKER_IMAGE)).strip() or DEFAULT_DOCKER_IMAGE
    container_case_dir = str(env.get(DOCKER_CASE_DIR_ENV_VAR, DEFAULT_DOCKER_CASE_DIR)).strip() or DEFAULT_DOCKER_CASE_DIR
    command_names = sorted({*OPENFOAM_COMMANDS, *required_commands})
    commands = {
        command: {
            "available": docker_available,
            "source": "docker_image",
        }
        for command in command_names
    }
    missing = [] if docker_available else ["docker"]
    reason = (
        f"Docker OpenFOAM backend available with image {image}."
        if docker_available
        else "Docker OpenFOAM backend selected but docker command is not available."
    )
    return {
        "mode": "docker",
        "backend": "docker",
        "available": docker_available,
        "reason": reason,
        "wm_project_dir": "",
        "commands": commands,
        "missing_commands": missing,
        "docker_available": docker_available,
        "docker_image": image,
        "docker_case_dir": container_case_dir,
    }


def _blocked_backend_info(
    *,
    backend: str,
    tool_available: bool,
    tool_name: str,
    commands: dict[str, dict[str, bool]],
    missing_required: list[str],
    wm_project_dir: str,
) -> dict[str, Any]:
    reason = (
        f"{backend} backend detection found {tool_name}, but OpenFOAM case execution through "
        f"{backend} is not implemented in this backend yet."
        if tool_available
        else f"{backend} backend was selected but {tool_name} is not available."
    )
    return {
        "mode": backend,
        "backend": backend,
        "available": False,
        "reason": reason,
        "wm_project_dir": wm_project_dir,
        "commands": commands,
        "missing_commands": list(missing_required),
        f"{tool_name}_available": tool_available,
    }


class OpenFOAMRunner:
    def __init__(self, executor: CommandExecutor | None = None) -> None:
        self.executor = executor or SubprocessCommandExecutor()

    def run_step(self, command: list[str], case_dir: Path, log_path: Path, timeout_seconds: int) -> dict[str, Any]:
        return self.executor.run(command, case_dir, log_path, timeout_seconds)


@dataclass
class DockerCommandExecutor:
    image: str = DEFAULT_DOCKER_IMAGE
    container_case_dir: str = DEFAULT_DOCKER_CASE_DIR
    docker_command: str = "docker"

    def run(self, command: list[str], cwd: Path, log_path: Path, timeout_seconds: int) -> dict[str, Any]:
        docker_command = self._docker_run_command(command, cwd)
        log_path.parent.mkdir(parents=True, exist_ok=True)
        with log_path.open("w", encoding="utf-8", errors="replace") as log:
            process = subprocess.run(
                docker_command,
                stdout=log,
                stderr=subprocess.STDOUT,
                text=True,
                timeout=timeout_seconds,
            )

        return {
            "command": " ".join(command),
            "return_code": process.returncode,
            "log": log_path.name,
            "log_path": str(log_path),
            "backend": "docker",
            "docker_image": self.image,
            "docker_command": docker_command,
        }

    def _docker_run_command(self, command: list[str], case_dir: Path) -> list[str]:
        return [
            self.docker_command,
            "run",
            "--rm",
            "-v",
            f"{case_dir.resolve()}:{self.container_case_dir}",
            "-w",
            self.container_case_dir,
            self.image,
            *command,
        ]


class DockerOpenFOAMRunner(OpenFOAMRunner):
    def __init__(
        self,
        *,
        image: str = DEFAULT_DOCKER_IMAGE,
        container_case_dir: str = DEFAULT_DOCKER_CASE_DIR,
        executor: CommandExecutor | None = None,
    ) -> None:
        super().__init__(executor or DockerCommandExecutor(image=image, container_case_dir=container_case_dir))

    @classmethod
    def from_runtime_info(cls, runtime_info: dict[str, Any]) -> "DockerOpenFOAMRunner":
        image = str(runtime_info.get("docker_image", DEFAULT_DOCKER_IMAGE)).strip() or DEFAULT_DOCKER_IMAGE
        case_dir = str(runtime_info.get("docker_case_dir", DEFAULT_DOCKER_CASE_DIR)).strip() or DEFAULT_DOCKER_CASE_DIR
        return cls(image=image, container_case_dir=case_dir)
