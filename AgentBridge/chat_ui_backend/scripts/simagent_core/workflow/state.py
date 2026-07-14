from collections.abc import Callable

from ..models import TaskContext


WORKFLOW_ACTIONS = ("generate", "validate", "run", "replan", "repair_action")
IN_PROGRESS_STATUSES = frozenset({
    "planning",
    "loading_reference",
    "generating_files",
    "validating",
    "running",
})
GENERATED_CASE_STATUSES = frozenset({
    "generated",
    "validation_failed",
    "validated",
    "run_blocked",
    "run_failed",
    "run_completed",
})
VALIDATED_CASE_STATUSES = frozenset({
    "validated",
    "run_blocked",
    "run_failed",
    "run_completed",
})
REQUIRED_PLAN_FIELDS = (
    "solver_family",
    "solver_name",
    "physics_domain",
    "case_category",
    "case_name",
)


def _has_valid_plan(task: TaskContext) -> bool:
    plan = task.plan if isinstance(task.plan, dict) else {}
    return bool(plan) and all(str(plan.get(field, "")).strip() for field in REQUIRED_PLAN_FIELDS)


def _has_generated_case(task: TaskContext) -> bool:
    return task.status in GENERATED_CASE_STATUSES and bool(task.generated_files)


def _has_validated_case(task: TaskContext) -> bool:
    validation = task.validation if isinstance(task.validation, dict) else {}
    return (
        task.status in VALIDATED_CASE_STATUSES
        and _has_generated_case(task)
        and str(validation.get("status", "")).strip() == "validated"
    )


ACTION_RULES: dict[str, Callable[[TaskContext], bool]] = {
    "generate": _has_valid_plan,
    "validate": lambda task: _has_valid_plan(task) and _has_generated_case(task),
    "run": lambda task: _has_valid_plan(task) and _has_validated_case(task),
    "replan": _has_valid_plan,
    "repair_action": lambda task: True,
}


def allowed_workflow_actions(task: TaskContext) -> list[str]:
    if task.status in IN_PROGRESS_STATUSES:
        return []
    return [action for action in WORKFLOW_ACTIONS if ACTION_RULES[action](task)]


def workflow_state_error(task: TaskContext, requested_action: str) -> dict:
    allowed_actions = allowed_workflow_actions(task)
    if requested_action in allowed_actions:
        return {}

    current_status = str(task.status).strip()
    return {
        "error": (
            f"Action '{requested_action}' is not allowed while task status is "
            f"'{current_status or 'unknown'}'."
        ),
        "error_code": "invalid_workflow_state",
        "current_status": current_status,
        "requested_action": requested_action,
        "allowed_actions": allowed_actions,
    }
