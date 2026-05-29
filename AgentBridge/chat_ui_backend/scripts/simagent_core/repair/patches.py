from pathlib import Path
import os
import re
from typing import Any

from ..models import TaskContext


ALLOWED_OPS = {
    "set_dictionary_value",
    "add_missing_boundary_field",
    "adjust_time_step",
    "enable_adjust_time_step",
    "update_solver_tolerance",
}
IDENTIFIER_PATTERN = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


class RepairPatchError(ValueError):
    pass


def apply_repair_action(task: TaskContext, repair_action: dict[str, Any]) -> dict[str, Any]:
    patches = repair_action.get("patches", [])
    if not isinstance(patches, list) or not patches:
        return {"status": "no_patches", "applied": [], "skipped": []}

    prepared = _prepare_repair_patches(task, patches)
    for path, content in prepared["contents"].items():
        path.write_text(content, encoding="utf-8")

    return {
        "status": "applied" if prepared["applied"] else "no_changes",
        "applied": prepared["applied"],
        "skipped": prepared["skipped"],
    }


def _prepare_repair_patches(task: TaskContext, patches: list[Any]) -> dict[str, Any]:
    originals: dict[Path, str] = {}
    staged: dict[Path, str] = {}
    applied: list[dict[str, Any]] = []
    skipped: list[dict[str, Any]] = []

    for patch in patches:
        if not isinstance(patch, dict):
            raise RepairPatchError("repair patch must be an object")
        target, updated_content, result = _prepare_patch(task, patch, originals, staged)
        if result.get("changed"):
            staged[target] = updated_content
            applied.append(result)
        else:
            skipped.append(result)

    return {
        "contents": {
            path: content
            for path, content in staged.items()
            if content != originals.get(path, "")
        },
        "applied": applied,
        "skipped": skipped,
    }


def _prepare_patch(
    task: TaskContext,
    patch: dict[str, Any],
    originals: dict[Path, str],
    staged: dict[Path, str],
) -> tuple[Path, str, dict[str, Any]]:
    operation = str(patch.get("op", "")).strip()
    _require_allowed_op(operation)

    if operation == "set_dictionary_value":
        return _prepare_set_dictionary_value(task, patch, originals, staged)
    if operation == "add_missing_boundary_field":
        return _prepare_add_missing_boundary_field(task, patch, originals, staged)
    if operation == "adjust_time_step":
        patch = {**patch, "path": patch.get("path", "case/system/controlDict"), "key": "deltaT"}
        return _prepare_set_dictionary_value(task, patch, originals, staged, operation="adjust_time_step")
    if operation == "enable_adjust_time_step":
        return _prepare_enable_adjust_time_step(task, patch, originals, staged)
    if operation == "update_solver_tolerance":
        return _prepare_update_solver_tolerance(task, patch, originals, staged)
    raise RepairPatchError(f"unsupported repair patch op: {operation}")


def _prepare_set_dictionary_value(
    task: TaskContext,
    patch: dict[str, Any],
    originals: dict[Path, str],
    staged: dict[Path, str],
    *,
    operation: str = "set_dictionary_value",
) -> tuple[Path, str, dict[str, Any]]:
    rel_path = _relative_path(patch)
    path = _resolve_case_file(task, rel_path)
    key = _require_identifier(patch.get("key", ""), f"{operation}.key")
    value = _require_value(patch.get("value", ""), f"{operation}.value")
    if operation == "adjust_time_step":
        _require_positive_number(value, "adjust_time_step.value")

    content = _content_for(path, originals, staged)
    updated = _replace_or_append_assignment(content, key, value)
    changed = updated != content
    return path, updated, {"op": operation, "path": rel_path, "key": key, "value": value, "changed": changed}


def _prepare_enable_adjust_time_step(
    task: TaskContext,
    patch: dict[str, Any],
    originals: dict[Path, str],
    staged: dict[Path, str],
) -> tuple[Path, str, dict[str, Any]]:
    rel_path = str(patch.get("path", "case/system/controlDict")).strip()
    path = _resolve_case_file(task, rel_path)
    content = _content_for(path, originals, staged)
    value = _require_value(patch.get("value", "yes"), "enable_adjust_time_step.value")
    updated = _replace_or_append_assignment(content, "adjustTimeStep", value)
    max_co = _require_positive_number(
        patch.get("maxCo", patch.get("max_co", "0.5")),
        "enable_adjust_time_step.maxCo",
    )
    if max_co:
        updated = _replace_or_append_assignment(updated, "maxCo", max_co)
    changed = updated != content
    return path, updated, {"op": "enable_adjust_time_step", "path": rel_path, "changed": changed}


