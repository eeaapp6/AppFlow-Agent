from pathlib import Path
import re
from typing import Any

from ..models import TaskContext
from ..repair.proposals import repair_action_for_gate_review
from .models import GateResult


FATAL_LOG_MARKERS = [
    "FOAM FATAL ERROR",
    "Floating point exception",
    "Segmentation fault",
    "core dumped",
]
NAN_PATTERN = re.compile(r"(?<![A-Za-z0-9_])(?:nan|inf)(?![A-Za-z0-9_])", re.IGNORECASE)
RESIDUAL_PATTERN = re.compile(
    r"Solving for\s+([^,]+),\s*Initial residual\s*=\s*([^,]+),\s*Final residual\s*=\s*([^,]+)",
    re.IGNORECASE,
)
HIGH_FINAL_RESIDUAL_THRESHOLD = 1.0
MESH_LOG_ROLES = {"checkmesh", "blockmesh", "mesh"}
MESH_ERROR_PATTERNS = [
    re.compile(r"\*\*\*error", re.IGNORECASE),
    re.compile(r"failed\s+[1-9]\d*\s+mesh\s+checks?", re.IGNORECASE),
    re.compile(r"negative\s+volume", re.IGNORECASE),
    re.compile(r"zero\s+or\s+negative\s+pyramid\s+volume", re.IGNORECASE),
]
MESH_WARNING_MARKERS = [
    "severely non-orthogonal",
    "highly skew",
    "max skewness",
    "max aspect ratio",
]
INTERNAL_EVIDENCE_KEY = "_result_review_evidence"
INTERNAL_METRICS_KEY = "_result_review_metrics"
FAILED_MESH_CHECKS_PATTERN = re.compile(r"failed\s+([0-9]+)\s+mesh\s+checks?", re.IGNORECASE)
MESH_METRIC_PATTERNS = {
    "max_non_orthogonality": [
        re.compile(r"max(?:imum)?\s+non[-\s]?orthogonality\s*[=:]\s*([-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)", re.IGNORECASE),
        re.compile(r"mesh\s+non[-\s]?orthogonality\s+max\s*:\s*([-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)", re.IGNORECASE),
    ],
    "max_skewness": [
        re.compile(r"max(?:imum)?\s+skewness\s*[=:]\s*([-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)", re.IGNORECASE),
    ],
    "max_aspect_ratio": [
        re.compile(r"max(?:imum)?\s+aspect\s+ratio\s*[=:]\s*([-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)", re.IGNORECASE),
    ],
    "min_volume": [
        re.compile(r"min(?:imum)?\s+volume\s*[=:]\s*([-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)", re.IGNORECASE),
    ],
}


def review_run_results(task: TaskContext, run_result: dict[str, Any]) -> GateResult:
    result = GateResult("result_review")
    status = str(run_result.get("status", "")).strip()
    if status != "run_completed":
        if _review_run_diagnostics(run_result, result):
            _finalize_review(task, run_result, result)
            return result
        reason = str(run_result.get("reason", "")).strip() or "Run did not complete."
        _add_error(
            result,
            "result.run_incomplete",
            reason,
            {"status": status or "unknown", "reason": reason},
        )
        _finalize_review(task, run_result, result)
        return result

    if not _review_run_diagnostics(run_result, result):
        _review_logs(task, run_result, result)
    _review_latest_result_path(task, run_result, result)
    _finalize_review(task, run_result, result)
    return result


