from pathlib import Path
import re

from ...models import SimulationPlan, TaskContext
from .mesh import GMSH_MESH_CASE_PATH, is_block_mesh_dict, plan_uses_external_gmsh_mesh


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

    boundary_errors = bool(boundary_validation.get("missing_boundary_fields") or boundary_validation.get("extra_boundary_fields"))
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
        if item.format != "openfoam-dict" or not item.path:
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
    block_mesh_path = task_dir / "case/system/blockMeshDict"
    if not block_mesh_path.exists():
        return {
            "mesh_patches": [],
            "field_patches": {},
            "missing_boundary_fields": {},
            "extra_boundary_fields": {},
        }

    mesh_patches = _parse_block_mesh_boundary_patches(block_mesh_path)
    field_files = _existing_initial_field_files(task_dir)

    field_patches = {}
    missing_boundary_fields = {}
    extra_boundary_fields = {}

    for rel_path in field_files:
        patches = _parse_boundary_field_patches(task_dir / rel_path)
        field_name = Path(rel_path).name
        field_patches[field_name] = patches
        missing = sorted(set(mesh_patches) - set(patches))
        extra = sorted(set(patches) - set(mesh_patches))
        if missing:
            missing_boundary_fields[field_name] = missing
        if extra:
            extra_boundary_fields[field_name] = extra

    return {
        "mesh_patches": mesh_patches,
        "field_patches": field_patches,
        "missing_boundary_fields": missing_boundary_fields,
        "extra_boundary_fields": extra_boundary_fields,
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


def _parse_block_mesh_boundary_patches(path: Path) -> list[str]:
    content = _strip_openfoam_comments(path.read_text(encoding="utf-8", errors="replace"))
    body = _extract_named_parenthesized_block(content, "boundary")
    if not body:
        return []

    patches = []
    for match in re.finditer(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*\{", body, flags=re.MULTILINE):
        name = match.group(1)
        if name not in {"type", "faces"}:
            patches.append(name)
    return patches


def _parse_boundary_field_patches(path: Path) -> list[str]:
    content = _strip_openfoam_comments(path.read_text(encoding="utf-8", errors="replace"))
    body = _extract_named_braced_block(content, "boundaryField")
    if not body:
        return []

    patches = []
    for match in re.finditer(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*\{", body, flags=re.MULTILINE):
        name = match.group(1)
        if name not in {"type", "value"}:
            patches.append(name)
    return patches


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
