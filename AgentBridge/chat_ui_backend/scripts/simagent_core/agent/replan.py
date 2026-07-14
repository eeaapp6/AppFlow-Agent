from copy import deepcopy
import re

from ..gates.spec_gate import review_simulation_spec
from ..models import SimulationPlan
from ..solvers.openfoam.capabilities import generation_mode_for, split_supported_changes
from ..spec import SimulationSpec
from ..spec.from_plan import simulation_spec_from_openfoam_plan
from ..state.task_store import TaskStore


REPLAN_KEY_ALIASES = {
    "end_time": "end_time",
    "final_time": "end_time",
    "endtime": "end_time",
    "delta_t": "delta_t",
    "deltat": "delta_t",
    "time_step": "delta_t",
    "timestep": "delta_t",
    "write_interval": "write_interval",
    "writeinterval": "write_interval",
    "nu": "kinematic_viscosity",
    "viscosity": "kinematic_viscosity",
    "kinematic_viscosity": "kinematic_viscosity",
}


def apply_replan(task, message: str, task_store: TaskStore) -> tuple[dict, dict]:
    if not task.plan:
        raise ValueError("Task has no plan. Run /foam/plan before /foam/replan.")

    plan = SimulationPlan.from_dict(task.plan)
    detected_changes, unrecognized_changes = parse_replan_changes(message)
    geometry_type = _generated_geometry_type(plan)
    supported_changes, unsupported_changes = split_supported_changes(
        plan.reference_case,
        detected_changes,
        generation_mode=plan.generation_mode,
        geometry_type=geometry_type,
    )
    if not supported_changes:
        return plan.to_dict(), {
            "status": "no_supported_changes",
            "applied_changes": {},
            "unrecognized_changes": unrecognized_changes or {"message": message},
        }

    invalid_change = _invalid_positive_change(supported_changes)
    if invalid_change:
        return plan.to_dict(), {
            "status": "invalid_changes",
            "error": invalid_change,
            "applied_changes": {},
            "unrecognized_changes": unrecognized_changes,
        }

    generated_spec = None
    if plan.generation_mode == "generated_case":
        try:
            generated_spec = _updated_generated_rect_channel_spec(
                plan,
                geometry_type,
                supported_changes,
            )
        except ValueError as exc:
            return plan.to_dict(), {
                "status": "invalid_simulation_spec",
                "error": str(exc),
                "applied_changes": {},
                "unrecognized_changes": unrecognized_changes,
            }

    requested_changes = dict(plan.requested_changes)
    requested_changes.update(supported_changes)
    plan.requested_changes = requested_changes

    parameters = dict(plan.parameters)
    parameter_changes = parameters.get("requested_changes", {})
    if not isinstance(parameter_changes, dict):
        parameter_changes = {}
    parameter_changes.update(supported_changes)
    parameters["requested_changes"] = parameter_changes
    if unsupported_changes:
        existing_unsupported = parameters.get("unsupported_changes", {})
        if not isinstance(existing_unsupported, dict):
            existing_unsupported = {}
        existing_unsupported.update(unsupported_changes)
        parameters["unsupported_changes"] = existing_unsupported
    plan.parameters = parameters
    if plan.generation_mode != "generated_case":
        plan.generation_mode = generation_mode_for(plan.reference_case, requested_changes)

    spec_gate_review = _refresh_simulation_spec(plan, generated_spec)
    task_store.invalidate_after_replan(task, plan, spec_gate_review)
    return plan.to_dict(), {
        "status": "updated",
        "applied_changes": supported_changes,
        "unrecognized_changes": unrecognized_changes,
    }


def _refresh_simulation_spec(
    plan: SimulationPlan,
    generated_spec: SimulationSpec | None = None,
) -> dict:
    spec = generated_spec or simulation_spec_from_openfoam_plan(plan)
    parameters = dict(plan.parameters)
    parameters["simulation_spec"] = spec.to_dict()
    plan.parameters = parameters
    return review_simulation_spec(spec).to_dict()


def _generated_geometry_type(plan: SimulationPlan) -> str:
    if plan.generation_mode != "generated_case":
        return ""
    route = plan.parameters.get("route_decision", {})
    if isinstance(route, dict):
        geometry_type = str(route.get("geometry_type", "")).strip()
        if geometry_type:
            return geometry_type
    spec_data = plan.parameters.get("simulation_spec", {})
    if isinstance(spec_data, dict):
        geometry = spec_data.get("geometry", {})
        if isinstance(geometry, dict):
            geometry_type = str(geometry.get("type", "")).strip()
            if geometry_type:
                if plan.case_name == "rect_channel" and geometry_type != "rect_channel":
                    return "rect_channel"
                return geometry_type
    return "rect_channel" if plan.case_name == "rect_channel" else ""


