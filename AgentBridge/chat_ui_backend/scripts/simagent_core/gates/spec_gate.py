from typing import Any

from .models import GateResult
from ..spec import BoundarySpec, GeometrySpec, MeshSpec, SimulationSpec, SolverSpec


SUPPORTED_GEOMETRY_TYPES = {
    "reference_case",
    "external_gmsh",
    "free_gmsh",
    "rect_channel",
    "pipe_2d",
    "cylinder_channel",
    "t_pipe",
    "cylinder_union",
}
EXPERIMENTAL_GEOMETRY_TYPES = {"free_gmsh"}
SUPPORTED_MESH_TYPES = {"reference_mesh", "gmsh_external", "blockMesh", "snappyHexMesh", "gmsh"}
SUPPORTED_BOUNDARY_ROLES = {"velocity_inlet", "pressure_outlet", "wall", "empty", "unknown"}
SUPPORTED_SOLVERS = {"icoFoam", "simpleFoam", "pimpleFoam"}


def review_simulation_spec(spec: SimulationSpec) -> GateResult:
    result = GateResult("spec")
    _review_geometry(spec.geometry, result)
    _review_mesh(spec.mesh, result)
    _review_boundaries(spec.boundaries, result)
    _review_solver(spec.solver, result)
    return result


def _review_geometry(geometry: GeometrySpec, result: GateResult) -> None:
    if not geometry.type:
        result.add_error("geometry.missing_type", "SimulationSpec.geometry.type is required.")
        return
    if geometry.type not in SUPPORTED_GEOMETRY_TYPES:
        result.add_error("geometry.unsupported_type", f"Unsupported geometry type: {geometry.type}.")
    elif geometry.type in EXPERIMENTAL_GEOMETRY_TYPES:
        result.add_warning("geometry.experimental_type", f"Geometry type is experimental: {geometry.type}.")

    parts = geometry.parameters.get("parts")
    if isinstance(parts, list) and len(parts) > 4:
        result.add_warning("geometry.many_parts", "More than four geometry parts may be unreliable in this phase.")

    for field_name in ["radius", "length", "height", "width", "depth"]:
        _require_positive_number_if_present(geometry.parameters, field_name, result)


def _review_mesh(mesh: MeshSpec, result: GateResult) -> None:
    if not mesh.type:
        result.add_error("mesh.missing_type", "SimulationSpec.mesh.type is required.")
        return
    if mesh.type not in SUPPORTED_MESH_TYPES:
        result.add_error("mesh.unsupported_type", f"Unsupported mesh type: {mesh.type}.")


def _review_boundaries(boundaries: list[BoundarySpec], result: GateResult) -> None:
    names: set[str] = set()
    for boundary in boundaries:
        if not boundary.name:
            result.add_error("boundary.missing_name", "Boundary name is required.")
        if boundary.name in names:
            result.add_error("boundary.duplicate_name", f"Duplicate boundary name: {boundary.name}.")
        names.add(boundary.name)
        if boundary.role not in SUPPORTED_BOUNDARY_ROLES:
            result.add_error("boundary.unsupported_role", f"Unsupported boundary role: {boundary.role}.")


def _review_solver(solver: SolverSpec, result: GateResult) -> None:
    if not solver.name:
        result.add_error("solver.missing_name", "SimulationSpec.solver.name is required.")
        return
    if solver.name not in SUPPORTED_SOLVERS:
        result.add_error("solver.unsupported_name", f"Unsupported solver: {solver.name}.")


def _require_positive_number_if_present(data: dict[str, Any], key: str, result: GateResult) -> None:
    if key not in data:
        return
    try:
        value = float(data[key])
    except (TypeError, ValueError):
        result.add_error(f"geometry.invalid_{key}", f"Geometry parameter {key} must be numeric.")
        return
    if value <= 0:
        result.add_error(f"geometry.non_positive_{key}", f"Geometry parameter {key} must be positive.")