def _review_run_diagnostics(run_result: dict[str, Any], result: GateResult) -> bool:
    diagnostics = run_result.get("diagnostics", {})
    if not isinstance(diagnostics, dict):
        return False

    items = diagnostics.get("items", [])
    if not isinstance(items, list) or not items:
        _record_metrics(result, _diagnostic_metrics(diagnostics))
        severity = str(diagnostics.get("severity", "")).strip()
        summary = str(diagnostics.get("summary", "")).strip()
        if severity == "failed":
            _add_error(
                result,
                "result.diagnostics_failed",
                summary or "Run diagnostics failed without itemized evidence.",
                {"reason": summary or "Run diagnostics failed."},
            )
            return True
        if severity == "warning":
            _add_warning(
                result,
                "result.diagnostics_warning",
                summary or "Run diagnostics reported a warning without itemized evidence.",
                {"reason": summary or "Run diagnostics warning."},
            )
            return True
        return False

    for item in items:
        if not isinstance(item, dict):
            continue
        code = str(item.get("code", "")).strip()
        message = str(item.get("message", "")).strip()
        if not code or not message:
            continue
        severity = str(item.get("severity", "")).strip()
        evidence = _diagnostic_evidence(item)
        if severity == "warning":
            _add_warning(result, code, message, evidence)
        else:
            _add_error(result, code, message, evidence)

    _record_metrics(result, _diagnostic_metrics(diagnostics))
    return bool(result.issues)


def _review_logs(task: TaskContext, run_result: dict[str, Any], result: GateResult) -> None:
    logs = run_result.get("logs", [])
    if not isinstance(logs, list):
        return

    for item in logs:
        if not isinstance(item, dict):
            continue

        log_path = _resolve_task_path(task, str(item.get("path", "")).strip())
        if not log_path or not log_path.exists() or not log_path.is_file():
            continue

        content = log_path.read_text(encoding="utf-8", errors="replace")
        display_path = str(item.get("path", log_path.name))
        command = _log_command(item, log_path)
        _review_log_content(display_path, content, result, command)
        if _is_mesh_log(item, log_path):
            _review_mesh_log_content(display_path, content, result, command)


def _review_log_content(display_path: str, content: str, result: GateResult, command: str) -> None:
    for marker in FATAL_LOG_MARKERS:
        if marker in content:
            message = f"{display_path} contains fatal marker: {marker}."
            _add_error(
                result,
                "result.log_fatal",
                message,
                _log_evidence(display_path, command, content, content.find(marker), marker=marker),
            )
            break

    nan_match = NAN_PATTERN.search(content)
    if nan_match:
        _record_metrics(result, {"has_nan_or_inf": True})
        message = f"{display_path} contains nan or inf in solver output."
        _add_error(
            result,
            "result.residual_nan",
            message,
            _log_evidence(display_path, command, content, nan_match.start()),
        )

    residuals = _residual_entries(content)
    _record_metrics(result, _residual_metrics(residuals))
    high_residual = next((entry for entry in residuals if entry["final"] > HIGH_FINAL_RESIDUAL_THRESHOLD), None)
    if high_residual:
        message = (
            f"{display_path} final residual for {high_residual['field']} is "
            f"{high_residual['final']:.3g}, above {HIGH_FINAL_RESIDUAL_THRESHOLD:.3g}."
        )
        _add_error(
            result,
            "result.residual_high",
            message,
            _residual_evidence(display_path, command, high_residual),
        )

    if residuals and "ExecutionTime" not in content:
        message = f"{display_path} contains solver residuals but no ExecutionTime marker."
        _add_warning(
            result,
            "result.no_execution_time",
            message,
            _residual_evidence(display_path, command, residuals[0]),
        )


def _review_mesh_log_content(display_path: str, content: str, result: GateResult, command: str) -> None:
    _record_metrics(result, _mesh_metrics(content))

    for pattern in MESH_ERROR_PATTERNS:
        match = pattern.search(content)
        if match:
            message = f"{display_path} contains mesh quality failure marker."
            _add_error(
                result,
                "result.mesh_quality_failed",
                message,
                _log_evidence(display_path, command, content, match.start()),
            )
            return

    lower_content = content.lower()
    for marker in MESH_WARNING_MARKERS:
        index = lower_content.find(marker.lower())
        if index >= 0:
            message = f"{display_path} contains mesh quality warning marker: {marker}."
            _add_warning(
                result,
                "result.mesh_quality_warning",
                message,
                _log_evidence(display_path, command, content, index, marker=marker),
            )
            return


def _is_mesh_log(item: dict[str, Any], log_path: Path) -> bool:
    role = str(item.get("role", "")).strip().lower()
    name = str(item.get("name", "")).strip().lower()
    path_name = log_path.name.lower()
    values = {role, name, path_name}
    return any(
        marker in value
        for value in values
        for marker in MESH_LOG_ROLES
        if value
    )