def _invalid_positive_change(changes: dict) -> str:
    for key in ("end_time", "delta_t", "write_interval", "kinematic_viscosity"):
        if key not in changes:
            continue
        try:
            value = float(changes[key])
        except (TypeError, ValueError):
            return f"Replan parameter {key} must be numeric."
        if value <= 0:
            return f"Replan parameter {key} must be positive."
    return ""


def _updated_generated_rect_channel_spec(
    plan: SimulationPlan,
    geometry_type: str,
    changes: dict,
) -> SimulationSpec:
    spec_data = plan.parameters.get("simulation_spec")
    if not isinstance(spec_data, dict):
        raise ValueError("Generated case simulation_spec must be an object.")
    geometry = spec_data.get("geometry")
    if not isinstance(geometry, dict) or str(geometry.get("type", "")).strip() != "rect_channel":
        raise ValueError("Generated rect_channel Replan requires simulation_spec.geometry.type=rect_channel.")
    if geometry_type != "rect_channel":
        raise ValueError("Generated rect_channel Replan requires the authoritative geometry_type rect_channel.")

    spec = SimulationSpec.from_dict(deepcopy(spec_data))
    if spec.mesh.type != "blockMesh":
        raise ValueError("Generated rect_channel Replan requires simulation_spec.mesh.type=blockMesh.")
    if spec.solver.name != "icoFoam":
        raise ValueError("Generated rect_channel Replan requires simulation_spec.solver.name=icoFoam.")
    required_boundaries = {"inlet", "outlet", "walls", "frontAndBack"}
    if not required_boundaries.issubset({item.name for item in spec.boundaries}):
        raise ValueError("Generated rect_channel Replan requires the canonical boundary set.")

    if "end_time" in changes:
        value = float(changes["end_time"])
        spec.numerics.time["end_time"] = value
        spec.solver.parameters["end_time"] = value
    if "delta_t" in changes:
        value = float(changes["delta_t"])
        spec.numerics.time["delta_t"] = value
        spec.numerics.controls["max_delta_t"] = value
        spec.solver.parameters["delta_t"] = value
    if "write_interval" in changes:
        value = int(float(changes["write_interval"]))
        spec.outputs.controls["write_interval"] = value
        spec.solver.parameters["write_interval"] = value
    if "kinematic_viscosity" in changes:
        value = float(changes["kinematic_viscosity"])
        spec.physics.properties["kinematic_viscosity"] = value
        spec.solver.parameters["nu"] = value
        if "kinematic_viscosity" in spec.solver.parameters:
            spec.solver.parameters["kinematic_viscosity"] = value
    spec.generation_mode = "generated_case"

    spec_review = review_simulation_spec(spec)
    if spec_review.status == "failed":
        messages = "; ".join(issue.message for issue in spec_review.issues)
        raise ValueError(f"Updated generated simulation_spec is invalid: {messages}")
    return spec


def parse_replan_changes(message: str) -> tuple[dict, dict]:
    changes: dict[str, str] = {}
    unrecognized: dict[str, str] = {}
    normalized = message.strip()
    for raw_key, value in _iter_replan_assignments(normalized):
        canonical_key = REPLAN_KEY_ALIASES.get(_normalize_replan_key(raw_key))
        if canonical_key:
            changes[canonical_key] = value
        else:
            unrecognized[raw_key] = value
    if not changes and not unrecognized:
        unrecognized["message"] = message
    return changes, unrecognized


def _iter_replan_assignments(message: str) -> list[tuple[str, str]]:
    patterns = [
        r"(?P<key>[A-Za-z_][A-Za-z0-9_]*)\s*(?:=|to|as|改成|修改为|设置为|set to)\s*(?P<value>[-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)",
        r"(?:把|将|修改|设置|set)\s*(?P<key>[A-Za-z_][A-Za-z0-9_]*)\s*(?:=|to|as|成|改成|修改为|设置为)?\s*(?P<value>[-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)",
    ]
    matches: list[tuple[str, str]] = []
    for pattern in patterns:
        for match in re.finditer(pattern, message, flags=re.IGNORECASE):
            key = match.group("key").strip()
            value = match.group("value").strip()
            matches.append((key, value))
    return matches


def _normalize_replan_key(key: str) -> str:
    return re.sub(r"[^a-z0-9_]", "", key.strip().lower())
