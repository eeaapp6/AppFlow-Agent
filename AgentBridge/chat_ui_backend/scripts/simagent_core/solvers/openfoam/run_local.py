import os
import subprocess
from pathlib import Path

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
from .validate import required_openfoam_files


def check_openfoam_run_readiness(task: TaskContext, plan: SimulationPlan | None = None) -> dict:
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
        }

    wm_project_dir = os.environ.get("WM_PROJECT_DIR", "").strip()
    if not wm_project_dir:
        return {
            "status": "run_blocked",
            "reason": "OpenFOAM runtime is not configured. WM_PROJECT_DIR is not set.",
            "missing_files": [],
            "wm_project_dir": "",
        }

    return {
        "status": "run_ready",
        "reason": "OpenFOAM runtime detected.",
        "missing_files": [],
        "wm_project_dir": wm_project_dir,
    }


def _run_command(command: list[str], case_dir: Path, log_path: Path, timeout_seconds: int) -> dict:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("w", encoding="utf-8", errors="replace") as log:
        process = subprocess.run(
            command,
            cwd=str(case_dir),
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
    wm_project_dir: str,
    logs: list[dict] | None = None,
) -> dict:
    return {
        "status": "run_failed",
        "reason": reason,
        "steps": steps,
        "pipeline": pipeline,
        "pipeline_info": pipeline_info,
        "logs": logs if logs is not None else _logs_from_steps(steps),
        "wm_project_dir": wm_project_dir,
    }


def run_openfoam_case(task: TaskContext, plan: SimulationPlan | None = None, timeout_seconds: int = 300) -> dict:
    readiness = check_openfoam_run_readiness(task, plan)
    if readiness.get("status") != "run_ready":
        return readiness

    case_dir = Path(task.case_dir)
    logs_dir = Path(task.logs_dir)
    steps = []
    pipeline, pipeline_info = _select_run_pipeline(plan)
    execution_gate = review_run_pipeline(pipeline)
    pipeline_info["execution_gate"] = execution_gate.to_dict()
    if execution_gate.status == "failed":
        return _run_failure(
            reason="Run pipeline failed execution gate.",
            steps=[],
            pipeline=pipeline,
            pipeline_info=pipeline_info,
            logs=[],
            wm_project_dir=readiness.get("wm_project_dir", ""),
        )

    for pipeline_step in pipeline:
        command = pipeline_step_to_command(pipeline_step)
        log_path = logs_dir / f"{command[0]}.log"
        try:
            step = _run_command(command, case_dir, log_path, timeout_seconds)
        except subprocess.TimeoutExpired:
            return _run_failure(
                reason=f"{command[0]} timed out after {timeout_seconds} seconds.",
                steps=steps,
                pipeline=pipeline,
                pipeline_info=pipeline_info,
                logs=[{"path": f"logs/{command[0]}.log", "role": command[0], "format": "text"}],
                wm_project_dir=readiness.get("wm_project_dir", ""),
            )
        except FileNotFoundError:
            return _run_failure(
                reason=f"Command not found: {command[0]}. Ensure OpenFOAM is sourced in the backend environment.",
                steps=steps,
                pipeline=pipeline,
                pipeline_info=pipeline_info,
                logs=[],
                wm_project_dir=readiness.get("wm_project_dir", ""),
            )

        step["stage"] = pipeline_step.get("stage", "")
        step["source"] = pipeline_step.get("source", "")
        steps.append(step)
        if step["return_code"] != 0:
            return _run_failure(
                reason=f"{command[0]} failed with return code {step['return_code']}.",
                steps=steps,
                pipeline=pipeline,
                pipeline_info=pipeline_info,
                wm_project_dir=readiness.get("wm_project_dir", ""),
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
        "outputs": output_index,
    }


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