def _log_command(item: dict[str, Any], log_path: Path) -> str:
    for key in ("command", "role", "name"):
        value = str(item.get(key, "")).strip()
        if value:
            return value
    return log_path.stem


def _residual_entries(content: str) -> list[dict[str, Any]]:
    entries: list[dict[str, Any]] = []
    for match in RESIDUAL_PATTERN.finditer(content):
        final = _safe_float(match.group(3))
        initial = _safe_float(match.group(2))
        if initial is None or final is None:
            continue
        line_number, excerpt = _line_excerpt(content, match.start())
        entries.append(
            {
                "field": match.group(1).strip(),
                "initial": initial,
                "final": final,
                "line": line_number,
                "excerpt": excerpt,
            }
        )
    return entries


def _safe_float(value: str) -> float | None:
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def _review_latest_result_path(task: TaskContext, run_result: dict[str, Any], result: GateResult) -> None:
    outputs = run_result.get("outputs", {})
    outputs = outputs if isinstance(outputs, dict) else {}
    results = outputs.get("results", {})
    results = results if isinstance(results, dict) else {}
    latest_path = str(results.get("latest_path", "")).strip()
    if not latest_path:
        message = "Run completed but no latest result time directory was reported."
        _add_error(
            result,
            "result.missing_latest_path",
            message,
            {"output": "outputs.results.latest_path", "reason": message},
        )
        return

    resolved_path = _resolve_task_path(task, latest_path)
    if not resolved_path or not resolved_path.exists() or not resolved_path.is_dir():
        message = f"Latest result directory does not exist: {latest_path}."
        _add_error(result, "result.missing_latest_dir", message, {"path": latest_path})


def _finalize_review(task: TaskContext, run_result: dict[str, Any], result: GateResult) -> None:
    _attach_diagnostics(result)
    result.metadata["result_review"] = _result_review_summary(task, run_result, result)


def _result_review_summary(task: TaskContext, run_result: dict[str, Any], result: GateResult) -> dict[str, Any]:
    output_state = _output_state(task, run_result)
    action = repair_action_for_gate_review(result.to_dict())
    can_show_results = result.status in {"passed", "warning"} and output_state["has_displayable_results"]
    should_offer_repair = result.status == "failed" and bool(action)
    should_offer_rerun = result.status in {"failed", "warning"} or str(run_result.get("status", "")).strip() != "run_completed"
    review = {
        "status": result.status,
        "summary": result.metadata.get("diagnostics", {}).get("summary", ""),
        "items": [item.to_dict() for item in result.issues],
        "can_show_results": can_show_results,
        "should_offer_repair": should_offer_repair,
        "should_offer_rerun": should_offer_rerun,
        "should_offer_vtk": can_show_results and output_state["has_raw_results"] and not output_state["has_vtk_results"],
        "should_offer_paraview": can_show_results,
        "has_raw_results": output_state["has_raw_results"],
        "has_vtk_results": output_state["has_vtk_results"],
        "latest_result_path": output_state["latest_result_path"],
    }
    if action:
        review["repair_action_id"] = str(action.get("id", "")).strip()
        review["repair_mode"] = "executable" if isinstance(action.get("patches"), list) and action.get("patches") else "manual"
    else:
        review["repair_mode"] = "none"
    return review


def _output_state(task: TaskContext, run_result: dict[str, Any]) -> dict[str, Any]:
    outputs = run_result.get("outputs", {})
    outputs = outputs if isinstance(outputs, dict) else {}
    results = outputs.get("results", {})
    results = results if isinstance(results, dict) else {}
    latest_path = str(results.get("latest_path", "")).strip()
    latest_resolved = _resolve_task_path(task, latest_path) if latest_path else None
    has_raw_results = bool(latest_resolved and latest_resolved.exists() and latest_resolved.is_dir() and _is_result_time_path(latest_path))
    has_vtk_results = _has_vtk_result(task)
    return {
        "has_displayable_results": has_raw_results or has_vtk_results,
        "has_raw_results": has_raw_results,
        "has_vtk_results": has_vtk_results,
        "latest_result_path": latest_path if has_raw_results else "",
    }


