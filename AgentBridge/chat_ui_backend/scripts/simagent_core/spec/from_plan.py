from typing import Any

from ..models import ReferenceCase, SimulationPlan
from ..solvers.openfoam.mesh import mesh_request_from_parameters, plan_uses_external_gmsh_mesh
from .simulation_spec import (
    BoundarySpec,
    GeometrySpec,
    MeshSpec,
    NumericsSpec,
    OutputsSpec,
    PhysicsSpec,
    SimulationSpec,
    SolverSpec,
)


TIME_CHANGE_KEYS = ("start_time", "end_time", "delta_t")
NUMERICS_CONTROL_KEYS = ("adjust_time_step", "max_co", "max_delta_t", "run_time_modifiable")
OUTPUT_CONTROL_KEYS = (
    "write_control",
    "write_interval",
    "write_format",
    "write_precision",
    "write_compression",
    "purge_write",
    "time_format",
    "time_precision",
)
PHYSICS_PROPERTY_KEYS = ("kinematic_viscosity", "density")
SOLVER_PARAMETER_KEYS = ("end_time", "delta_t", "write_interval", "kinematic_viscosity", "density")


def simulation_spec_from_openfoam_plan(plan: SimulationPlan) -> SimulationSpec:
    mesh_request = mesh_request_from_parameters(plan.parameters)
    geometry = _geometry_from_plan(plan, mesh_request)
    mesh = _mesh_from_plan(mesh_request)
    return SimulationSpec(
        geometry=geometry,
        mesh=mesh,
        boundaries=_default_boundaries(plan.reference_case, plan.requested_changes),
        solver=SolverSpec(
            name=plan.solver_name,
            case_type=_case_type(plan),
            parameters=_solver_parameters(plan),
        ),
        generation_mode=plan.generation_mode,
        physics=_physics_from_plan(plan),
        numerics=_numerics_from_plan(plan),
        outputs=_outputs_from_plan(plan),
    )


def _physics_from_plan(plan: SimulationPlan) -> PhysicsSpec:
    changes = _requested_changes(plan)
    domain = plan.physics_domain.strip() or "incompressible"
    model = "newtonian" if domain == "incompressible" else ""
    return PhysicsSpec(
        domain=domain,
        model=model,
        properties=_pick_changes(changes, PHYSICS_PROPERTY_KEYS),
    )


def _numerics_from_plan(plan: SimulationPlan) -> NumericsSpec:
    changes = _requested_changes(plan)
    return NumericsSpec(
        algorithm=_algorithm_for_solver(plan.solver_name),
        time=_pick_changes(changes, TIME_CHANGE_KEYS),
        controls=_pick_changes(changes, NUMERICS_CONTROL_KEYS),
    )


def _outputs_from_plan(plan: SimulationPlan) -> OutputsSpec:
    changes = _requested_changes(plan)
    return OutputsSpec(
        format="openfoam",
        result_format="vtk",
        fields=_fields_from_plan(plan),
        controls=_pick_changes(changes, OUTPUT_CONTROL_KEYS),
    )


def _requested_changes(plan: SimulationPlan) -> dict[str, Any]:
    return plan.requested_changes if isinstance(plan.requested_changes, dict) else {}


def _pick_changes(changes: dict[str, Any], keys: tuple[str, ...]) -> dict[str, Any]:
    return {key: changes[key] for key in keys if key in changes}


def _algorithm_for_solver(solver_name: str) -> str:
    if solver_name == "icoFoam":
        return "PISO"
    if solver_name == "simpleFoam":
        return "SIMPLE"
    if solver_name == "pimpleFoam":
        return "PIMPLE"
    return ""


def _fields_from_plan(plan: SimulationPlan) -> list[str]:
    fields = [
        item.path.rsplit("/", 1)[-1]
        for item in plan.planned_files
        if item.path.startswith("case/0/") and item.path.rsplit("/", 1)[-1]
    ]
    return sorted(set(fields)) or ["U", "p"]


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


def _default_boundaries(
    reference_case: ReferenceCase | None,
    changes: dict[str, Any] | None = None,
) -> list[BoundarySpec]:
    requested_changes = changes if isinstance(changes, dict) else {}
    if reference_case and reference_case.case_name.lower() == "pitzdaily":
        return [
            BoundarySpec(
                "inlet",
                "velocity_inlet",
                _boundary_value("inlet_velocity", "velocity", requested_changes),
            ),
            BoundarySpec(
                "outlet",
                "pressure_outlet",
                _boundary_value("outlet_pressure", "pressure", requested_changes),
            ),
        ]
    return [
        BoundarySpec(
            "movingWall",
            "wall",
            _boundary_value("lid_velocity", "velocity", requested_changes),
        ),
        BoundarySpec("fixedWalls", "wall"),
        BoundarySpec("frontAndBack", "empty"),
    ]


def _boundary_value(change_key: str, value_key: str, changes: dict[str, Any]) -> dict[str, Any]:
    if change_key not in changes:
        return {}
    return {value_key: changes[change_key]}


def _case_type(plan: SimulationPlan) -> str:
    if plan.physics_domain == "incompressible" and plan.solver_name == "simpleFoam":
        return "steady_incompressible"
    if plan.physics_domain == "incompressible" and plan.solver_name == "icoFoam":
        return "transient_incompressible"
    return plan.physics_domain


def _solver_parameters(plan: SimulationPlan) -> dict[str, Any]:
    return _pick_changes(_requested_changes(plan), SOLVER_PARAMETER_KEYS)
