from math import sqrt
from typing import Any

from ..models import SimulationPlan, TaskContext
from ..spec import SimulationSpec
from .models import GateResult


CHARACTERISTIC_LENGTH_ASSUMPTION = "Uses channel height as characteristic length."
COURANT_ASSUMPTION = "Uses minimum blockMesh cell size for estimated Courant number."
COURANT_WARNING_THRESHOLD = 1.0
COURANT_HIGH_RISK_THRESHOLD = 5.0
REYNOLDS_ICOFOAM_WARNING_THRESHOLD = 1000.0
REYNOLDS_LOW_WARNING_THRESHOLD = 1e-6
MIN_NX = 5
MIN_NY = 3
MAX_ASPECT_RATIO = 100.0
MIN_ASPECT_RATIO = 1.0


def review_physics_sanity(
    task: TaskContext,
    plan: SimulationPlan,
    run_result: dict[str, Any] | None = None,
) -> GateResult:
    result = GateResult("physics_sanity")
    supported = _is_supported_rect_channel(plan)
    if not supported:
        result.metadata["physics_sanity"] = _metadata(
            supported=False,
            case_name=plan.case_name,
            status=result.status,
            summary="Physics sanity gate is not supported for this case.",
            metrics={},
        )
        return result

    spec = _simulation_spec(plan)
    if spec is None:
        result.add_error(
            "physics.invalid_spec",
            "generated rect_channel plan is missing a parseable SimulationSpec.",
        )
        result.metadata["physics_sanity"] = _metadata(
            supported=True,
            case_name=plan.case_name,
            status=result.status,
            summary="Physics sanity failed because SimulationSpec is missing or invalid.",
            metrics={},
        )
        return result

    values = _extract_values(spec)
    _review_required_values(values, result)
    if result.status != "failed":
        _review_warnings(values, result)

    metrics = _metrics(values) if _values_are_computable(values) else {}
    result.metadata["physics_sanity"] = _metadata(
        supported=True,
        case_name=plan.case_name,
        status=result.status,
        summary=_summary(result),
        metrics=metrics,
    )
    return result


def _is_supported_rect_channel(plan: SimulationPlan) -> bool:
    return plan.generation_mode == "generated_case" and plan.case_name == "rect_channel"


def _simulation_spec(plan: SimulationPlan) -> SimulationSpec | None:
    parameters = plan.parameters if isinstance(plan.parameters, dict) else {}
    spec_data = parameters.get("simulation_spec", {})
    if not isinstance(spec_data, dict) or not spec_data:
        return None
    try:
        return SimulationSpec.from_dict(spec_data)
    except (TypeError, ValueError):
        return None


def _extract_values(spec: SimulationSpec) -> dict[str, Any]:
    geometry = spec.geometry.parameters
    mesh = spec.mesh.parameters
    numerics_time = spec.numerics.time
    cells = mesh.get("cells", [])
    nx, ny, nz = _mesh_cells(cells)
    velocity = _inlet_velocity(spec)
    velocity_magnitude = _velocity_magnitude(velocity)
    length = _float_or_none(geometry.get("length"))
    height = _float_or_none(geometry.get("height"))
    depth = _float_or_none(geometry.get("depth"))
    nu = _float_or_none(spec.physics.properties.get("kinematic_viscosity"))
    delta_t = _float_or_none(numerics_time.get("delta_t", spec.solver.parameters.get("delta_t")))
    return {
        "length": length,
        "height": height,
        "depth": depth,
        "nu": nu,
        "delta_t": delta_t,
        "nx": nx,
        "ny": ny,
        "nz": nz,
        "velocity": velocity,
        "velocity_magnitude": velocity_magnitude,
        "solver_name": spec.solver.name,
    }


def _review_required_values(values: dict[str, Any], result: GateResult) -> None:
    if not _is_positive(values.get("length")) or not _is_positive(values.get("height")) or not _is_positive(values.get("depth")):
        result.add_error("physics.invalid_geometry", "rect_channel length, height, and depth must be positive.")
    if not _is_positive(values.get("nu")):
        result.add_error("physics.invalid_viscosity", "Kinematic viscosity must be positive.")
    if not _is_positive(values.get("delta_t")):
        result.add_error("physics.invalid_time_step", "deltaT must be positive.")
    if not _is_positive_int(values.get("nx")) or not _is_positive_int(values.get("ny")):
        result.add_error("physics.invalid_mesh_cells", "Mesh cell counts nx and ny must be positive.")
    if values.get("velocity_magnitude") is None or _has_negative_inlet_velocity(values.get("velocity")):
        result.add_error("physics.invalid_velocity", "Inlet velocity must be parseable and non-negative for rect_channel.")


