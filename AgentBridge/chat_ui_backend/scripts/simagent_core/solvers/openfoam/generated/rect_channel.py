import re
from typing import Any

from ....models import PlannedFile, SimulationPlan
from ....router import parse_case_intent, select_generation_route
from ....spec import (
    BoundarySpec,
    GeometrySpec,
    MeshSpec,
    NumericsSpec,
    OutputsSpec,
    PhysicsSpec,
    SimulationSpec,
    SolverSpec,
)
from ..capabilities import summarize_capabilities
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
DEFAULT_LENGTH = 5.0
DEFAULT_HEIGHT = 1.0
DEFAULT_DEPTH = 0.1
DEFAULT_INLET_VELOCITY = 1.0
DEFAULT_OUTLET_PRESSURE = 0.0
DEFAULT_END_TIME = 1.0
DEFAULT_DELTA_T = 0.005
DEFAULT_NU = 0.01
DEFAULT_WRITE_INTERVAL = 20
DEFAULT_MESH_DENSITY = 20
DEFAULT_MAX_CO = 0.5

LENGTH_KEYS = ["length", "channel length", "L", "\u957f\u5ea6", "\u957f"]
HEIGHT_KEYS = ["height", "channel height", "H", "\u9ad8\u5ea6", "\u9ad8"]
WIDTH_KEYS = ["width", "channel width", "W", "\u5bbd\u5ea6", "\u5bbd"]
DEPTH_KEYS = ["depth", "\u6df1\u5ea6"]
THICKNESS_KEYS = ["thickness", "\u539a\u5ea6", "\u539a"]

_NUMBER_PATTERN = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


class RectChannelGeometryError(ValueError):
    def __init__(
        self,
        dimension: str,
        value: float,
        *,
        provided_as: str = "",
    ) -> None:
        self.dimension = dimension
        self.value = value
        self.provided_as = provided_as
        self.code = f"geometry.non_positive_{dimension}"
        field_label = dimension
        if provided_as and provided_as != dimension:
            field_label = f"{dimension} (provided as {provided_as})"
        super().__init__(
            f"Rectangular channel {field_label} must be positive; received {value:g}."
        )


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
        "capabilities": summarize_capabilities(
            None,
            requested_changes,
            generation_mode="generated_case",
            geometry_type="rect_channel",
        ),
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
    validate_rect_channel_geometry(text)
    length = _extract_positive_float(text, LENGTH_KEYS, DEFAULT_LENGTH)
    height = _extract_positive_float(
        text,
        HEIGHT_KEYS,
        _extract_positive_float(text, WIDTH_KEYS, DEFAULT_HEIGHT),
    )
    depth = _extract_positive_float(text, DEPTH_KEYS + THICKNESS_KEYS, DEFAULT_DEPTH)
    velocity = _extract_positive_float(
        text,
        ["inlet velocity", "velocity", "U", "\u5165\u53e3\u901f\u5ea6", "\u6d41\u901f", "\u901f\u5ea6"],
        DEFAULT_INLET_VELOCITY,
    )
    pressure = _extract_float(
        text,
        ["pressure", "outlet pressure", "\u51fa\u53e3\u538b\u529b"],
        DEFAULT_OUTLET_PRESSURE,
    )
    end_time = _extract_positive_float(
        text,
        ["endTime", "end_time", "end time", "final_time", "final time", "\u8fd0\u884c\u5230", "\u7ed3\u675f\u65f6\u95f4"],
        DEFAULT_END_TIME,
    )
    delta_t = _extract_positive_float(
        text,
        ["deltaT", "delta_t", "time step", "timestep", "\u65f6\u95f4\u6b65", "\u65f6\u95f4\u6b65\u957f"],
        DEFAULT_DELTA_T,
    )
    nu = _extract_positive_float(
        text,
        ["nu", "kinematic viscosity", "viscosity", "\u8fd0\u52a8\u7c98\u5ea6", "\u8fd0\u52a8\u9ecf\u5ea6", "\u7c98\u5ea6", "\u9ecf\u5ea6"],
        DEFAULT_NU,
    )

    mesh_density = _extract_positive_float(
        text,
        [
            "mesh density",
            "mesh_density",
            "cells per meter",
            "cells/m",
            "resolution",
            "\u7f51\u683c\u5bc6\u5ea6",
            "\u6bcf\u7c73\u7f51\u683c",
            "\u6bcf\u7c73\u5355\u5143",
        ],
        DEFAULT_MESH_DENSITY,
    )
    nx, ny = _mesh_cells_from_text(text, length, height, mesh_density)

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
                "mesh_density_per_meter": mesh_density,
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
                "write_interval": DEFAULT_WRITE_INTERVAL,
                "nu": nu,
            },
        ),
        generation_mode="generated_case",
        physics=PhysicsSpec(
            domain="incompressible",
            model="newtonian",
            properties={
                "kinematic_viscosity": nu,
            },
        ),
        numerics=NumericsSpec(
            algorithm="PISO",
            time={
                "start_time": 0.0,
                "end_time": end_time,
                "delta_t": delta_t,
            },
            schemes={
                "ddt": "Euler",
                "div_phi_U": "Gauss linear",
                "laplacian": "Gauss linear orthogonal",
            },
            linear_solvers={
                "p": {"solver": "PCG", "tolerance": 1e-6, "relTol": 0.05},
                "U": {"solver": "smoothSolver", "tolerance": 1e-5, "relTol": 0},
            },
            controls={
                "adjust_time_step": "no",
                "max_co": DEFAULT_MAX_CO,
                "max_delta_t": delta_t,
            },
        ),
        outputs=OutputsSpec(
            format="openfoam",
            result_format="vtk",
            fields=["U", "p"],
            controls={
                "write_control": "timeStep",
                "write_interval": DEFAULT_WRITE_INTERVAL,
            },
        ),
    )


