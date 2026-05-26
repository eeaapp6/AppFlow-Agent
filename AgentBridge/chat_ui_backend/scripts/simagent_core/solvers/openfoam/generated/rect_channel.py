import re
from typing import Any

from ....models import PlannedFile, SimulationPlan
from ....router import parse_case_intent, select_generation_route
from ....spec import BoundarySpec, GeometrySpec, MeshSpec, SimulationSpec, SolverSpec
from ..templates.icofoam_cavity import _openfoam_header


RECT_CHANNEL_FILES = [
    PlannedFile("mesh", "case/system/blockMeshDict", "openfoam-dict"),
    PlannedFile("control", "case/system/controlDict", "openfoam-dict"),
    PlannedFile("numerics", "case/system/fvSchemes", "openfoam-dict"),
    PlannedFile("linear_solver", "case/system/fvSolution", "openfoam-dict"),
    PlannedFile("physical_properties", "case/constant/physicalProperties", "openfoam-dict"),
    PlannedFile("initial_condition", "case/0/U", "openfoam-dict"),
    PlannedFile("initial_condition", "case/0/p", "openfoam-dict"),
]


def is_rect_channel_request(user_requirement: str) -> bool:
    intent = parse_case_intent(user_requirement)
    route = select_generation_route(intent)
    return route.selected_mode == "generated_case" and route.geometry_type == "rect_channel"


def create_rect_channel_plan(user_requirement: str) -> SimulationPlan:
    spec = rect_channel_spec_from_text(user_requirement)
    requested_changes = _requested_changes_from_spec(spec)
    parameters = {
        "simulation_spec": spec.to_dict(),
        "generation_mode": "generated_case",
        "case_template": "generated_rect_channel",
        "requested_changes": requested_changes,
        "unsupported_changes": {},
        "capabilities": {
            "reference_selection": "generated",
            "case_generation": "rect_channel",
            "supported_changes": sorted(requested_changes),
            "unsupported_changes": [],
            "run_pipeline": "basic_blockMesh_solver",
        },
    }
    return SimulationPlan(
        solver_family="openfoam",
        solver_name=spec.solver.name,
        physics_domain="incompressible",
        case_category="generated",
        case_name="rect_channel",
        description=user_requirement or "Generated rectangular channel case.",
        generation_mode="generated_case",
        requested_changes=requested_changes,
        parameters=parameters,
        planned_files=list(RECT_CHANNEL_FILES),
    )


def rect_channel_spec_from_text(user_requirement: str) -> SimulationSpec:
    text = str(user_requirement or "")
    length = _extract_dimension(text, ["length", "\u957f", "\u957f\u5ea6"], default=5.0)
    height = _extract_dimension(text, ["height", "\u9ad8", "\u9ad8\u5ea6"], default=1.0)
    depth = _extract_dimension(text, ["depth", "\u539a", "\u6df1\u5ea6"], default=0.1)
    velocity = _extract_dimension(text, ["velocity", "inlet velocity", "\u5165\u53e3\u901f\u5ea6"], default=1.0)
    pressure = _extract_dimension(text, ["pressure", "outlet pressure", "\u51fa\u53e3\u538b\u529b"], default=0.0)
    end_time = _extract_dimension(text, ["end_time", "final_time", "\u8fd0\u884c\u5230", "\u7ed3\u675f\u65f6\u95f4"], default=1.0)
    delta_t = _extract_dimension(text, ["delta_t", "time step", "\u65f6\u95f4\u6b65"], default=0.005)
    nu = _extract_dimension(text, ["nu", "\u7c98\u5ea6", "\u8fd0\u52a8\u7c98\u5ea6"], default=0.01)

    nx = max(1, int(round(length * 20)))
    ny = max(1, int(round(height * 20)))

    return SimulationSpec(
        geometry=GeometrySpec(
            type="rect_channel",
            parameters={
                "length": length,
                "height": height,
                "depth": depth,
            },
        ),
        mesh=MeshSpec(
            type="blockMesh",
            parameters={
                "cells": [nx, ny, 1],
            },
        ),
        boundaries=[
            BoundarySpec("inlet", "velocity_inlet", {"velocity": [velocity, 0.0, 0.0]}),
            BoundarySpec("outlet", "pressure_outlet", {"pressure": pressure}),
            BoundarySpec("walls", "wall"),
            BoundarySpec("frontAndBack", "empty"),
        ],
        solver=SolverSpec(
            name="icoFoam",
            case_type="transient_incompressible",
            parameters={
                "end_time": end_time,
                "delta_t": delta_t,
                "write_interval": 20,
                "nu": nu,
            },
        ),
        generation_mode="generated_case",
    )


