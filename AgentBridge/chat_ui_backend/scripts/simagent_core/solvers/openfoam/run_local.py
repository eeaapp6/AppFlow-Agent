import os
from pathlib import Path
import subprocess
from typing import Any

from ...models import SimulationPlan, TaskContext
from ...gates.execution_gate import review_run_pipeline
from .knowledge import AllrunScriptDatabase, CommandHelpDatabase
from .mesh import plan_uses_external_gmsh_mesh
from .run_pipeline import (
    build_basic_run_pipeline,
    parse_allrun_pipeline_preview,
    pipeline_step_to_command,
    prepare_executable_pipeline,
    run_pipeline_mode,
)
from .run_diagnostics import diagnostics_for_steps, runtime_blocked_diagnostics
from .runtime import DockerOpenFOAMRunner, OpenFOAMRunner, detect_openfoam_runtime
from .validate import required_openfoam_files


def check_openfoam_run_readiness(
    task: TaskContext,
    plan: SimulationPlan | None = None,
    *,
    runtime_info: dict[str, Any] | None = None,
) -> dict:
    if runtime_info is None:
        runtime_info = detect_openfoam_runtime(required_commands=_required_pipeline_commands(build_basic_run_pipeline(plan)))

    task_dir = Path(task.task_dir)
    missing_files = [
        rel_path
        for rel_path in required_openfoam_files(plan)
        if not (task_dir / rel_path).exists()
    ]
    if missing_files:
        return {
            "status": "run_blocked",
            "reason": "OpenFOAM case files are incomplete.",
            "missing_files": missing_files,
            "wm_project_dir": os.environ.get("WM_PROJECT_DIR", ""),
            "runtime_info": runtime_info or {},
        }

    if runtime_info and not runtime_info.get("available", False):
        return {
            "status": "run_blocked",
            "reason": runtime_info.get("reason", "OpenFOAM runtime is unavailable."),
            "missing_files": [],
            "wm_project_dir": runtime_info.get("wm_project_dir", ""),
            "runtime_info": runtime_info,
        }

    return {
        "status": "run_ready",
        "reason": "OpenFOAM runtime detected.",
        "missing_files": [],
        "wm_project_dir": (runtime_info or {}).get("wm_project_dir", os.environ.get("WM_PROJECT_DIR", "").strip()),
        "runtime_info": runtime_info or {},
    }


def _is_time_directory(path: Path) -> bool:
    if not path.is_dir():
        return False
    try:
        float(path.name)
    except ValueError:
        return False
    return True


def _discover_case_outputs(task: TaskContext) -> dict:
    case_dir = Path(task.case_dir)
    poly_mesh_dir = case_dir / "constant" / "polyMesh"
    time_dirs = sorted(
        [path for path in case_dir.iterdir() if _is_time_directory(path)],
        key=lambda path: float(path.name),
    )
    result_time_dirs = [path for path in time_dirs if path.name != "0"]
    latest_time_dir = result_time_dirs[-1] if result_time_dirs else None

    mesh_files = []
    if poly_mesh_dir.exists():
        mesh_files = [
            {
                "path": f"case/constant/polyMesh/{item.name}",
                "role": item.stem,
                "format": "openfoam-polyMesh",
            }
            for item in sorted(poly_mesh_dir.iterdir())
            if item.is_file()
        ]

    result_entries = []
    for time_dir in result_time_dirs:
        fields = [
            {
                "path": f"case/{time_dir.name}/{field.name}",
                "role": field.name,
                "format": "openfoam-field",
            }
            for field in sorted(time_dir.iterdir())
            if field.is_file()
        ]
        result_entries.append(
            {
                "time": time_dir.name,
                "path": f"case/{time_dir.name}",
                "fields": fields,
            }
        )

    return {
        "mesh": {
            "path": "case/constant/polyMesh",
            "files": mesh_files,
            "primary": "case/constant/polyMesh" if poly_mesh_dir.exists() else "",
        },
        "results": {
            "path": "case",
            "time_directories": result_entries,
            "latest_time": latest_time_dir.name if latest_time_dir else "",
            "latest_path": f"case/{latest_time_dir.name}" if latest_time_dir else "",
        },
        "render_hint": {
            "primary_geometry": "case/constant/polyMesh" if poly_mesh_dir.exists() else "",
            "primary_result": f"case/{latest_time_dir.name}" if latest_time_dir else "",
            "field_candidates": ["U", "p"],
            "coordinate_unit": "m",
        },
    }


