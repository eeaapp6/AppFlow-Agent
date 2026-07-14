from pathlib import Path
import re
from typing import Any

from ...models import SimulationPlan, TaskContext
from .mesh import GMSH_MESH_CASE_PATH, is_block_mesh_dict, plan_uses_external_gmsh_mesh
from .knowledge.tutorial_details import is_reference_script_path


REQUIRED_OPENFOAM_FILES = [
    "case/system/controlDict",
    "case/system/fvSchemes",
    "case/system/fvSolution",
    "case/system/blockMeshDict",
    "case/constant/physicalProperties",
    "case/0/U",
    "case/0/p",
]


def required_openfoam_files(plan: SimulationPlan | None = None) -> list[str]:
    if not plan:
        return list(REQUIRED_OPENFOAM_FILES)

    files = [
        item.path
        for item in plan.planned_files
        if item.required and item.path
    ]
    if plan_uses_external_gmsh_mesh(plan.parameters):
        files = [
            item
            for item in files
            if not is_block_mesh_dict(item)
        ]
        if GMSH_MESH_CASE_PATH not in files:
            files.append(GMSH_MESH_CASE_PATH)
    return sorted(set(files)) or list(REQUIRED_OPENFOAM_FILES)


def validate_openfoam_case(task: TaskContext, plan: SimulationPlan | None = None) -> dict:
    task_dir = Path(task.task_dir)
    checked_files = []
    missing_files = []

    for rel_path in required_openfoam_files(plan):
        checked_files.append(rel_path)
        if not (task_dir / rel_path).exists():
            missing_files.append(rel_path)

    boundary_validation = {}
    dictionary_validation = {}
    if not missing_files:
        boundary_validation = _validate_boundary_fields(task_dir)
        dictionary_validation = _validate_dictionary_content(task_dir, plan)

    boundary_errors = bool(
        boundary_validation.get("missing_boundary_fields")
        or boundary_validation.get("extra_boundary_fields")
        or boundary_validation.get("empty_boundary_fields")
        or boundary_validation.get("boundary_condition_errors")
    )
    dictionary_errors = bool(dictionary_validation.get("dictionary_errors"))

    return {
        "status": "validated" if not missing_files and not boundary_errors and not dictionary_errors else "validation_failed",
        "checked_files": checked_files,
        "missing_files": missing_files,
        **boundary_validation,
        **dictionary_validation,
    }


def _validate_dictionary_content(task_dir: Path, plan: SimulationPlan | None) -> dict:
    errors = []
    files_to_check = _dictionary_files_to_check(plan)

    for rel_path, expected_object in files_to_check.items():
        path = task_dir / rel_path
        if not path.exists():
            continue
        content = _strip_openfoam_comments((task_dir / rel_path).read_text(encoding="utf-8", errors="replace"))
        actual_object = _extract_openfoam_assignment(content, "object")
        if actual_object and actual_object != expected_object:
            errors.append({
                "file": rel_path,
                "message": f"FoamFile object should be {expected_object}, got {actual_object}.",
            })
        if not actual_object:
            errors.append({
                "file": rel_path,
                "message": "FoamFile object is missing.",
            })

    control_path = task_dir / "case/system/controlDict"
    if control_path.exists():
        control_content = _strip_openfoam_comments(control_path.read_text(encoding="utf-8", errors="replace"))
        application = _extract_openfoam_assignment(control_content, "application")
        expected_solver = plan.solver_name if plan else ""
        if expected_solver and application and application != expected_solver:
            errors.append({
                "file": "case/system/controlDict",
                "message": f"application should be {expected_solver}, got {application}.",
            })
        if not application:
            errors.append({
                "file": "case/system/controlDict",
                "message": "application is missing.",
            })

    physical_path = task_dir / "case/constant/physicalProperties"
    if physical_path.exists() and (not plan or plan.physics_domain.lower() == "incompressible"):
        physical_content = _strip_openfoam_comments(physical_path.read_text(encoding="utf-8", errors="replace"))
        if not re.search(r"^\s*nu\s+\[[^\]]+\]\s+[^;]+;", physical_content, flags=re.MULTILINE):
            errors.append({
                "file": "case/constant/physicalProperties",
                "message": "nu with dimensions is missing.",
            })

    for rel_path in _initial_field_files(plan):
        path = task_dir / rel_path
        if not path.exists():
            continue
        field_content = _strip_openfoam_comments((task_dir / rel_path).read_text(encoding="utf-8", errors="replace"))
        if not re.search(r"^\s*dimensions\s+\[[^\]]+\]\s*;", field_content, flags=re.MULTILINE):
            errors.append({
                "file": rel_path,
                "message": "dimensions entry is missing.",
            })

    return {"dictionary_errors": errors}


