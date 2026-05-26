import os
import re
from pathlib import Path
from typing import Any

from ...models import SimulationPlan
from .mesh import GMSH_MESH_FILE_NAME, plan_uses_external_gmsh_mesh


ALLOWED_ENV_VARS = {"FOAM_TUTORIALS", "WM_PROJECT_DIR", "FOAM_CASE"}
DENIED_COMMANDS = {
    "rm",
    "cp",
    "mv",
    "ln",
    "chmod",
    "sed",
    "awk",
    "python",
    "python3",
    "bash",
    "sh",
    "mpirun",
    "runParallel",
}

SHELL_TOKENS = {"|", "||", "&&", ";", ">", ">>", "<", "`"}
RUN_APPLICATION_OPTIONS_WITH_VALUE = {"-s", "-a", "-o", "-prefix"}
RUN_APPLICATION_FLAGS = {"-append", "-overwrite"}
PATH_ARGUMENT_FLAGS = {"-case", "-dict"}


def _solver_command_from_plan(plan: SimulationPlan | None) -> str:
    solver_name = plan.solver_name.strip() if plan and plan.solver_name else ""
    return solver_name or "icoFoam"


def build_basic_run_pipeline(plan: SimulationPlan | None = None) -> list[dict[str, Any]]:
    if plan and plan_uses_external_gmsh_mesh(plan.parameters):
        return _external_gmsh_pipeline(plan)
    return [
        {
            "command": "blockMesh",
            "args": [],
            "stage": "mesh",
            "source": "basic",
        },
        {
            "command": _solver_command_from_plan(plan),
            "args": [],
            "stage": "solve",
            "source": "basic",
        },
    ]


def _external_gmsh_pipeline(plan: SimulationPlan) -> list[dict[str, Any]]:
    return [
        {
            "command": "gmshToFoam",
            "args": [GMSH_MESH_FILE_NAME],
            "stage": "mesh",
            "source": "basic",
        },
        {
            "command": "checkMesh",
            "args": [],
            "stage": "mesh_check",
            "source": "basic",
        },
        {
            "command": _solver_command_from_plan(plan),
            "args": [],
            "stage": "solve",
            "source": "basic",
        },
    ]


def pipeline_step_to_command(step: dict[str, Any]) -> list[str]:
    command = str(step.get("command", "")).strip()
    if not command:
        raise ValueError("Run pipeline step has no command.")

    args = step.get("args", [])
    if not isinstance(args, list):
        raise ValueError(f"Run pipeline args must be a list for command: {command}")

    return [command, *[str(item) for item in args]]


def run_pipeline_mode() -> str:
    mode = os.environ.get("SIMAGENT_RUN_PIPELINE_MODE", "basic").strip().lower()
    return mode if mode in {"basic", "allrun_safe"} else "basic"


def _stage_for_command(command: str) -> str:
    lowered = command.lower()
    if "mesh" in lowered or command in {"surfaceFeatures", "topoSet", "createPatch", "setFields"}:
        return "mesh"
    if lowered.endswith("foam"):
        return "solve"
    return "utility"


def _normalize_env_arg(arg: str) -> tuple[str, list[str], str]:
    if "$" not in arg:
        return arg, [], ""
    if "$(" in arg or "`" in arg:
        return arg, [], "command substitution is not allowed"

    required_env: list[str] = []

    def replace_braced(match: re.Match) -> str:
        name = match.group(1)
        if name not in ALLOWED_ENV_VARS:
            raise ValueError(f"unknown environment variable {name}")
        required_env.append(name)
        return f"${{{name}}}"

    def replace_plain(match: re.Match) -> str:
        name = match.group(1)
        if name not in ALLOWED_ENV_VARS:
            raise ValueError(f"unknown environment variable {name}")
        required_env.append(name)
        return f"${{{name}}}"

    try:
        normalized = re.sub(r"\$\{([A-Za-z_][A-Za-z0-9_]*)\}", replace_braced, arg)
        normalized = re.sub(r"\$([A-Za-z_][A-Za-z0-9_]*)", replace_plain, normalized)
    except ValueError as exc:
        return arg, [], str(exc)

    return normalized, sorted(set(required_env)), ""


def _normalize_args(args: list[str]) -> tuple[list[str], list[str], str]:
    normalized_args: list[str] = []
    required_env: list[str] = []
    for arg in args:
        if any(symbol in arg for symbol in ["`", "|", ";", ">", "<"]):
            return args, [], "arg contains shell syntax"
        normalized, env_names, error = _normalize_env_arg(arg)
        if error:
            return args, [], error
        normalized_args.append(normalized)
        required_env.extend(env_names)
    return normalized_args, sorted(set(required_env)), ""