def _logs_from_steps(steps: list[dict]) -> list[dict]:
    return [
        {"path": f"logs/{item['log']}", "role": item["command"].split()[0], "format": "text"}
        for item in steps
    ]


def _run_failure(
    *,
    reason: str,
    steps: list[dict],
    pipeline: list[dict],
    pipeline_info: dict,
    runtime_info: dict[str, Any],
    logs: list[dict] | None = None,
    status: str = "run_failed",
    diagnostics: dict[str, Any] | None = None,
) -> dict:
    result = {
        "status": status,
        "reason": reason,
        "steps": steps,
        "pipeline": pipeline,
        "pipeline_info": pipeline_info,
        "logs": logs if logs is not None else _logs_from_steps(steps),
        "wm_project_dir": runtime_info.get("wm_project_dir", ""),
        "runtime_info": runtime_info,
    }
    if diagnostics:
        result["diagnostics"] = diagnostics
    return result


def run_openfoam_case(
    task: TaskContext,
    plan: SimulationPlan | None = None,
    timeout_seconds: int = 300,
    *,
    runner: OpenFOAMRunner | None = None,
    runtime_info: dict[str, Any] | None = None,
) -> dict:
    pipeline, pipeline_info = _select_run_pipeline(plan)
    execution_gate = review_run_pipeline(pipeline)
    pipeline_info["execution_gate"] = execution_gate.to_dict()
    if execution_gate.status == "failed":
        runtime_info = runtime_info or detect_openfoam_runtime(required_commands=[])
        return _run_failure(
            reason="Run pipeline failed execution gate.",
            steps=[],
            pipeline=pipeline,
            pipeline_info=pipeline_info,
            logs=[],
            runtime_info=runtime_info,
        )

    runtime_info = runtime_info or detect_openfoam_runtime(required_commands=_required_pipeline_commands(pipeline))
    readiness = check_openfoam_run_readiness(task, plan, runtime_info=runtime_info)
    if readiness.get("status") != "run_ready":
        readiness.setdefault("pipeline", pipeline)
        readiness.setdefault("pipeline_info", pipeline_info)
        readiness.setdefault("steps", [])
        readiness.setdefault("logs", [])
        readiness.setdefault(
            "diagnostics",
            runtime_blocked_diagnostics(str(readiness.get("reason", "")).strip()),
        )
        return readiness

    case_dir = Path(task.case_dir)
    logs_dir = Path(task.logs_dir)
    steps = []
    backend = str(runtime_info.get("backend", "local")).strip().lower() or "local"
    if backend == "wsl":
        return _run_failure(
            reason="WSL OpenFOAM backend is detected but execution through WSL is not implemented yet.",
            steps=[],
            pipeline=pipeline,
            pipeline_info=pipeline_info,
            logs=[],
            runtime_info={**runtime_info, "available": False},
            status="run_blocked",
        )

    runner = runner or _runner_for_runtime(runtime_info)
    for pipeline_step in pipeline:
        command = pipeline_step_to_command(pipeline_step)
        log_path = logs_dir / f"{command[0]}.log"
        try:
            step = runner.run_step(command, case_dir, log_path, timeout_seconds)
        except subprocess.TimeoutExpired:
            log_file = f"logs/{command[0]}.log"
            diagnostics = diagnostics_for_steps(
                steps,
                events=[{
                    "kind": "timeout",
                    "command": command[0],
                    "log_file": log_file,
                    "timeout_seconds": timeout_seconds,
                }],
            )
            return _run_failure(
                reason=f"{command[0]} timed out after {timeout_seconds} seconds.",
                steps=steps,
                pipeline=pipeline,
                pipeline_info=pipeline_info,
                logs=[{"path": log_file, "role": command[0], "format": "text"}],
                runtime_info=runtime_info,
                diagnostics=diagnostics,
            )
        except FileNotFoundError:
            missing_command = _missing_command_for_runtime(runtime_info, command[0])
            missing_runtime = {**runtime_info, "available": False, "missing_commands": [missing_command]}
            reason = _missing_command_reason(runtime_info, missing_command)
            return _run_failure(
                reason=reason,
                steps=steps,
                pipeline=pipeline,
                pipeline_info=pipeline_info,
                logs=[],
                runtime_info=missing_runtime,
                status="run_blocked",
                diagnostics=runtime_blocked_diagnostics(reason),
            )

        step["stage"] = pipeline_step.get("stage", "")
        step["source"] = pipeline_step.get("source", "")
        steps.append(step)
        if step["return_code"] != 0:
            diagnostics = diagnostics_for_steps(steps)
            return _run_failure(
                reason=f"{command[0]} failed with return code {step['return_code']}.",
                steps=steps,
                pipeline=pipeline,
                pipeline_info=pipeline_info,
                runtime_info=runtime_info,
                diagnostics=diagnostics,
            )

    diagnostics = diagnostics_for_steps(steps)
    if diagnostics.get("severity") == "failed":
        return _run_failure(
            reason=f"OpenFOAM run completed with error diagnostics: {diagnostics.get('summary', '')}",
            steps=steps,
            pipeline=pipeline,
            pipeline_info=pipeline_info,
            runtime_info=runtime_info,
            diagnostics=diagnostics,
        )

    output_index = _discover_case_outputs(task)
    commands_text = " and ".join(step["command"] for step in pipeline)
    return {
        "status": "run_completed",
        "reason": f"{commands_text} completed successfully.",
        "steps": steps,
        "pipeline": pipeline,
        "pipeline_info": pipeline_info,
        "logs": _logs_from_steps(steps),
        "wm_project_dir": readiness.get("wm_project_dir", ""),
        "runtime_info": runtime_info,
        "outputs": output_index,
        "diagnostics": diagnostics,
    }