def _dictionary_files_to_check(plan: SimulationPlan | None) -> dict[str, str]:
    if not plan:
        return {
            "case/system/controlDict": "controlDict",
            "case/system/fvSchemes": "fvSchemes",
            "case/system/fvSolution": "fvSolution",
            "case/system/blockMeshDict": "blockMeshDict",
            "case/constant/physicalProperties": "physicalProperties",
            "case/0/U": "U",
            "case/0/p": "p",
        }

    files = {}
    for item in plan.planned_files:
        if item.format != "openfoam-dict" or not item.path or is_reference_script_path(item.path):
            continue
        files[item.path] = Path(item.path).name
    return files


def _initial_field_files(plan: SimulationPlan | None) -> list[str]:
    if not plan:
        return ["case/0/U", "case/0/p"]
    return sorted(
        item.path
        for item in plan.planned_files
        if item.path.startswith("case/0/") and item.format == "openfoam-dict"
    )


def _validate_boundary_fields(task_dir: Path) -> dict:
    mesh_patch_map = _parse_mesh_patch_map(task_dir)
    if not mesh_patch_map:
        return {
            "mesh_patches": [],
            "mesh_patch_types": {},
            "field_patches": {},
            "boundary_field_types": {},
            "missing_boundary_fields": {},
            "extra_boundary_fields": {},
            "empty_boundary_fields": [],
            "boundary_condition_errors": [],
            "boundary_condition_warnings": [],
        }

    mesh_patches = sorted(mesh_patch_map)
    mesh_patch_types = {name: mesh_patch_map[name].get("type", "") for name in mesh_patches}
    field_files = _existing_initial_field_files(task_dir)

    field_patches = {}
    boundary_field_types = {}
    missing_boundary_fields = {}
    extra_boundary_fields = {}
    empty_boundary_fields = []
    boundary_condition_errors = []
    boundary_condition_warnings = []

    for rel_path in field_files:
        field_patch_map = _parse_boundary_field_patch_map(task_dir / rel_path)
        patches = sorted(field_patch_map)
        field_name = Path(rel_path).name
        field_patches[field_name] = patches
        boundary_field_types[field_name] = {
            patch_name: field_patch_map[patch_name].get("type", "")
            for patch_name in patches
        }
        if not patches:
            empty_boundary_fields.append(field_name)
        missing = sorted(set(mesh_patches) - set(patches))
        extra = sorted(set(patches) - set(mesh_patches))
        if missing:
            missing_boundary_fields[field_name] = missing
        if extra:
            extra_boundary_fields[field_name] = extra
        condition_errors, condition_warnings = _review_field_boundary_conditions(
            rel_path,
            field_name,
            mesh_patch_map,
            field_patch_map,
        )
        boundary_condition_errors.extend(condition_errors)
        boundary_condition_warnings.extend(condition_warnings)

    return {
        "mesh_patches": mesh_patches,
        "mesh_patch_types": mesh_patch_types,
        "field_patches": field_patches,
        "boundary_field_types": boundary_field_types,
        "missing_boundary_fields": missing_boundary_fields,
        "extra_boundary_fields": extra_boundary_fields,
        "empty_boundary_fields": empty_boundary_fields,
        "boundary_condition_errors": boundary_condition_errors,
        "boundary_condition_warnings": boundary_condition_warnings,
    }


def _existing_initial_field_files(task_dir: Path) -> list[str]:
    zero_dir = task_dir / "case/0"
    if not zero_dir.exists():
        return []
    return [
        f"case/0/{item.name}"
        for item in sorted(zero_dir.iterdir())
        if item.is_file()
    ]


def _parse_mesh_patch_map(task_dir: Path) -> dict[str, dict[str, str]]:
    poly_boundary_path = task_dir / "case/constant/polyMesh/boundary"
    if poly_boundary_path.exists():
        patch_map = _parse_poly_mesh_boundary_patches(poly_boundary_path)
        if patch_map:
            return patch_map

    block_mesh_path = task_dir / "case/system/blockMeshDict"
    if block_mesh_path.exists():
        return _parse_block_mesh_boundary_patches(block_mesh_path)

    return {}


