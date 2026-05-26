from typing import Any

from ..models import ReferenceCase, SimulationPlan
from ..solvers.openfoam.mesh import mesh_request_from_parameters, plan_uses_external_gmsh_mesh
from .simulation_spec import BoundarySpec, GeometrySpec, MeshSpec, SimulationSpec, SolverSpec


def simulation_spec_from_openfoam_plan(plan: SimulationPlan) -> SimulationSpec:
    mesh_request = mesh_request_from_parameters(plan.parameters)
    geometry = _geometry_from_plan(plan, mesh_request)
    mesh = _mesh_from_plan(mesh_request)
    return SimulationSpec(
        geometry=geometry,
        mesh=mesh,
        boundaries=_default_boundaries(plan.reference_case),
        solver=SolverSpec(
            name=plan.solver_name,
            case_type=_case_type(plan),
            parameters=_solver_parameters(plan),
        ),
        generation_mode=plan.generation_mode,
    )


def _geometry_from_plan(plan: SimulationPlan, mesh_request: dict[str, Any]) -> GeometrySpec:
    if plan_uses_external_gmsh_mesh(plan.parameters):
        return GeometrySpec(
            type="external_gmsh",
            source=str(mesh_request.get("source_path", "")).strip(),
            parameters={"case_path": mesh_request.get("case_path", "case/geometry.msh")},
        )
    if plan.reference_case:
        return GeometrySpec(
            type="reference_case",
            source=plan.reference_case.case_name,
            parameters=plan.reference_case.to_dict(),
        )
    return GeometrySpec(type="reference_case", source=plan.case_name, parameters={})


def _mesh_from_plan(mesh_request: dict[str, Any]) -> MeshSpec:
    mesh_mode = str(mesh_request.get("mode", "reference_mesh")).strip() or "reference_mesh"
    return MeshSpec(type=mesh_mode, parameters=mesh_request)


def _default_boundaries(reference_case: ReferenceCase | None) -> list[BoundarySpec]:
    if reference_case and reference_case.case_name == "pitzDaily":
        return [
            BoundarySpec("inlet", "velocity_inlet"),
            BoundarySpec("outlet", "pressure_outlet"),
        ]
    return [
        BoundarySpec("movingWall", "wall"),
        BoundarySpec("fixedWalls", "wall"),
        BoundarySpec("frontAndBack", "empty"),
    ]


def _case_type(plan: SimulationPlan) -> str:
    if plan.physics_domain == "incompressible" and plan.solver_name == "simpleFoam":
        return "steady_incompressible"
    if plan.physics_domain == "incompressible" and plan.solver_name == "icoFoam":
        return "transient_incompressible"
    return plan.physics_domain


def _solver_parameters(plan: SimulationPlan) -> dict[str, Any]:
    parameters: dict[str, Any] = {}
    for key in ["end_time", "delta_t", "write_interval", "kinematic_viscosity", "density"]:
        if key in plan.requested_changes:
            parameters[key] = plan.requested_changes[key]
    return parameters