def _required_pipeline_commands(pipeline: list[dict]) -> list[str]:
    commands = []
    for step in pipeline:
        if not isinstance(step, dict):
            continue
        command = str(step.get("command", "")).strip()
        if command:
            commands.append(command)
    return sorted(set(commands))


def _runner_for_runtime(runtime_info: dict[str, Any]) -> OpenFOAMRunner:
    backend = str(runtime_info.get("backend", "local")).strip().lower()
    if backend == "docker":
        return DockerOpenFOAMRunner.from_runtime_info(runtime_info)
    return OpenFOAMRunner()


def _missing_command_for_runtime(runtime_info: dict[str, Any], command: str) -> str:
    return "docker" if str(runtime_info.get("backend", "")).strip().lower() == "docker" else command


def _missing_command_reason(runtime_info: dict[str, Any], missing_command: str) -> str:
    if str(runtime_info.get("backend", "")).strip().lower() == "docker":
        return "Docker command not found. Install Docker or select a different OpenFOAM backend."
    return f"Command not found: {missing_command}. Ensure OpenFOAM is sourced in the backend environment."


def _select_run_pipeline(plan: SimulationPlan | None) -> tuple[list[dict], dict]:
    basic_pipeline = build_basic_run_pipeline(plan)
    if plan and plan_uses_external_gmsh_mesh(plan.parameters):
        return basic_pipeline, {
            "mode": "basic",
            "selected": "gmsh_external",
            "fallback_used": False,
            "warnings": [],
        }

    mode = run_pipeline_mode()
    if mode != "allrun_safe":
        return basic_pipeline, {
            "mode": "basic",
            "selected": "basic",
            "fallback_used": False,
            "warnings": [],
        }

    if not plan or not plan.reference_case:
        return basic_pipeline, {
            "mode": "allrun_safe",
            "selected": "basic",
            "fallback_used": True,
            "warnings": ["No reference_case is available for Allrun pipeline."],
        }

    allrun_database = AllrunScriptDatabase.from_default_data()
    script = allrun_database.load_script(plan.reference_case)
    if not script.strip():
        return basic_pipeline, {
            "mode": "allrun_safe",
            "selected": "basic",
            "fallback_used": True,
            "warnings": ["No Allrun script was found for the selected reference case."],
        }

    allowed_commands = CommandHelpDatabase.from_default_data().load_command_names()
    preview = parse_allrun_pipeline_preview(script, allowed_commands, solver_name=plan.solver_name)
    executable = prepare_executable_pipeline(preview)
    if not executable.get("executable"):
        return basic_pipeline, {
            "mode": "allrun_safe",
            "selected": "basic",
            "fallback_used": True,
            "allrun_preview": preview,
            "allrun_executable": executable,
            "warnings": executable.get("warnings", []),
        }

    return executable.get("steps", []), {
        "mode": "allrun_safe",
        "selected": "allrun_safe",
        "fallback_used": False,
        "allrun_preview": preview,
        "allrun_executable": executable,
        "warnings": executable.get("warnings", []),
    }
