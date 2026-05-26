import re

from ..models import SimulationPlan
from ..solvers.openfoam.capabilities import generation_mode_for, split_supported_changes
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
    supported_changes, unsupported_changes = split_supported_changes(plan.reference_case, detected_changes)
    if not supported_changes:
        task_store.update_status(task, "planned")
        return plan.to_dict(), {
            "status": "no_supported_changes",
            "applied_changes": {},
            "unrecognized_changes": unrecognized_changes or {"message": message},
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
    plan.generation_mode = generation_mode_for(plan.reference_case, requested_changes)

    task_store.attach_plan(task, plan)
    return plan.to_dict(), {
        "status": "updated",
        "applied_changes": supported_changes,
        "unrecognized_changes": unrecognized_changes,
    }


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