def _is_result_time_path(path_text: str) -> bool:
    name = Path(path_text.replace("\\", "/").rstrip("/")).name
    try:
        return float(name) != 0
    except ValueError:
        return False


def _has_vtk_result(task: TaskContext) -> bool:
    vtk_dir = Path(task.case_dir) / "VTK"
    return vtk_dir.exists() and any(path.suffix.lower() == ".vtk" for path in vtk_dir.iterdir() if path.is_file())


def _resolve_task_path(task: TaskContext, relative_path: str) -> Path | None:
    cleaned = relative_path.replace("\\", "/").strip().strip("/")
    if not cleaned:
        return None

    path = Path(cleaned)
    if path.is_absolute():
        return path
    return Path(task.task_dir) / cleaned


def _attach_diagnostics(result: GateResult) -> None:
    evidence_items = result.metadata.pop(INTERNAL_EVIDENCE_KEY, [])
    metrics = result.metadata.pop(INTERNAL_METRICS_KEY, {})
    result.metadata["diagnostics"] = _diagnostics_from_result(result, evidence_items, metrics)


def _diagnostics_from_result(
    result: GateResult,
    evidence_items: list[dict[str, Any]],
    metrics: dict[str, Any],
) -> dict[str, Any]:
    primary_issue = _primary_issue(result)
    categories = sorted({_category_for_issue(issue.code) for issue in result.issues if _category_for_issue(issue.code)})
    return {
        "severity": result.status,
        "categories": categories,
        "summary": _diagnostic_summary(primary_issue, categories, result.status),
        "primary_issue": primary_issue.to_dict() if primary_issue else {},
        "evidence": _evidence_for_issue(primary_issue, evidence_items),
        "metrics": metrics if isinstance(metrics, dict) else {},
    }


def _primary_issue(result: GateResult):
    errors = [issue for issue in result.issues if issue.severity == "error"]
    if errors:
        return errors[0]
    return result.issues[0] if result.issues else None


def _category_for_issue(code: str) -> str:
    if code.startswith("result.mesh_"):
        return "mesh"
    if code.startswith("physics."):
        return "physics"
    if code.startswith("result.residual_") or code in {"result.courant_high", "result.divergence", "result.continuity_abnormal"}:
        return "numerics"
    if code in {
        "result.pressure_reference_missing",
        "result.turbulence_properties_missing",
        "result.fvsolution_solver_block_missing",
        "result.fvsolution_solver_keyword_missing",
    }:
        return "configuration"
    if code in {
        "result.log_fatal",
        "result.log_warning",
        "result.run_incomplete",
        "result.no_execution_time",
        "result.command_nonzero",
        "result.command_timeout",
        "result.runtime_unavailable",
        "result.diagnostics_failed",
        "result.diagnostics_warning",
    }:
        return "runtime"
    if code in {"result.missing_boundary_field", "result.unknown_patch", "result.patch_type_inconsistent"}:
        return "boundary"
    if code.startswith("result.missing_latest_"):
        return "output"
    return "result"


def _diagnostic_summary(primary_issue, categories: list[str], status: str) -> str:
    if primary_issue:
        category_text = ", ".join(categories) if categories else "result"
        return f"{category_text}: {primary_issue.message}"
    if status == "passed":
        return "Result review passed."
    return "Result review completed without a primary issue."


def _add_error(result: GateResult, code: str, message: str, evidence: dict[str, Any]) -> None:
    result.add_error(code, message)
    _record_evidence(result, code, message, evidence)


def _add_warning(result: GateResult, code: str, message: str, evidence: dict[str, Any]) -> None:
    result.add_warning(code, message)
    _record_evidence(result, code, message, evidence)