def render_rect_channel_case(spec: SimulationSpec) -> list[tuple[PlannedFile, str]]:
    return [
        (RECT_CHANNEL_FILES[0], _render_block_mesh_dict(spec)),
        (RECT_CHANNEL_FILES[1], _render_control_dict(spec)),
        (RECT_CHANNEL_FILES[2], _render_fv_schemes()),
        (RECT_CHANNEL_FILES[3], _render_fv_solution()),
        (RECT_CHANNEL_FILES[4], _render_physical_properties(spec)),
        (RECT_CHANNEL_FILES[5], _render_u(spec)),
        (RECT_CHANNEL_FILES[6], _render_p(spec)),
    ]


def _render_block_mesh_dict(spec: SimulationSpec) -> str:
    geometry = spec.geometry.parameters
    mesh = spec.mesh.parameters
    length = _float_value(geometry, "length", 5.0)
    height = _float_value(geometry, "height", 1.0)
    depth = _float_value(geometry, "depth", 0.1)
    cells = mesh.get("cells", [100, 20, 1])
    nx, ny, nz = [_positive_int(item, default) for item, default in zip(cells, [100, 20, 1])]

    return _openfoam_header("dictionary", "blockMeshDict") + f"""
convertToMeters 1;

vertices
(
    (0 0 0)
    ({length:g} 0 0)
    ({length:g} {height:g} 0)
    (0 {height:g} 0)
    (0 0 {depth:g})
    ({length:g} 0 {depth:g})
    ({length:g} {height:g} {depth:g})
    (0 {height:g} {depth:g})
);

blocks
(
    hex (0 1 2 3 4 5 6 7) ({nx} {ny} {nz}) simpleGrading (1 1 1)
);

edges
(
);

boundary
(
    inlet
    {{
        type patch;
        faces
        (
            (0 4 7 3)
        );
    }}
    outlet
    {{
        type patch;
        faces
        (
            (1 2 6 5)
        );
    }}
    walls
    {{
        type wall;
        faces
        (
            (0 1 5 4)
            (3 7 6 2)
        );
    }}
    frontAndBack
    {{
        type empty;
        faces
        (
            (0 3 2 1)
            (4 5 6 7)
        );
    }}
);

mergePatchPairs
(
);
"""


def _render_control_dict(spec: SimulationSpec) -> str:
    solver = spec.solver.parameters
    end_time = _float_value(solver, "end_time", 1.0)
    delta_t = _float_value(solver, "delta_t", 0.005)
    write_interval = _positive_int(solver.get("write_interval", 20), 20)
    return _openfoam_header("dictionary", "controlDict") + f"""
application     {spec.solver.name};

startFrom       startTime;
startTime       0;
stopAt          endTime;
endTime         {end_time:g};
deltaT          {delta_t:g};

writeControl    timeStep;
writeInterval   {write_interval};
purgeWrite      0;

writeFormat     ascii;
writePrecision  6;
writeCompression off;

timeFormat      general;
timePrecision   6;

runTimeModifiable true;
"""


def _render_fv_schemes() -> str:
    return _openfoam_header("dictionary", "fvSchemes") + """
ddtSchemes
{
    default         Euler;
}

gradSchemes
{
    default         Gauss linear;
}

divSchemes
{
    default         none;
    div(phi,U)      Gauss linear;
}

laplacianSchemes
{
    default         Gauss linear orthogonal;
}

interpolationSchemes
{
    default         linear;
}

snGradSchemes
{
    default         orthogonal;
}
"""


