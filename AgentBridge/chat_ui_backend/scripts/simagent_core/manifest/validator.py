from pathlib import Path
from typing import Any

from ..models import TaskContext


REQUIRED_TOP_LEVEL_FIELDS = [
    "version",
    "workflow",
    "solver",
    "artifacts",
    "case_summary",
    "appflow_hints",
]


def validate_appflow_manifest(manifest: dict[str, Any], task: TaskContext | None = None) -> dict[str, Any]:
    missing_fields = [field for field in REQUIRED_TOP_LEVEL_FIELDS if field not in manifest]

    workflow = manifest.get("workflow", {}) if isinstance(manifest.get("workflow", {}), dict) else {}
    solver = manifest.get("solver", {}) if isinstance(manifest.get("solver", {}), dict) else {}
    artifacts = manifest.get("artifacts", {}) if isinstance(manifest.get("artifacts", {}), dict) else {}
    appflow_hints = (
        manifest.get("appflow_hints", {})
        if isinstance(manifest.get("appflow_hints", {}), dict)
        else {}
    )

    for field in ["id", "status"]:
        if not workflow.get(field):
            missing_fields.append(f"workflow.{field}")

    if not solver.get("family"):
        missing_fields.append("solver.family")

    if not artifacts.get("case_dir"):
        missing_fields.append("artifacts.case_dir")

    checked_paths = []
    missing_paths = []
    for field in ["case_dir", "mesh_dir", "result_dir"]:
        value = str(artifacts.get(field, "")).strip()
        if not value:
            continue
        checked_paths.append({"field": f"artifacts.{field}", "path": value})
        if field == "result_dir" and (
            appflow_hints.get("export_vtk") or appflow_hints.get("run_foam_to_vtk")
        ):
            continue
        if not _path_exists(value, task):
            missing_paths.append({"field": f"artifacts.{field}", "path": value})

    for index, item in enumerate(artifacts.get("logs", [])):
        if not isinstance(item, dict):
            continue
        value = str(item.get("path", "")).strip()
        if not value:
            continue
        field = f"artifacts.logs[{index}].path"
        checked_paths.append({"field": field, "path": value})
        if not _path_exists(value, task):
            missing_paths.append({"field": field, "path": value})

    status = "valid" if not missing_fields and not missing_paths else "invalid"
    return {
        "status": status,
        "missing_fields": missing_fields,
        "checked_paths": checked_paths,
        "missing_paths": missing_paths,
    }


def _path_exists(path: str, task: TaskContext | None) -> bool:
    if Path(path).exists():
        return True
    if not task or not task.host_output_root:
        return False

    normalized_path = path.replace("\\", "/")
    normalized_host_root = task.host_output_root.replace("\\", "/").rstrip("/")
    if not normalized_path.startswith(normalized_host_root):
        return False

    relative_path = normalized_path[len(normalized_host_root):].lstrip("/")
    candidate = Path(task.output_root) / relative_path
    return candidate.exists()