def _parse_block_mesh_boundary_patches(path: Path) -> dict[str, dict[str, str]]:
    content = _strip_openfoam_comments(path.read_text(encoding="utf-8", errors="replace"))
    body = _extract_named_parenthesized_block(content, "boundary")
    if not body:
        return {}

    return _patch_map_from_body(body)


def _parse_poly_mesh_boundary_patches(path: Path) -> dict[str, dict[str, str]]:
    content = _strip_openfoam_comments(path.read_text(encoding="utf-8", errors="replace"))
    body = _extract_first_parenthesized_block(content)
    if not body:
        return {}
    return _patch_map_from_body(body)


def _patch_map_from_body(body: str) -> dict[str, dict[str, str]]:
    patches: dict[str, dict[str, str]] = {}
    for name, block in _iter_named_braced_blocks(body):
        if name in {"type", "faces", "value"}:
            continue
        patches[name] = {
            "type": _extract_openfoam_assignment(block, "type"),
        }
    return patches


def _parse_boundary_field_patch_map(path: Path) -> dict[str, dict[str, Any]]:
    content = _strip_openfoam_comments(path.read_text(encoding="utf-8", errors="replace"))
    body = _extract_named_braced_block(content, "boundaryField")
    if not body:
        return {}

    patches: dict[str, dict[str, Any]] = {}
    for name, block in _iter_named_braced_blocks(body):
        if name in {"type", "value"}:
            continue
        patches[name] = {
            "type": _extract_openfoam_assignment(block, "type"),
            "has_value": _has_openfoam_assignment(block, "value"),
        }
    return patches


def _review_field_boundary_conditions(
    rel_path: str,
    field_name: str,
    mesh_patch_map: dict[str, dict[str, str]],
    field_patch_map: dict[str, dict[str, Any]],
) -> tuple[list[dict], list[dict]]:
    errors: list[dict] = []
    warnings: list[dict] = []
    for patch_name, field_patch in field_patch_map.items():
        mesh_patch = mesh_patch_map.get(patch_name, {})
        patch_type = str(field_patch.get("type", "")).strip()
        mesh_type = str(mesh_patch.get("type", "")).strip()
        patch_role = _patch_role(patch_name, mesh_type)
        if not patch_type:
            errors.append(_boundary_condition_error(
                "boundary.missing_type",
                rel_path,
                field_name,
                patch_name,
                "Boundary patch type is missing.",
            ))
            continue
        condition_errors, condition_warnings = _review_patch_condition(
            rel_path,
            field_name,
            patch_name,
            patch_role,
            patch_type,
            field_patch,
        )
        errors.extend(condition_errors)
        warnings.extend(condition_warnings)
    return errors, warnings


def _review_patch_condition(
    rel_path: str,
    field_name: str,
    patch_name: str,
    patch_role: str,
    patch_type: str,
    field_patch: dict[str, Any],
) -> tuple[list[dict], list[dict]]:
    normalized_type = patch_type.strip()
    errors: list[dict] = []
    warnings: list[dict] = []
    if patch_role == "empty" and normalized_type != "empty":
        errors.append(_boundary_condition_error(
            "boundary.empty_patch_type",
            rel_path,
            field_name,
            patch_name,
            f"Patch {patch_name} is an empty mesh patch but field {field_name} uses {patch_type}.",
        ))
    if field_name == "U":
        condition_errors, condition_warnings = _review_velocity_boundary(
            rel_path,
            field_name,
            patch_name,
            patch_role,
            normalized_type,
        )
        errors.extend(condition_errors)
        warnings.extend(condition_warnings)
    elif field_name == "p":
        condition_errors, condition_warnings = _review_pressure_boundary(
            rel_path,
            field_name,
            patch_name,
            patch_role,
            normalized_type,
            field_patch,
        )
        errors.extend(condition_errors)
        warnings.extend(condition_warnings)
    return errors, warnings


def _review_velocity_boundary(
    rel_path: str,
    field_name: str,
    patch_name: str,
    patch_role: str,
    patch_type: str,
) -> tuple[list[dict], list[dict]]:
    if patch_role == "inlet" and patch_type == "zeroGradient":
        return [_boundary_condition_error(
            "boundary.inlet_velocity_zero_gradient",
            rel_path,
            field_name,
            patch_name,
            "Velocity inlet should not use zeroGradient.",
        )], []
    if patch_role == "wall" and patch_type == "zeroGradient":
        return [_boundary_condition_error(
            "boundary.wall_velocity_unphysical",
            rel_path,
            field_name,
            patch_name,
            f"Velocity wall patch should use noSlip, fixedValue, or a wall function, got {patch_type}.",
        )], []
    if patch_role == "wall" and patch_type == "slip":
        return [], [_boundary_condition_warning(
            "boundary.wall_velocity_slip",
            rel_path,
            field_name,
            patch_name,
            "Velocity wall patch uses slip; verify this is intended for the selected wall.",
        )]
    return [], []