def _render_fv_solution() -> str:
    return _openfoam_header("dictionary", "fvSolution") + """
solvers
{
    p
    {
        solver          PCG;
        preconditioner  DIC;
        tolerance       1e-06;
        relTol          0.05;
    }

    pFinal
    {
        $p;
        relTol          0;
    }

    U
    {
        solver          smoothSolver;
        smoother        symGaussSeidel;
        tolerance       1e-05;
        relTol          0;
    }
}

PISO
{
    nCorrectors     2;
    nNonOrthogonalCorrectors 0;
    pRefCell        0;
    pRefValue       0;
}
"""


def _render_physical_properties(spec: SimulationSpec) -> str:
    nu = _float_value(spec.solver.parameters, "nu", 0.01)
    return _openfoam_header("dictionary", "physicalProperties") + f"""
nu              [0 2 -1 0 0 0 0] {nu:g};
"""


def _render_u(spec: SimulationSpec) -> str:
    velocity = _velocity(spec)
    return _openfoam_header("volVectorField", "U") + f"""
dimensions      [0 1 -1 0 0 0 0];

internalField   uniform (0 0 0);

boundaryField
{{
    inlet
    {{
        type            fixedValue;
        value           uniform ({velocity[0]:g} {velocity[1]:g} {velocity[2]:g});
    }}

    outlet
    {{
        type            zeroGradient;
    }}

    walls
    {{
        type            noSlip;
    }}

    frontAndBack
    {{
        type            empty;
    }}
}}
"""


def _render_p(spec: SimulationSpec) -> str:
    pressure = _outlet_pressure(spec)
    return _openfoam_header("volScalarField", "p") + f"""
dimensions      [0 2 -2 0 0 0 0];

internalField   uniform 0;

boundaryField
{{
    inlet
    {{
        type            zeroGradient;
    }}

    outlet
    {{
        type            fixedValue;
        value           uniform {pressure:g};
    }}

    walls
    {{
        type            zeroGradient;
    }}

    frontAndBack
    {{
        type            empty;
    }}
}}
"""


def _requested_changes_from_spec(spec: SimulationSpec) -> dict[str, Any]:
    solver = spec.solver.parameters
    velocity = _velocity(spec)
    return {
        "end_time": solver.get("end_time", 1.0),
        "delta_t": solver.get("delta_t", 0.005),
        "write_interval": solver.get("write_interval", 20),
        "kinematic_viscosity": solver.get("nu", 0.01),
        "inlet_velocity": f"({velocity[0]:g} {velocity[1]:g} {velocity[2]:g})",
        "outlet_pressure": _outlet_pressure(spec),
    }


def _velocity(spec: SimulationSpec) -> list[float]:
    for boundary in spec.boundaries:
        if boundary.role == "velocity_inlet":
            value = boundary.value.get("velocity", [1.0, 0.0, 0.0])
            if isinstance(value, list) and len(value) >= 3:
                return [float(value[0]), float(value[1]), float(value[2])]
    return [1.0, 0.0, 0.0]


def _outlet_pressure(spec: SimulationSpec) -> float:
    for boundary in spec.boundaries:
        if boundary.role == "pressure_outlet":
            return float(boundary.value.get("pressure", 0.0))
    return 0.0


def _extract_dimension(text: str, keys: list[str], default: float) -> float:
    for key in keys:
        escaped_key = re.escape(key)
        patterns = [
            rf"{escaped_key}\s*(?:=|:|\uff1a|\u4e3a|\u662f)?\s*([-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)",
            rf"([-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)\s*(?:m/s|m|s)?\s*{escaped_key}",
        ]
        for pattern in patterns:
            match = re.search(pattern, text, flags=re.IGNORECASE)
            if match:
                return float(match.group(1))
    return default


def _float_value(data: dict[str, Any], key: str, default: float) -> float:
    try:
        return float(data.get(key, default))
    except (TypeError, ValueError):
        return default


def _positive_int(value: Any, default: int) -> int:
    try:
        parsed = int(value)
    except (TypeError, ValueError):
        return default
    return parsed if parsed > 0 else default
