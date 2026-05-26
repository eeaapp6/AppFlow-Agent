import re
from pathlib import Path
from typing import Any


GMSH_MESH_FILE_NAME = "geometry.msh"
GMSH_MESH_CASE_PATH = f"case/{GMSH_MESH_FILE_NAME}"
MESH_MODE_REFERENCE = "reference_mesh"
MESH_MODE_GMSH_EXTERNAL = "gmsh_external"


def detect_gmsh_mesh_request(user_requirement: str) -> dict[str, Any]:
    """Return the mesh plan implied by the user text.

    This only detects external .msh import. Future free/parametric GMSH generation
    should use a different mode instead of overloading this one.
    """
    text = str(user_requirement or "")
    source_path = _extract_msh_path(text)
    if source_path:
        return external_gmsh_mesh_request(source_path)

    lowered = text.lower()
    if "gmsh" in lowered or ".msh" in lowered:
        request = external_gmsh_mesh_request("")
        request["missing_source"] = True
        return request

    return {"mode": MESH_MODE_REFERENCE}


def external_gmsh_mesh_request(source_path: str) -> dict[str, Any]:
    return {
        "mode": MESH_MODE_GMSH_EXTERNAL,
        "source_path": str(source_path or "").strip(),
        "case_path": GMSH_MESH_CASE_PATH,
    }


def mesh_request_from_parameters(parameters: dict[str, Any]) -> dict[str, Any]:
    mesh = parameters.get("mesh", {}) if isinstance(parameters, dict) else {}
    return mesh if isinstance(mesh, dict) else {}


def plan_uses_external_gmsh_mesh(parameters: dict[str, Any]) -> bool:
    return mesh_request_from_parameters(parameters).get("mode") == MESH_MODE_GMSH_EXTERNAL


def external_gmsh_source_path(parameters: dict[str, Any]) -> str:
    mesh = mesh_request_from_parameters(parameters)
    return str(mesh.get("source_path", "")).strip().strip('"')


def is_block_mesh_dict(path: str) -> bool:
    return str(path).replace("\\", "/") == "case/system/blockMeshDict"


def _extract_msh_path(text: str) -> str:
    quoted = re.search(r"""["']([^"']+\.msh)["']""", text, flags=re.IGNORECASE)
    if quoted:
        return quoted.group(1).strip()

    windows = re.search(r"([A-Za-z]:[\\/][^\r\n,;]+?\.msh)", text, flags=re.IGNORECASE)
    if windows:
        return windows.group(1).strip().strip('"')

    posix = re.search(r"((?:/|\.{1,2}/)[^\r\n,;]+?\.msh)", text, flags=re.IGNORECASE)
    if posix:
        return posix.group(1).strip().strip('"')

    bare = re.search(r"([^\s,;]+\.msh)", text, flags=re.IGNORECASE)
    if bare:
        return bare.group(1).strip().strip('"')

    return ""


def copy_external_gmsh_mesh(task_dir: Path, source_path: str) -> Path:
    source = Path(source_path).expanduser()
    if not source.exists() or not source.is_file():
        raise FileNotFoundError(f"GMSH mesh file not found: {source}")
    if source.suffix.lower() != ".msh":
        raise ValueError(f"GMSH mesh file must use .msh extension: {source}")

    target = task_dir / "case" / GMSH_MESH_FILE_NAME
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(source.read_bytes())
    return target
