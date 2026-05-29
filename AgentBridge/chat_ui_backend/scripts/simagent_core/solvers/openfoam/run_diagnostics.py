from pathlib import Path
import re
from typing import Any


HIGH_FINAL_RESIDUAL_THRESHOLD = 1.0
HIGH_COURANT_THRESHOLD = 1.0
RESIDUAL_PATTERN = re.compile(
    r"Solving for\s+([^,]+),\s*Initial residual\s*=\s*([^,]+),\s*Final residual\s*=\s*([^,]+)",
    re.IGNORECASE,
)
COURANT_PATTERN = re.compile(
    r"Courant\s+Number[^\n]*?max\s*[:=]\s*([-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)",
    re.IGNORECASE,
)
MISSING_PATCH_FIELD_PATTERN = re.compile(
    r"Cannot\s+find\s+patchField\s+entry\s+for\s+(?:patch\s+)?([A-Za-z_][A-Za-z0-9_]*)",
    re.IGNORECASE,
)
UNKNOWN_PATCH_PATTERNS = [
    re.compile(r"Unknown\s+patch(?:\s+name)?\s+([A-Za-z_][A-Za-z0-9_]*)", re.IGNORECASE),
    re.compile(r"patch\s+([A-Za-z_][A-Za-z0-9_]*)\s+(?:not\s+found|does\s+not\s+exist)", re.IGNORECASE),
]
FIELD_PATH_PATTERN = re.compile(r"(?:^|[/\\])0[/\\]([A-Za-z_][A-Za-z0-9_]*)(?:[/\\]boundaryField)?", re.IGNORECASE)
NAN_PATTERN = re.compile(r"(?<![A-Za-z0-9_])(?:nan|inf)(?![A-Za-z0-9_])", re.IGNORECASE)
FATAL_MARKERS = [
    ("result.log_fatal", "FOAM FATAL ERROR", "OpenFOAM reported a fatal error."),
    ("result.residual_nan", "Floating point exception", "OpenFOAM reported a floating point exception."),
    ("result.log_fatal", "Segmentation fault", "OpenFOAM reported a segmentation fault."),
    ("result.log_fatal", "core dumped", "OpenFOAM reported a core dump."),
]
DIVERGENCE_PATTERN = re.compile(r"\b(?:divergence detected|solution diverged|diverging)\b", re.IGNORECASE)
WARNING_PATTERN = re.compile(r"(?:--> FOAM Warning|Warning:)", re.IGNORECASE)


def diagnostics_for_steps(
    steps: list[dict[str, Any]],
    *,
    events: list[dict[str, Any]] | None = None,
) -> dict[str, Any]:
    items: list[dict[str, Any]] = []
    metrics: dict[str, Any] = {}

    for event in events or []:
        item = _event_diagnostic(event)
        if item:
            items.append(item)

    for step in steps:
        if not isinstance(step, dict):
            continue
        log_path = _step_log_path(step)
        if not log_path or not log_path.exists() or not log_path.is_file():
            continue
        content = log_path.read_text(encoding="utf-8", errors="replace")
        parsed = parse_openfoam_log(
            content,
            command=str(step.get("command", "")).strip(),
            log_file=_display_log_file(step, log_path),
        )
        items.extend(parsed["items"])
        _merge_metrics(metrics, parsed["metrics"])

        return_code = _safe_int(step.get("return_code"))
        if return_code is not None and return_code != 0:
            item = _diagnostic(
                code="result.command_nonzero",
                severity="error",
                category="runtime",
                source="runtime",
                message=f"{_command_name(step)} exited with return code {return_code}.",
                log_file=_display_log_file(step, log_path),
                matched_line="",
                repair_hint="Inspect the command log before rerunning.",
            )
            item["return_code"] = return_code
            items.append(item)

    _merge_metrics(metrics, _metrics_from_items(items))
    return _diagnostic_bundle(items, metrics)