def _prepare_add_missing_boundary_field(
    task: TaskContext,
    patch: dict[str, Any],
    originals: dict[Path, str],
    staged: dict[Path, str],
) -> tuple[Path, str, dict[str, Any]]:
    rel_path = _relative_path(patch)
    path = _resolve_case_file(task, rel_path)
    patch_name = _require_identifier(patch.get("patch", ""), "add_missing_boundary_field.patch")
    field_name = _require_identifier(
        patch.get("field", Path(rel_path).name),
        "add_missing_boundary_field.field",
    )
    mesh_patch_type = _optional_identifier(
        patch.get("mesh_patch_type", ""),
        "add_missing_boundary_field.mesh_patch_type",
    )

    content = _content_for(path, originals, staged)
    boundary = _find_named_block(content, "boundaryField")
    if not boundary:
        raise RepairPatchError(f"boundaryField block not found in {rel_path}")
    if _find_named_block(content, patch_name, boundary[1] + 1, boundary[2]):
        return path, content, {
            "op": "add_missing_boundary_field",
            "path": rel_path,
            "patch": patch_name,
            "changed": False,
        }

    patch_type = _optional_identifier(patch.get("type", ""), "add_missing_boundary_field.type")
    patch_type = patch_type or _default_boundary_type(field_name, patch_name, mesh_patch_type)
    value = _optional_value(patch.get("value", ""), "add_missing_boundary_field.value")
    value = value or _default_boundary_value(field_name, patch_name, patch_type)
    block = _boundary_patch_block(patch_name, patch_type, value)
    updated = content[:boundary[2]].rstrip() + "\n" + block + "\n" + content[boundary[2]:]
    return path, updated, {
        "op": "add_missing_boundary_field",
        "path": rel_path,
        "patch": patch_name,
        "type": patch_type,
        "changed": True,
    }


def _prepare_update_solver_tolerance(
    task: TaskContext,
    patch: dict[str, Any],
    originals: dict[Path, str],
    staged: dict[Path, str],
) -> tuple[Path, str, dict[str, Any]]:
    rel_path = str(patch.get("path", "case/system/fvSolution")).strip()
    path = _resolve_case_file(task, rel_path)
    solver = _require_identifier(patch.get("solver", patch.get("field", "p")), "update_solver_tolerance.solver")
    tolerance = _require_positive_number(
        patch.get("tolerance", "1e-07"),
        "update_solver_tolerance.tolerance",
    )
    rel_tol = _require_non_negative_number(
        patch.get("relTol", patch.get("rel_tol", "0")),
        "update_solver_tolerance.relTol",
    )

    content = _content_for(path, originals, staged)
    block = _find_named_block(content, solver)
    if not block:
        return path, content, {
            "op": "update_solver_tolerance",
            "path": rel_path,
            "solver": solver,
            "changed": False,
        }
    block_content = content[block[1] + 1:block[2]]
    updated_block = _replace_or_append_assignment(block_content, "tolerance", tolerance)
    if rel_tol:
        updated_block = _replace_or_append_assignment(updated_block, "relTol", rel_tol)
    updated = content[:block[1] + 1] + updated_block + content[block[2]:]
    changed = updated != content
    return path, updated, {
        "op": "update_solver_tolerance",
        "path": rel_path,
        "solver": solver,
        "tolerance": tolerance,
        "changed": changed,
    }


def _content_for(path: Path, originals: dict[Path, str], staged: dict[Path, str]) -> str:
    if path in staged:
        return staged[path]
    if path not in originals:
        originals[path] = _read_existing(path)
    return originals[path]


def _require_allowed_op(operation: str) -> None:
    if not operation:
        raise RepairPatchError("repair patch op must not be empty")
    if operation not in ALLOWED_OPS:
        raise RepairPatchError(f"unsupported repair patch op: {operation}")


def _require_identifier(value: Any, field_name: str) -> str:
    text = str(value or "").strip()
    if not text or not IDENTIFIER_PATTERN.fullmatch(text):
        raise RepairPatchError(f"{field_name} must be an OpenFOAM identifier")
    return text


def _optional_identifier(value: Any, field_name: str) -> str:
    text = str(value or "").strip()
    if not text:
        return ""
    if not IDENTIFIER_PATTERN.fullmatch(text):
        raise RepairPatchError(f"{field_name} must be an OpenFOAM identifier")
    return text


def _require_value(value: Any, field_name: str) -> str:
    text = str(value if value is not None else "").strip()
    if not text:
        raise RepairPatchError(f"{field_name} must not be empty")
    if "\n" in text or "\r" in text:
        raise RepairPatchError(f"{field_name} must not contain newlines")
    return text


