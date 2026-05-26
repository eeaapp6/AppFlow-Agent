from typing import Any

from .models import GateResult


ALLOWED_OPENFOAM_COMMANDS = {
    "blockMesh",
    "surfaceFeatureExtract",
    "snappyHexMesh",
    "checkMesh",
    "gmshToFoam",
    "icoFoam",
    "simpleFoam",
    "pimpleFoam",
    "foamToVTK",
}
SHELL_SYNTAX = {"|", "||", "&&", ";", ">", ">>", "<", "`", "$("}


def review_run_pipeline(pipeline: list[dict[str, Any]]) -> GateResult:
    result = GateResult("execution")
    if not pipeline:
        result.add_error("pipeline.empty", "Run pipeline must contain at least one step.")
        return result

    for index, step in enumerate(pipeline, start=1):
        command = str(step.get("command", "")).strip()
        args = step.get("args", [])
        if not command:
            result.add_error("pipeline.missing_command", f"Pipeline step {index} has no command.")
            continue
        if command not in ALLOWED_OPENFOAM_COMMANDS:
            result.add_error("pipeline.command_not_allowed", f"Command is not allowed: {command}.")
        if not isinstance(args, list):
            result.add_error("pipeline.invalid_args", f"Pipeline args must be a list for command: {command}.")
            continue
        for arg in args:
            _review_arg(str(arg), command, result)
    return result


def _review_arg(arg: str, command: str, result: GateResult) -> None:
    if any(token in arg for token in SHELL_SYNTAX):
        result.add_error("pipeline.shell_syntax", f"Argument for {command} contains shell syntax: {arg}.")