def parse_allrun_pipeline_preview(
    script: str,
    allowed_commands: set[str],
    solver_name: str = "",
) -> dict[str, Any]:
    steps: list[dict[str, Any]] = []
    warnings: list[str] = []

    if not script.strip():
        return {
            "available": False,
            "allowed_command_count": len(allowed_commands),
            "steps": steps,
            "warnings": ["Allrun script is empty."],
        }

    for line_number, raw_line in enumerate(script.splitlines(), start=1):
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if "runApplication" not in line:
            continue

        tokens = line.split()
        if not tokens or tokens[0] != "runApplication":
            warnings.append(f"line {line_number}: skipped unsupported runApplication form")
            continue

        if any(token in SHELL_TOKENS for token in tokens):
            warnings.append(f"line {line_number}: skipped shell operator")
            continue

        command_tokens = tokens[1:]
        while command_tokens:
            option = command_tokens[0]
            if option in RUN_APPLICATION_OPTIONS_WITH_VALUE:
                command_tokens = command_tokens[2:] if len(command_tokens) >= 2 else []
                continue
            if option in RUN_APPLICATION_FLAGS:
                command_tokens = command_tokens[1:]
                continue
            break

        if not command_tokens:
            warnings.append(f"line {line_number}: no command after runApplication")
            continue

        command = command_tokens[0]
        args = command_tokens[1:]
        if command in {"$application", "\"$application\"", "${application}", "\"${application}\""}:
            command = solver_name.strip()
            if not command:
                warnings.append(f"line {line_number}: skipped $application because solver_name is empty")
                continue
        elif "$" in command or command.startswith("(") or command.startswith("\"$"):
            warnings.append(f"line {line_number}: skipped variable command {command}")
            continue
        if command in DENIED_COMMANDS:
            warnings.append(f"line {line_number}: denied command {command}")
            continue
        if command not in allowed_commands:
            warnings.append(f"line {line_number}: command not in OpenFOAM allowlist: {command}")
            continue
        args, required_env, arg_error = _normalize_args(args)
        if arg_error:
            warnings.append(f"line {line_number}: skipped args: {arg_error}")
            continue

        steps.append({
            "command": command,
            "args": args,
            "stage": _stage_for_command(command),
            "source": "allrun",
            "requires_env": required_env,
        })

    return {
        "available": bool(steps),
        "allowed_command_count": len(allowed_commands),
        "steps": steps,
        "warnings": warnings,
    }


def _expand_env_arg(arg: str, env: dict[str, str]) -> tuple[str, str]:
    def replace(match: re.Match) -> str:
        name = match.group(1)
        value = env.get(name, "").strip()
        if not value:
            raise ValueError(f"missing environment variable {name}")
        return value

    try:
        expanded = re.sub(r"\$\{([A-Za-z_][A-Za-z0-9_]*)\}", replace, arg)
    except ValueError as exc:
        return arg, str(exc)
    return expanded, ""


def _path_arg_indices(args: list[str]) -> set[int]:
    indices: set[int] = set()
    for index, arg in enumerate(args[:-1]):
        if arg in PATH_ARGUMENT_FLAGS:
            indices.add(index + 1)
    return indices


def prepare_executable_pipeline(
    preview: dict[str, Any],
    *,
    env: dict[str, str] | None = None,
) -> dict[str, Any]:
    env = env or dict(os.environ)
    executable_steps: list[dict[str, Any]] = []
    warnings: list[str] = list(preview.get("warnings", [])) if isinstance(preview.get("warnings", []), list) else []

    for step_index, step in enumerate(preview.get("steps", []), start=1):
        if not isinstance(step, dict):
            warnings.append(f"step {step_index}: skipped invalid step")
            continue

        command = str(step.get("command", "")).strip()
        args = step.get("args", [])
        if not command or not isinstance(args, list):
            warnings.append(f"step {step_index}: skipped invalid command or args")
            continue

        expanded_args: list[str] = []
        path_indices = _path_arg_indices([str(item) for item in args])
        step_failed = False
        for arg_index, raw_arg in enumerate(args):
            expanded, error = _expand_env_arg(str(raw_arg), env)
            if error:
                warnings.append(f"step {step_index}: {error}")
                step_failed = True
                break
            if arg_index in path_indices:
                path = Path(expanded)
                if not path.exists():
                    warnings.append(f"step {step_index}: path does not exist: {expanded}")
                    step_failed = True
                    break
                expanded = str(path)
            expanded_args.append(expanded)

        if step_failed:
            continue

        executable_steps.append({
            "command": command,
            "args": expanded_args,
            "stage": step.get("stage", ""),
            "source": "allrun_safe",
        })

    return {
        "executable": bool(executable_steps),
        "steps": executable_steps,
        "warnings": warnings,
    }