def parse_openfoam_log(content: str, *, command: str, log_file: str = "") -> dict[str, Any]:
    items: list[dict[str, Any]] = []
    metrics: dict[str, Any] = {}
    source = _source_for_command(command)

    for code, marker, message in FATAL_MARKERS:
        match = _line_match(content, marker)
        if match:
            items.append(
                _diagnostic(
                    code=code,
                    severity="error",
                    category="numerics" if code == "result.residual_nan" else "runtime",
                    source=source,
                    message=message,
                    log_file=log_file,
                    matched_line=match["line"],
                    line_number=match["line_number"],
                    repair_hint="Review the failing OpenFOAM dictionary and solver log.",
                )
            )
            if code == "result.residual_nan":
                metrics["has_nan_or_inf"] = True

    _parse_boundary_errors(content, command=command, log_file=log_file, source=source, items=items, metrics=metrics)
    _parse_unknown_patch_errors(content, log_file=log_file, source=source, items=items)
    _parse_courant(content, log_file=log_file, source=source, items=items, metrics=metrics)
    _parse_numerics(content, log_file=log_file, source=source, items=items, metrics=metrics)

    warning_match = WARNING_PATTERN.search(content)
    if warning_match:
        line_number, line = _line_at(content, warning_match.start())
        items.append(
            _diagnostic(
                code="result.log_warning",
                severity="warning",
                category="runtime",
                source=source,
                message="OpenFOAM log contains a warning.",
                log_file=log_file,
                matched_line=line,
                line_number=line_number,
            )
        )

    if command.strip().split(" ")[0] == "blockMesh" and _blockmesh_failed(content, items):
        match = _first_error_line(content)
        items.append(
            _diagnostic(
                code="result.mesh_quality_failed",
                severity="error",
                category="mesh",
                source="blockMesh",
                message="blockMesh reported a mesh generation failure.",
                log_file=log_file,
                matched_line=match["line"],
                line_number=match["line_number"],
                repair_hint="Review blockMeshDict vertices, blocks, and patch definitions.",
            )
        )

    return _diagnostic_bundle(items, metrics)


def timeout_diagnostic(command: str, log_file: str, timeout_seconds: int) -> dict[str, Any]:
    item = _diagnostic(
        code="result.command_timeout",
        severity="error",
        category="runtime",
        source="runtime",
        message=f"{command} timed out after {timeout_seconds} seconds.",
        log_file=log_file,
        matched_line="",
        repair_hint="Inspect logs and reduce case cost or stabilize numerics before rerunning.",
    )
    item["timeout_seconds"] = timeout_seconds
    return item


def runtime_blocked_diagnostics(reason: str) -> dict[str, Any]:
    return _diagnostic_bundle([
        _diagnostic(
            code="result.runtime_unavailable",
            severity="error",
            category="runtime",
            source="runtime",
            message=reason or "OpenFOAM runtime is unavailable.",
            log_file="",
            matched_line="",
            repair_hint="Configure the selected OpenFOAM backend before running.",
        )
    ], {})


def _parse_boundary_errors(
    content: str,
    *,
    command: str,
    log_file: str,
    source: str,
    items: list[dict[str, Any]],
    metrics: dict[str, Any],
) -> None:
    field_name = _field_name_from_log(content) or _field_name_from_command(command)
    missing: dict[str, list[str]] = {}
    for match in MISSING_PATCH_FIELD_PATTERN.finditer(content):
        patch = match.group(1)
        line_number, line = _line_at(content, match.start())
        item = _diagnostic(
            code="result.missing_boundary_field",
            severity="error",
            category="boundary",
            source=source,
            message=f"Missing boundaryField entry for patch {patch}.",
            log_file=log_file,
            matched_line=line,
            line_number=line_number,
            repair_hint="Add the missing patch entry to the initial field dictionary.",
        )
        item["patch"] = patch
        if field_name:
            item["field"] = field_name
            item["repair_action_input"] = {
                "op": "add_missing_boundary_field",
                "path": f"case/0/{field_name}",
                "field": field_name,
                "patch": patch,
            }
            missing.setdefault(field_name, []).append(patch)
        items.append(item)
    if missing:
        metrics["missing_boundary_fields"] = missing


def _parse_unknown_patch_errors(
    content: str,
    *,
    log_file: str,
    source: str,
    items: list[dict[str, Any]],
) -> None:
    for pattern in UNKNOWN_PATCH_PATTERNS:
        for match in pattern.finditer(content):
            patch = match.group(1)
            line_number, line = _line_at(content, match.start())
            item = _diagnostic(
                code="result.unknown_patch",
                severity="error",
                category="boundary",
                source=source,
                message=f"OpenFOAM referenced unknown patch {patch}.",
                log_file=log_file,
                matched_line=line,
                line_number=line_number,
                repair_hint="Check patch names in blockMeshDict, boundary, and field dictionaries.",
            )
            item["patch"] = patch
            items.append(item)


def _parse_courant(
    content: str,
    *,
    log_file: str,
    source: str,
    items: list[dict[str, Any]],
    metrics: dict[str, Any],
) -> None:
    values = []
    for match in COURANT_PATTERN.finditer(content):
        value = _safe_float(match.group(1))
        if value is None:
            continue
        values.append(value)
        if value > HIGH_COURANT_THRESHOLD:
            line_number, line = _line_at(content, match.start())
            item = _diagnostic(
                code="result.courant_high",
                severity="error",
                category="numerics",
                source=source,
                message=f"Maximum Courant number {value:.3g} is above {HIGH_COURANT_THRESHOLD:.3g}.",
                log_file=log_file,
                matched_line=line,
                line_number=line_number,
                repair_hint="Enable adjustTimeStep or reduce deltaT before rerunning.",
            )
            item["max_courant"] = value
            items.append(item)
    if values:
        metrics["max_courant"] = max(values)