def _optional_value(value: Any, field_name: str) -> str:
    text = str(value if value is not None else "").strip()
    if not text:
        return ""
    if "\n" in text or "\r" in text:
        raise RepairPatchError(f"{field_name} must not contain newlines")
    return text


def _require_positive_number(value: Any, field_name: str) -> str:
    text = _require_value(value, field_name)
    try:
        parsed = float(text)
    except ValueError as exc:
        raise RepairPatchError(f"{field_name} must be a positive number") from exc
    if parsed <= 0:
        raise RepairPatchError(f"{field_name} must be a positive number")
    return text


def _require_non_negative_number(value: Any, field_name: str) -> str:
    text = _require_value(value, field_name)
    try:
        parsed = float(text)
    except ValueError as exc:
        raise RepairPatchError(f"{field_name} must be a non-negative number") from exc
    if parsed < 0:
        raise RepairPatchError(f"{field_name} must be a non-negative number")
    return text


def _relative_path(patch: dict[str, Any]) -> str:
    rel_path = str(patch.get("path", "")).strip()
    if not rel_path:
        raise RepairPatchError("repair patch path must not be empty")
    return rel_path


def _resolve_case_file(task: TaskContext, rel_path: str) -> Path:
    normalized = rel_path.replace("\\", "/").strip()
    if normalized.startswith("/"):
        raise RepairPatchError(f"repair patch path escapes task case: {rel_path}")
    cleaned = normalized.strip("/")
    raw_path = Path(cleaned)
    if raw_path.is_absolute() or ".." in raw_path.parts:
        raise RepairPatchError(f"repair patch path escapes task case: {rel_path}")
    if not cleaned.startswith("case/"):
        raise RepairPatchError(f"repair patch path must stay under case/: {rel_path}")

    task_root = Path(task.task_dir).resolve()
    case_root = (task_root / "case").resolve()
    target = (task_root / cleaned).resolve()
    try:
        common = os.path.commonpath([str(case_root), str(target)])
    except ValueError as exc:
        raise RepairPatchError(f"repair patch path escapes task case: {rel_path}") from exc
    if common != str(case_root):
        raise RepairPatchError(f"repair patch path escapes task case: {rel_path}")
    return target


def _read_existing(path: Path) -> str:
    if not path.exists() or not path.is_file():
        raise RepairPatchError(f"repair target file does not exist: {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def _replace_or_append_assignment(content: str, key: str, value: str) -> str:
    pattern = re.compile(rf"(^\s*{re.escape(key)}\s+)([^;]+)(;)", flags=re.MULTILINE)
    updated, count = pattern.subn(rf"\g<1>{value}\3", content, count=1)
    if count:
        return updated
    insertion = f"\n{key}        {value};\n"
    return content.rstrip() + insertion


def _default_boundary_type(field_name: str, patch_name: str, mesh_patch_type: str) -> str:
    name = patch_name.lower()
    mesh_type = mesh_patch_type.lower()
    if mesh_type == "empty":
        return "empty"
    if field_name == "U":
        if mesh_type == "wall" or "wall" in name:
            return "noSlip"
        if "inlet" in name:
            return "fixedValue"
        return "zeroGradient"
    if field_name == "p":
        if "outlet" in name:
            return "fixedValue"
        return "zeroGradient"
    return "calculated"


def _default_boundary_value(field_name: str, patch_name: str, patch_type: str) -> str:
    if patch_type != "fixedValue":
        return ""
    if field_name == "U":
        return "uniform (0 0 0)"
    return "uniform 0"


def _boundary_patch_block(patch_name: str, patch_type: str, value: str) -> str:
    lines = [
        f"    {patch_name}",
        "    {",
        f"        type            {patch_type};",
    ]
    if value:
        lines.append(f"        value           {value};")
    lines.append("    }")
    return "\n".join(lines)


def _find_named_block(content: str, name: str, start: int = 0, end: int | None = None) -> tuple[int, int, int] | None:
    search_end = len(content) if end is None else end
    match = re.search(rf"(?m)^[ \t]*{re.escape(name)}\s*\{{", content[start:search_end])
    if not match:
        return None
    block_start = start + match.start()
    open_brace = content.find("{", block_start, start + match.end())
    if open_brace < 0:
        return None
    close_brace = _find_matching_brace(content, open_brace, search_end)
    if close_brace < 0:
        return None
    return block_start, open_brace, close_brace


def _find_matching_brace(content: str, open_brace: int, end: int) -> int:
    depth = 0
    for index in range(open_brace, end):
        char = content[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
    return -1