def _review_pressure_boundary(
    rel_path: str,
    field_name: str,
    patch_name: str,
    patch_role: str,
    patch_type: str,
    field_patch: dict[str, Any],
) -> tuple[list[dict], list[dict]]:
    if patch_role == "outlet" and patch_type == "fixedValue" and field_patch.get("has_value") is not True:
        return [_boundary_condition_error(
            "boundary.outlet_pressure_fixed_value_without_value",
            rel_path,
            field_name,
            patch_name,
            "Pressure outlet fixedValue must include an explicit value.",
        )], []
    if patch_role == "wall" and patch_type == "fixedValue":
        return [], [_boundary_condition_warning(
            "boundary.wall_pressure_fixed_value",
            rel_path,
            field_name,
            patch_name,
            "Pressure wall patch uses fixedValue; zeroGradient or a pressure wall function is usually preferred.",
        )]
    return [], []


def _patch_role(patch_name: str, mesh_type: str) -> str:
    name = patch_name.lower()
    patch_type = mesh_type.lower()
    if patch_type == "empty":
        return "empty"
    if patch_type == "wall" or "wall" in name:
        return "wall"
    if "inlet" in name:
        return "inlet"
    if "outlet" in name:
        return "outlet"
    return "unknown"


def _boundary_condition_error(code: str, file: str, field: str, patch: str, message: str) -> dict:
    return {
        "code": code,
        "file": file,
        "field": field,
        "patch": patch,
        "message": message,
    }


def _boundary_condition_warning(code: str, file: str, field: str, patch: str, message: str) -> dict:
    warning = _boundary_condition_error(code, file, field, patch, message)
    warning["severity"] = "warning"
    return warning


def _strip_openfoam_comments(content: str) -> str:
    content = re.sub(r"/\*.*?\*/", "", content, flags=re.DOTALL)
    content = re.sub(r"//.*", "", content)
    return content


def _extract_openfoam_assignment(content: str, key: str) -> str:
    match = re.search(rf"^\s*{re.escape(key)}\s+([^;]+);", content, flags=re.MULTILINE)
    return match.group(1).strip().strip('"') if match else ""


def _extract_named_parenthesized_block(content: str, name: str) -> str:
    start = re.search(rf"\b{re.escape(name)}\s*\(", content)
    if not start:
        return ""
    open_index = content.find("(", start.end() - 1)
    close_index = _find_matching_delimiter(content, open_index, "(", ")")
    return content[open_index + 1:close_index] if close_index != -1 else ""


def _extract_named_braced_block(content: str, name: str) -> str:
    start = re.search(rf"\b{re.escape(name)}\s*\{{", content)
    if not start:
        return ""
    open_index = content.find("{", start.end() - 1)
    close_index = _find_matching_delimiter(content, open_index, "{", "}")
    return content[open_index + 1:close_index] if close_index != -1 else ""


def _extract_first_parenthesized_block(content: str) -> str:
    open_index = content.find("(")
    if open_index == -1:
        return ""
    close_index = _find_matching_delimiter(content, open_index, "(", ")")
    return content[open_index + 1:close_index] if close_index != -1 else ""


def _iter_named_braced_blocks(content: str) -> list[tuple[str, str]]:
    blocks: list[tuple[str, str]] = []
    cursor = 0
    pattern = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*\{", flags=re.MULTILINE)
    while True:
        match = pattern.search(content, cursor)
        if not match:
            break
        name = match.group(1)
        open_index = content.find("{", match.end() - 1)
        close_index = _find_matching_delimiter(content, open_index, "{", "}")
        if close_index == -1:
            cursor = match.end()
            continue
        blocks.append((name, content[open_index + 1:close_index]))
        cursor = close_index + 1
    return blocks


def _has_openfoam_assignment(content: str, key: str) -> bool:
    return bool(re.search(rf"^\s*{re.escape(key)}\s+[^;]+;", content, flags=re.MULTILINE))


def _find_matching_delimiter(content: str, open_index: int, open_char: str, close_char: str) -> int:
    depth = 0
    for index in range(open_index, len(content)):
        char = content[index]
        if char == open_char:
            depth += 1
        elif char == close_char:
            depth -= 1
            if depth == 0:
                return index
    return -1