def _parse_numerics(
    content: str,
    *,
    log_file: str,
    source: str,
    items: list[dict[str, Any]],
    metrics: dict[str, Any],
) -> None:
    if NAN_PATTERN.search(content):
        match = NAN_PATTERN.search(content)
        if match:
            line_number, line = _line_at(content, match.start())
            items.append(
                _diagnostic(
                    code="result.residual_nan",
                    severity="error",
                    category="numerics",
                    source=source,
                    message="Solver log contains nan or inf.",
                    log_file=log_file,
                    matched_line=line,
                    line_number=line_number,
                    repair_hint="Stabilize timestep, boundary conditions, or mesh quality before rerunning.",
                )
            )
            metrics["has_nan_or_inf"] = True

    divergence_match = DIVERGENCE_PATTERN.search(content)
    if divergence_match:
        line_number, line = _line_at(content, divergence_match.start())
        items.append(
            _diagnostic(
                code="result.divergence",
                severity="error",
                category="numerics",
                source=source,
                message="Solver log indicates divergence.",
                log_file=log_file,
                matched_line=line,
                line_number=line_number,
                repair_hint="Review timestep, relaxation factors, boundary conditions, and mesh quality.",
            )
        )

    residuals = _residual_entries(content)
    if residuals:
        _merge_metrics(metrics, _residual_metrics(residuals))
        high = next((entry for entry in residuals if entry["final"] > HIGH_FINAL_RESIDUAL_THRESHOLD), None)
        if high:
            items.append(
                _diagnostic(
                    code="result.residual_high",
                    severity="error",
                    category="numerics",
                    source=source,
                    message=(
                        f"Final residual for {high['field']} is {high['final']:.3g}, "
                        f"above {HIGH_FINAL_RESIDUAL_THRESHOLD:.3g}."
                    ),
                    log_file=log_file,
                    matched_line=high["line"],
                    line_number=high["line_number"],
                    repair_hint="Review solver tolerance, relaxation factors, and boundary conditions.",
                )
            )


def _event_diagnostic(event: dict[str, Any]) -> dict[str, Any]:
    kind = str(event.get("kind", "")).strip()
    command = str(event.get("command", "")).strip()
    log_file = str(event.get("log_file", "")).strip()
    if kind == "timeout":
        return timeout_diagnostic(command, log_file, int(event.get("timeout_seconds", 0) or 0))
    if kind == "runtime_unavailable":
        return runtime_blocked_diagnostics(str(event.get("reason", "")).strip())["items"][0]
    return {}


def _diagnostic_bundle(items: list[dict[str, Any]], metrics: dict[str, Any]) -> dict[str, Any]:
    cleaned_items = [item for item in items if isinstance(item, dict) and item.get("code")]
    severity = _bundle_severity(cleaned_items)
    return {
        "severity": severity,
        "summary": _summary(cleaned_items, severity),
        "items": cleaned_items,
        "metrics": metrics if isinstance(metrics, dict) else {},
    }


def _bundle_severity(items: list[dict[str, Any]]) -> str:
    if any(item.get("severity") == "error" for item in items):
        return "failed"
    if any(item.get("severity") == "warning" for item in items):
        return "warning"
    return "passed"


def _summary(items: list[dict[str, Any]], severity: str) -> str:
    if not items:
        return "No OpenFOAM run diagnostics."
    errors = [item for item in items if item.get("severity") == "error"]
    primary = errors[0] if errors else items[0]
    return str(primary.get("message", "")).strip() or f"OpenFOAM diagnostics: {severity}."


def _diagnostic(
    *,
    code: str,
    severity: str,
    category: str,
    source: str,
    message: str,
    log_file: str,
    matched_line: str,
    line_number: int | None = None,
    repair_hint: str = "",
) -> dict[str, Any]:
    item = {
        "code": code,
        "severity": severity,
        "category": category,
        "message": message,
        "source": source,
        "log_file": log_file,
        "matched_line": _truncate(matched_line),
    }
    if line_number:
        item["line"] = line_number
    if repair_hint:
        item["repair_hint"] = repair_hint
    return item


def _step_log_path(step: dict[str, Any]) -> Path | None:
    text = str(step.get("log_path", "")).strip()
    if text:
        return Path(text)
    return None


def _display_log_file(step: dict[str, Any], log_path: Path) -> str:
    log_name = str(step.get("log", "")).strip() or log_path.name
    return f"logs/{log_name}" if not log_name.replace("\\", "/").startswith("logs/") else log_name.replace("\\", "/")