def _record_evidence(result: GateResult, code: str, message: str, evidence: dict[str, Any]) -> None:
    evidence_items = result.metadata.setdefault(INTERNAL_EVIDENCE_KEY, [])
    if not isinstance(evidence_items, list):
        evidence_items = []
        result.metadata[INTERNAL_EVIDENCE_KEY] = evidence_items

    cleaned = _clean_evidence(evidence)
    if not cleaned:
        return

    cleaned["issue_code"] = code
    cleaned["issue_message"] = message
    evidence_items.append(cleaned)


def _record_metrics(result: GateResult, metrics: dict[str, Any]) -> None:
    cleaned = _clean_metrics(metrics)
    if not cleaned:
        return

    existing = result.metadata.setdefault(INTERNAL_METRICS_KEY, {})
    if not isinstance(existing, dict):
        existing = {}
        result.metadata[INTERNAL_METRICS_KEY] = existing

    _merge_metrics(existing, cleaned)


def _clean_metrics(metrics: dict[str, Any]) -> dict[str, Any]:
    cleaned: dict[str, Any] = {}
    for key, value in metrics.items():
        if isinstance(value, bool):
            cleaned[key] = value
        elif isinstance(value, int):
            cleaned[key] = value
        elif isinstance(value, float):
            cleaned[key] = value
        elif isinstance(value, str):
            text = value.strip()
            if text:
                cleaned[key] = text
        elif isinstance(value, list):
            items = [str(item).strip() for item in value if str(item).strip()]
            if items:
                cleaned[key] = items
        elif isinstance(value, dict):
            dict_value = _clean_metric_dict(value)
            if dict_value:
                cleaned[key] = dict_value
    return cleaned


def _clean_metric_dict(value: dict[str, Any]) -> dict[str, Any]:
    cleaned: dict[str, Any] = {}
    for key, item in value.items():
        key_text = str(key).strip()
        if not key_text:
            continue
        if isinstance(item, list):
            entries = [str(entry).strip() for entry in item if str(entry).strip()]
            if entries:
                cleaned[key_text] = entries
        elif isinstance(item, (str, int, float, bool)):
            item_text = str(item).strip()
            if item_text:
                cleaned[key_text] = item
    return cleaned


def _diagnostic_metrics(diagnostics: dict[str, Any]) -> dict[str, Any]:
    metrics = diagnostics.get("metrics", {})
    return metrics if isinstance(metrics, dict) else {}


def _diagnostic_evidence(item: dict[str, Any]) -> dict[str, Any]:
    evidence = {
        "log_path": str(item.get("log_file", "")).strip(),
        "command": str(item.get("source", "")).strip(),
        "excerpt": str(item.get("matched_line", "")).strip(),
        "line": item.get("line", 0),
        "marker": str(item.get("code", "")).strip(),
        "field": str(item.get("field", "")).strip(),
        "patch": str(item.get("patch", "")).strip(),
        "max_courant": item.get("max_courant", ""),
        "return_code": item.get("return_code", ""),
    }
    return {key: value for key, value in evidence.items() if str(value).strip()}


def _merge_metrics(target: dict[str, Any], incoming: dict[str, Any]) -> None:
    incoming_worst_field = incoming.get("worst_residual_field", "")
    incoming_max_final = incoming.get("max_final_residual")
    existing_max_final = target.get("max_final_residual")
    should_update_worst_field = incoming_max_final is not None and (
        existing_max_final is None or float(incoming_max_final) >= float(existing_max_final)
    )

    for key, value in incoming.items():
        if key == "worst_residual_field":
            continue
        if key in {
            "max_initial_residual",
            "max_final_residual",
            "max_courant",
            "max_non_orthogonality",
            "max_skewness",
            "max_aspect_ratio",
            "failed_mesh_checks",
        }:
            target[key] = max(float(target.get(key, value)), float(value))
            continue
        if key == "min_volume" and key in target:
            target[key] = min(float(target[key]), float(value))
            continue
        if key in {"residual_count"}:
            target[key] = int(target.get(key, 0)) + int(value)
            continue
        if key in {"residual_fields"}:
            existing = target.get(key, [])
            existing = existing if isinstance(existing, list) else []
            target[key] = sorted({*existing, *value})
            continue
        if key == "missing_boundary_fields" and isinstance(value, dict):
            existing = target.get(key, {})
            existing = existing if isinstance(existing, dict) else {}
            for field_name, patch_names in value.items():
                current = existing.get(field_name, [])
                current = current if isinstance(current, list) else []
                incoming_items = patch_names if isinstance(patch_names, list) else [patch_names]
                existing[field_name] = sorted({
                    *[str(item) for item in current if str(item).strip()],
                    *[str(item) for item in incoming_items if str(item).strip()],
                })
            target[key] = existing
            continue
        if key == "has_nan_or_inf":
            target[key] = bool(target.get(key, False)) or bool(value)
            continue
        target[key] = value

    if should_update_worst_field and str(incoming_worst_field).strip():
        target["worst_residual_field"] = str(incoming_worst_field).strip()