def _review_warnings(values: dict[str, Any], result: GateResult) -> None:
    metrics = _metrics(values)
    velocity = metrics["velocity_magnitude"]
    if velocity == 0:
        result.add_warning("physics.zero_velocity", "Inlet velocity is zero; this is a stationary-flow setup.")

    estimated_courant = metrics["estimated_courant"]
    if estimated_courant > COURANT_HIGH_RISK_THRESHOLD:
        result.add_warning(
            "physics.courant_risk",
            f"Estimated Courant number {estimated_courant:.3g} is high; deltaT is likely too large.",
        )
    elif estimated_courant > COURANT_WARNING_THRESHOLD:
        result.add_warning(
            "physics.courant_risk",
            f"Estimated Courant number {estimated_courant:.3g} is above 1.",
        )

    reynolds_number = metrics["reynolds_number"]
    if reynolds_number > REYNOLDS_ICOFOAM_WARNING_THRESHOLD and values.get("solver_name") == "icoFoam":
        result.add_warning(
            "physics.reynolds_solver_risk",
            f"Estimated Reynolds number {reynolds_number:.3g} may be high for icoFoam laminar defaults.",
        )
    if reynolds_number < REYNOLDS_LOW_WARNING_THRESHOLD:
        result.add_warning(
            "physics.reynolds_solver_risk",
            f"Estimated Reynolds number {reynolds_number:.3g} is near zero; parameters may be scaled incorrectly.",
        )

    if int(values["nx"]) < MIN_NX or int(values["ny"]) < MIN_NY:
        result.add_warning("physics.mesh_too_coarse", "Mesh is very coarse for rect_channel sanity checks.")

    aspect_ratio = float(values["length"]) / float(values["height"])
    if aspect_ratio > MAX_ASPECT_RATIO or aspect_ratio < MIN_ASPECT_RATIO:
        result.add_warning(
            "physics.geometry_aspect_ratio_risk",
            f"Channel length/height ratio {aspect_ratio:.3g} is unusual.",
        )


def _metadata(
    *,
    supported: bool,
    case_name: str,
    status: str,
    summary: str,
    metrics: dict[str, Any],
) -> dict[str, Any]:
    return {
        "supported": supported,
        "case_name": case_name,
        "status": status,
        "summary": summary,
        "metrics": metrics,
        "assumptions": [
            CHARACTERISTIC_LENGTH_ASSUMPTION,
            COURANT_ASSUMPTION,
        ] if supported else [],
    }


def _summary(result: GateResult) -> str:
    if result.status == "failed":
        return "Physics sanity failed for generated rect_channel."
    if result.status == "warning":
        return "Physics sanity found potential setup risks for generated rect_channel."
    return "Physics sanity passed for generated rect_channel."


def _metrics(values: dict[str, Any]) -> dict[str, Any]:
    length = float(values["length"])
    height = float(values["height"])
    velocity = float(values["velocity_magnitude"])
    nu = float(values["nu"])
    delta_t = float(values["delta_t"])
    nx = int(values["nx"])
    ny = int(values["ny"])
    dx = length / nx
    dy = height / ny
    min_cell_size = min(dx, dy)
    return {
        "reynolds_number": velocity * height / nu,
        "estimated_courant": velocity * delta_t / min_cell_size,
        "velocity_magnitude": velocity,
        "min_cell_size": min_cell_size,
        "dx": dx,
        "dy": dy,
        "nx": nx,
        "ny": ny,
        "nz": int(values.get("nz") or 0),
    }


def _values_are_computable(values: dict[str, Any]) -> bool:
    required = ["length", "height", "nu", "delta_t"]
    return (
        all(_is_positive(values.get(key)) for key in required)
        and _float_or_none(values.get("velocity_magnitude")) is not None
        and not _has_negative_inlet_velocity(values.get("velocity"))
        and _is_positive_int(values.get("nx"))
        and _is_positive_int(values.get("ny"))
    )


def _mesh_cells(value: Any) -> tuple[int | None, int | None, int | None]:
    if not isinstance(value, list):
        return None, None, None
    padded = [*value, None, None, None]
    return _int_or_none(padded[0]), _int_or_none(padded[1]), _int_or_none(padded[2])


def _inlet_velocity(spec: SimulationSpec) -> list[float] | float | None:
    for boundary in spec.boundaries:
        if boundary.role != "velocity_inlet":
            continue
        value = boundary.value.get("velocity")
        if isinstance(value, list) and len(value) >= 3:
            components = [_float_or_none(item) for item in value[:3]]
            return components if all(item is not None for item in components) else None
        return _float_or_none(value)
    return None


def _velocity_magnitude(value: Any) -> float | None:
    if isinstance(value, list):
        if len(value) < 3 or any(item is None for item in value[:3]):
            return None
        return sqrt(sum(float(item) ** 2 for item in value[:3]))
    parsed = _float_or_none(value)
    if parsed is None:
        return None
    return abs(parsed)


def _has_negative_inlet_velocity(value: Any) -> bool:
    if isinstance(value, list):
        first = _float_or_none(value[0]) if value else None
        return first is not None and first < 0
    parsed = _float_or_none(value)
    return parsed is not None and parsed < 0


def _float_or_none(value: Any) -> float | None:
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def _int_or_none(value: Any) -> int | None:
    try:
        return int(float(value))
    except (TypeError, ValueError):
        return None


def _is_positive(value: Any) -> bool:
    parsed = _float_or_none(value)
    return parsed is not None and parsed > 0


def _is_positive_int(value: Any) -> bool:
    parsed = _int_or_none(value)
    return parsed is not None and parsed > 0