def _command_name(step: dict[str, Any]) -> str:
    command = str(step.get("command", "")).strip()
    return command.split()[0] if command else "OpenFOAM command"


def _source_for_command(command: str) -> str:
    name = command.strip().split()[0] if command.strip() else ""
    if name == "blockMesh":
        return "blockMesh"
    if name in {"icoFoam", "simpleFoam", "pisoFoam", "pimpleFoam"}:
        return "solver"
    return name or "runtime"


def _field_name_from_log(content: str) -> str:
    match = FIELD_PATH_PATTERN.search(content)
    return match.group(1) if match else ""


def _field_name_from_command(command: str) -> str:
    parts = command.strip().split()
    return parts[0] if len(parts) == 1 and parts[0] in {"U", "p", "k", "epsilon", "omega", "nut", "nuTilda"} else ""


def _blockmesh_failed(content: str, existing_items: list[dict[str, Any]]) -> bool:
    if any(item.get("source") == "blockMesh" and item.get("code") == "result.mesh_quality_failed" for item in existing_items):
        return False
    return bool(re.search(r"\*\*\*Error|FOAM FATAL ERROR|failed", content, re.IGNORECASE))


def _first_error_line(content: str) -> dict[str, Any]:
    match = re.search(r"\*\*\*Error|FOAM FATAL ERROR|failed", content, re.IGNORECASE)
    if not match:
        return {"line_number": 0, "line": ""}
    line_number, line = _line_at(content, match.start())
    return {"line_number": line_number, "line": line}


def _line_match(content: str, marker: str) -> dict[str, Any]:
    index = content.find(marker)
    if index < 0:
        return {}
    line_number, line = _line_at(content, index)
    return {"line_number": line_number, "line": line}


def _line_at(content: str, index: int) -> tuple[int, str]:
    index = max(0, index)
    start = content.rfind("\n", 0, index) + 1
    end = content.find("\n", index)
    if end < 0:
        end = len(content)
    line_number = content.count("\n", 0, start) + 1
    return line_number, _truncate(content[start:end].strip())


def _residual_entries(content: str) -> list[dict[str, Any]]:
    entries: list[dict[str, Any]] = []
    for match in RESIDUAL_PATTERN.finditer(content):
        initial = _safe_float(match.group(2))
        final = _safe_float(match.group(3))
        if initial is None or final is None:
            continue
        line_number, line = _line_at(content, match.start())
        entries.append({
            "field": match.group(1).strip(),
            "initial": initial,
            "final": final,
            "line_number": line_number,
            "line": line,
        })
    return entries


def _residual_metrics(residuals: list[dict[str, Any]]) -> dict[str, Any]:
    worst = max(residuals, key=lambda item: float(item["final"]))
    return {
        "residual_count": len(residuals),
        "residual_fields": sorted({str(item["field"]) for item in residuals}),
        "max_initial_residual": max(float(item["initial"]) for item in residuals),
        "max_final_residual": float(worst["final"]),
        "worst_residual_field": str(worst["field"]),
    }


def _metrics_from_items(items: list[dict[str, Any]]) -> dict[str, Any]:
    metrics: dict[str, Any] = {}
    for item in items:
        if item.get("code") == "result.residual_nan":
            metrics["has_nan_or_inf"] = True
        if item.get("code") == "result.courant_high":
            value = _safe_float(item.get("max_courant"))
            if value is not None:
                metrics["max_courant"] = max(float(metrics.get("max_courant", value)), value)
    return metrics


def _merge_metrics(target: dict[str, Any], incoming: dict[str, Any]) -> None:
    for key, value in incoming.items():
        if key in {"max_final_residual", "max_initial_residual", "max_courant"}:
            target[key] = max(float(target.get(key, value)), float(value))
        elif key == "residual_count":
            target[key] = int(target.get(key, 0)) + int(value)
        elif key == "residual_fields":
            existing = target.get(key, [])
            existing = existing if isinstance(existing, list) else []
            target[key] = sorted({*existing, *value})
        elif key == "has_nan_or_inf":
            target[key] = bool(target.get(key, False)) or bool(value)
        elif key == "missing_boundary_fields" and isinstance(value, dict):
            existing = target.setdefault(key, {})
            if not isinstance(existing, dict):
                existing = {}
                target[key] = existing
            for field, patches in value.items():
                current = existing.setdefault(field, [])
                current = current if isinstance(current, list) else []
                existing[field] = sorted({*current, *[str(patch) for patch in patches]})
        else:
            target[key] = value


def _safe_float(value: Any) -> float | None:
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def _safe_int(value: Any) -> int | None:
    try:
        return int(value)
    except (TypeError, ValueError):
        return None


def _truncate(text: str, limit: int = 240) -> str:
    if len(text) <= limit:
        return text
    return text[: limit - 3].rstrip() + "..."