def _clean_evidence(evidence: dict[str, Any]) -> dict[str, Any]:
    cleaned: dict[str, Any] = {}
    for key, value in evidence.items():
        if isinstance(value, str):
            text = value.strip()
            if text:
                cleaned[key] = text
        elif isinstance(value, (int, float)):
            cleaned[key] = value
    return cleaned


def _residual_metrics(residuals: list[dict[str, Any]]) -> dict[str, Any]:
    if not residuals:
        return {}

    worst = max(residuals, key=lambda item: float(item["final"]))
    return {
        "residual_count": len(residuals),
        "residual_fields": sorted({str(item["field"]) for item in residuals if str(item.get("field", "")).strip()}),
        "max_initial_residual": max(float(item["initial"]) for item in residuals),
        "max_final_residual": float(worst["final"]),
        "worst_residual_field": str(worst["field"]),
    }


def _mesh_metrics(content: str) -> dict[str, Any]:
    metrics: dict[str, Any] = {}
    failed_match = FAILED_MESH_CHECKS_PATTERN.search(content)
    if failed_match:
        metrics["failed_mesh_checks"] = int(failed_match.group(1))

    for name, patterns in MESH_METRIC_PATTERNS.items():
        value = _first_metric_value(content, patterns)
        if value is not None:
            metrics[name] = value
    return metrics


def _first_metric_value(content: str, patterns: list[re.Pattern]) -> float | None:
    for pattern in patterns:
        match = pattern.search(content)
        if not match:
            continue
        value = _safe_float(match.group(1))
        if value is not None:
            return value
    return None


def _evidence_for_issue(primary_issue, evidence_items: list[dict[str, Any]]) -> dict[str, Any]:
    if not primary_issue or not isinstance(evidence_items, list):
        return {}

    for item in evidence_items:
        if not isinstance(item, dict):
            continue
        if item.get("issue_code") == primary_issue.code and item.get("issue_message") == primary_issue.message:
            return {key: value for key, value in item.items() if not key.startswith("issue_")}
    return {}


def _log_evidence(
    display_path: str,
    command: str,
    content: str,
    index: int,
    *,
    marker: str = "",
) -> dict[str, Any]:
    line_number, excerpt = _line_excerpt(content, index)
    return {
        "log_path": display_path,
        "command": command,
        "line": line_number,
        "excerpt": excerpt,
        "marker": marker,
    }


def _residual_evidence(display_path: str, command: str, residual: dict[str, Any]) -> dict[str, Any]:
    return {
        "log_path": display_path,
        "command": command,
        "line": residual.get("line", 0),
        "excerpt": str(residual.get("excerpt", "")).strip(),
        "field": str(residual.get("field", "")).strip(),
        "final_residual": residual.get("final", 0),
    }


def _line_excerpt(content: str, index: int) -> tuple[int, str]:
    index = max(0, index)
    start = content.rfind("\n", 0, index) + 1
    end = content.find("\n", index)
    if end < 0:
        end = len(content)

    line_number = content.count("\n", 0, start) + 1
    excerpt = content[start:end].strip()
    return line_number, _truncate_excerpt(excerpt)


def _truncate_excerpt(text: str, limit: int = 220) -> str:
    if len(text) <= limit:
        return text
    return text[: limit - 3].rstrip() + "..."