def validate_rect_channel_geometry(user_requirement: str) -> None:
    text = str(user_requirement or "")
    dimensions = [
        ("length", "length", LENGTH_KEYS),
        ("height", "height", HEIGHT_KEYS),
        ("height", "width", WIDTH_KEYS),
        ("depth", "depth", DEPTH_KEYS),
        ("depth", "thickness", THICKNESS_KEYS),
    ]
    for dimension, provided_as, keys in dimensions:
        value = _find_explicit_float(text, keys)
        if value is not None and value <= 0:
            raise RectChannelGeometryError(
                dimension,
                value,
                provided_as=provided_as,
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
    end_time = _time_value(spec, "end_time", DEFAULT_END_TIME)
    delta_t = _time_value(spec, "delta_t", DEFAULT_DELTA_T)
    write_interval = _write_interval(spec)
    max_co = _float_value(spec.numerics.controls, "max_co", DEFAULT_MAX_CO)
    max_delta_t = _float_value(spec.numerics.controls, "max_delta_t", delta_t)
    return _openfoam_header("dictionary", "controlDict") + f"""
application     {spec.solver.name};

startFrom       startTime;
startTime       0;
stopAt          endTime;
endTime         {end_time:g};
deltaT          {delta_t:g};
adjustTimeStep  no;
maxCo           {max_co:g};
maxDeltaT       {max_delta_t:g};

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

fluxRequired
{
    default         no;
    p;
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
    nu = _kinematic_viscosity(spec)
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
    velocity = _velocity(spec)
    return {
        "end_time": _time_value(spec, "end_time", DEFAULT_END_TIME),
        "delta_t": _time_value(spec, "delta_t", DEFAULT_DELTA_T),
        "write_interval": _write_interval(spec),
        "kinematic_viscosity": _kinematic_viscosity(spec),
        "inlet_velocity": f"({velocity[0]:g} {velocity[1]:g} {velocity[2]:g})",
        "outlet_pressure": _outlet_pressure(spec),
    }


def _time_value(spec: SimulationSpec, key: str, default: float) -> float:
    return _float_value(spec.numerics.time, key, _float_value(spec.solver.parameters, key, default))


def _write_interval(spec: SimulationSpec) -> int:
    fallback = spec.solver.parameters.get("write_interval", DEFAULT_WRITE_INTERVAL)
    return _positive_int(spec.outputs.controls.get("write_interval", fallback), DEFAULT_WRITE_INTERVAL)


def _kinematic_viscosity(spec: SimulationSpec) -> float:
    fallback = _float_value(spec.solver.parameters, "nu", DEFAULT_NU)
    return _float_value(spec.physics.properties, "kinematic_viscosity", fallback)


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


def _find_explicit_float(text: str, keys: list[str]) -> float | None:
    for key in keys:
        key_pattern = _key_pattern(key)
        patterns = [
            rf"{key_pattern}\s*(?:=|:|\uff1a|\u4e3a|\u662f|to)?\s*({_NUMBER_PATTERN})",
            rf"({_NUMBER_PATTERN})\s*(?:m/s|m\^2/s|m2/s|m|s|cells/m|\u7c73/\u79d2|\u7c73|\u79d2)?\s*(?:\u7684)?\s*{key_pattern}",
        ]
        for pattern in patterns:
            match = re.search(pattern, text, flags=re.IGNORECASE)
            if match:
                return float(match.group(1))
    return None


def _extract_float(text: str, keys: list[str], default: float) -> float:
    value = _find_explicit_float(text, keys)
    return default if value is None else value


def _extract_positive_float(text: str, keys: list[str], default: float) -> float:
    value = _extract_float(text, keys, default)
    return value if value > 0 else default


def _mesh_cells_from_text(text: str, length: float, height: float, mesh_density: float) -> tuple[int, int]:
    nx = _extract_positive_int(text, ["nx", "x cells", "length cells", "\u957f\u5ea6\u5355\u5143\u6570", "x\u65b9\u5411\u7f51\u683c"], 0)
    ny = _extract_positive_int(text, ["ny", "y cells", "height cells", "width cells", "\u9ad8\u5ea6\u5355\u5143\u6570", "\u5bbd\u5ea6\u5355\u5143\u6570", "y\u65b9\u5411\u7f51\u683c"], 0)

    mesh_pair = _extract_mesh_cell_pair(text)
    if mesh_pair:
        nx = nx or mesh_pair[0]
        ny = ny or mesh_pair[1]

    return (
        nx or max(1, int(round(length * mesh_density))),
        ny or max(1, int(round(height * mesh_density))),
    )


def _extract_mesh_cell_pair(text: str) -> tuple[int, int] | None:
    if not re.search(r"mesh|cell|\u7f51\u683c|\u5355\u5143", text, flags=re.IGNORECASE):
        return None
    match = re.search(rf"({_NUMBER_PATTERN})\s*(?:x|\*|\u00d7)\s*({_NUMBER_PATTERN})", text, flags=re.IGNORECASE)
    if not match:
        return None
    nx = _positive_int(match.group(1), 0)
    ny = _positive_int(match.group(2), 0)
    return (nx, ny) if nx and ny else None


def _extract_positive_int(text: str, keys: list[str], default: int) -> int:
    value = _extract_positive_float(text, keys, float(default))
    return max(0, int(round(value)))


def _key_pattern(key: str) -> str:
    escaped = re.escape(key).replace(r"\ ", r"\s+")
    if key.isascii() and re.match(r"^[A-Za-z_][A-Za-z0-9_]*(?:\s+[A-Za-z_][A-Za-z0-9_]*)*$", key):
        return rf"(?<![A-Za-z0-9_]){escaped}(?![A-Za-z0-9_])"
    return escaped


def _float_value(data: dict[str, Any], key: str, default: float) -> float:
    try:
        return float(data.get(key, default))
    except (TypeError, ValueError):
        return default


def _positive_int(value: Any, default: int) -> int:
    try:
        parsed = int(float(value))
    except (TypeError, ValueError):
        return default
    return parsed if parsed > 0 else default
