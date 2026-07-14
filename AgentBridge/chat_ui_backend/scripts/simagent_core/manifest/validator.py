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
    readiness = appflow_import_readiness(manifest, status)
    return {
        "status": status,
        "missing_fields": missing_fields,
        "checked_paths": checked_paths,
        "missing_paths": missing_paths,
        **readiness,
    }


def appflow_import_readiness(manifest: dict[str, Any], structure_status: str) -> dict[str, Any]:
    blockers: list[dict[str, str]] = []
    blocker_codes: set[str] = set()

    def add_blocker(code: str, message: str) -> None:
        if code in blocker_codes:
            return
        blocker_codes.add(code)
        blockers.append({"code": code, "message": message})

    workflow = manifest.get("workflow", {}) if isinstance(manifest.get("workflow", {}), dict) else {}
    run = workflow.get("run", {}) if isinstance(workflow.get("run", {}), dict) else {}
    solver = manifest.get("solver", {}) if isinstance(manifest.get("solver", {}), dict) else {}
    appflow_hints = (
        manifest.get("appflow_hints", {})
        if isinstance(manifest.get("appflow_hints", {}), dict)
        else {}
    )

    if structure_status != "valid":
        add_blocker("manifest.invalid_structure", "Manifest structure or required paths are invalid.")
    if manifest.get("version") != 1:
        add_blocker("manifest.unsupported_version", "Manifest version must be 1.")
    if str(workflow.get("status", "")).strip() != "succeeded":
        add_blocker("manifest.workflow_not_succeeded", "Workflow has not succeeded.")
    if str(run.get("status", "")).strip() != "run_completed":
        add_blocker("manifest.run_not_completed", "OpenFOAM run has not completed.")
    if str(solver.get("family", "")).strip().lower() != "openfoam":
        add_blocker("manifest.unsupported_solver", "Only OpenFOAM manifests can be imported.")
    if not str(solver.get("command", "")).strip():
        add_blocker("manifest.missing_solver_command", "Solver command is required for import.")
    if bool(appflow_hints.get("offer_repair")) or bool(
        appflow_hints.get("has_suggested_repair_actions")
    ):
        add_blocker("manifest.repair_required", "A suggested repair must be handled before import.")

    return {
        "import_ready": not blockers,
        "import_blockers": blockers,
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
