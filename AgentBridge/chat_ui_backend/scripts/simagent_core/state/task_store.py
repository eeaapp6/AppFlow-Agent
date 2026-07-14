from pathlib import Path
from copy import deepcopy
import json

from ..config import Config
from ..models import SimulationPlan, TaskContext
from ..repair import repair_action_for_gate_review
from ..utils import atomic_write_json, make_task_id, now_iso


REPLAN_INVALIDATED_GATES = frozenset({
    "generation",
    "static_validation",
    "validation",
    "physics_sanity",
    "execution",
    "result_review",
    "manifest",
})


class TaskStore:
    def __init__(self, config: Config | None = None):
        self._config = config or Config.from_environment()

    def create_task(
        self,
        user_requirement: str,
        output_root: str,
        solver_family: str,
        host_output_root: str = "",
    ) -> TaskContext:
        root = Path(output_root or self._config.default_output_root).expanduser().resolve()
        task_id = make_task_id()
        task_dir = root / task_id
        now = now_iso()

        task = TaskContext(
            version=1,
            task_id=task_id,
            status="context_created",
            user_requirement=user_requirement,
            solver_family=solver_family,
            output_root=str(root),
            host_output_root=host_output_root,
            task_dir=str(task_dir),
            case_dir=str(task_dir / "case"),
            mesh_dir=str(task_dir / "mesh"),
            results_dir=str(task_dir / "results"),
            logs_dir=str(task_dir / "logs"),
            context_path=str(task_dir / "task_context.json"),
            manifest_path=str(task_dir / "agent_manifest_v1.json"),
            reference_files_path=str(task_dir / "reference_files.json"),
            created_at=now,
            updated_at=now,
        )

        for path in [task.case_dir, task.mesh_dir, task.results_dir, task.logs_dir]:
            Path(path).mkdir(parents=True, exist_ok=True)

        self.save_task(task)
        return task

    def save_task(self, task: TaskContext) -> None:
        task.updated_at = now_iso()
        atomic_write_json(Path(task.context_path), task.to_dict())

    def load_task(self, task_dir: str) -> TaskContext:
        context_path = Path(task_dir).expanduser().resolve() / "task_context.json"
        if not context_path.exists():
            raise FileNotFoundError(f"task_context.json not found: {context_path}")
        return TaskContext.from_dict(json.loads(context_path.read_text(encoding="utf-8")))

    def update_status(self, task: TaskContext, status: str, error_message: str = "") -> TaskContext:
        task.status = status
        task.error_message = error_message
        self.save_task(task)
        return task

    def attach_plan(self, task: TaskContext, plan: SimulationPlan) -> TaskContext:
        task.status = "planned"
        task.plan = plan.to_dict()
        task.solver_family = plan.solver_family
        self.attach_gate_reviews(task, _gate_reviews_from_plan(plan), save=False)
        self.save_task(task)
        return task

    def invalidate_after_replan(
        self,
        task: TaskContext,
        plan: SimulationPlan,
        spec_gate_review: dict,
    ) -> TaskContext:
        retained_gate_reviews = _retained_after_replan(task.gate_reviews)
        retained_gate_reviews = _replace_gate_review(retained_gate_reviews, spec_gate_review)
        parameters = dict(plan.parameters)
        parameters["gate_reviews"] = retained_gate_reviews
        plan.parameters = parameters

        task.status = "planned"
        task.plan = plan.to_dict()
        task.solver_family = plan.solver_family
        task.generated_files = []
        task.validation = {}
        task.run = {}
        task.error_message = ""
        task.gate_reviews = retained_gate_reviews
        task.plan["gate_reviews"] = task.gate_reviews
        task.plan["repair_history"] = task.repair_history
        self.save_task(task)
        return task

    def attach_generated_files(self, task: TaskContext, generated_files: list[dict]) -> TaskContext:
        task.status = "generated"
        task.generated_files = generated_files
        task.plan["generated_files"] = generated_files
        self.save_task(task)
        return task

    def attach_reference_files(self, task: TaskContext, reference_files: list[dict]) -> TaskContext:
        task.status = "reference_loaded"
        task.reference_files = reference_files
        task.plan["reference_files"] = reference_files
        task.plan["reference_files_path"] = task.reference_files_path
        task.reference_files_removed = False
        task.plan["reference_files_removed"] = False
        self.save_task(task)
        return task

    def mark_reference_files_removed(self, task: TaskContext, removed: bool) -> TaskContext:
        task.reference_files_removed = removed
        task.plan["reference_files_removed"] = removed
        self.save_task(task)
        return task

    def attach_validation_result(self, task: TaskContext, validation_result: dict) -> TaskContext:
        task.status = validation_result.get("status", "validation_failed")
        task.validation = validation_result
        task.plan["validation"] = validation_result
        _attach_repair_validation_follow_up(task, validation_result)
        task.error_message = ""
        if task.status == "validation_failed":
            missing = validation_result.get("missing_files", [])
            task.error_message = "Missing required files: " + ", ".join(missing)
        self.save_task(task)
        return task

    def attach_run_result(self, task: TaskContext, run_result: dict) -> TaskContext:
        task.status = run_result.get("status", "run_blocked")
        task.run = run_result
        task.plan["run"] = run_result
        _attach_repair_run_follow_up(task, run_result)
        task.error_message = ""
        if task.status == "run_blocked":
            task.error_message = run_result.get("reason", "")
        self.save_task(task)
        return task

    def attach_gate_review(self, task: TaskContext, gate_review: dict, save: bool = True) -> TaskContext:
        return self.attach_gate_reviews(task, [gate_review], save=save)

    def attach_gate_reviews(
        self,
        task: TaskContext,
        gate_reviews: list[dict],
        save: bool = True,
    ) -> TaskContext:
        existing = task.gate_reviews if isinstance(task.gate_reviews, list) else []
        task.gate_reviews = _merge_gate_reviews(existing, gate_reviews)
        task.plan["gate_reviews"] = task.gate_reviews
        if save:
            self.save_task(task)
        return task

    def record_repair_action(self, task: TaskContext, repair_action: dict) -> TaskContext:
        record = _repair_history_record(task, repair_action)
        task.repair_history.append(record)
        task.plan["repair_history"] = task.repair_history
        self.save_task(task)
        return task


def _gate_reviews_from_plan(plan: SimulationPlan) -> list[dict]:
    parameters = plan.parameters if isinstance(plan.parameters, dict) else {}
    gate_reviews = parameters.get("gate_reviews", [])
    return [item for item in gate_reviews if isinstance(item, dict)] if isinstance(gate_reviews, list) else []


def _retained_after_replan(gate_reviews: list[dict]) -> list[dict]:
    return [
        deepcopy(review)
        for review in gate_reviews
        if isinstance(review, dict) and str(review.get("gate", "")).strip() not in REPLAN_INVALIDATED_GATES
    ]


def _replace_gate_review(gate_reviews: list[dict], replacement: dict) -> list[dict]:
    target_gate = str(replacement.get("gate", "")).strip()
    result: list[dict] = []
    replaced = False
    for review in gate_reviews:
        if str(review.get("gate", "")).strip() != target_gate:
            result.append(deepcopy(review))
        elif not replaced:
            result.append(deepcopy(replacement))
            replaced = True
    if not replaced:
        result.append(deepcopy(replacement))
    return result


def _merge_gate_reviews(existing: list[dict], incoming: list[dict]) -> list[dict]:
    merged = [_with_repair_action(item) for item in existing if isinstance(item, dict)]
    for review in incoming:
        if not isinstance(review, dict):
            continue

        incoming_review = _with_repair_action(review)
        incoming_gate = str(incoming_review.get("gate", "")).strip()
        if not incoming_gate:
            merged.append(incoming_review)
            continue

        replaced = False
        for index, existing_review in enumerate(merged):
            if str(existing_review.get("gate", "")).strip() == incoming_gate:
                merged[index] = incoming_review
                replaced = True
                break
        if not replaced:
            merged.append(incoming_review)
    return merged


def _with_repair_action(gate_review: dict) -> dict:
    review = deepcopy(gate_review)
    if review.get("next_repair_action"):
        return review

    action = repair_action_for_gate_review(review)
    if action:
        review["next_repair_action"] = action
    return review


def _repair_history_record(task: TaskContext, repair_action: dict) -> dict:
    action = deepcopy(repair_action) if isinstance(repair_action, dict) else {}
    source_gate = str(action.get("source_gate", "")).strip() or _source_gate_for_action(task, action)
    source_review = _source_review_for_action(task, action, source_gate)
    patch_result = action.get("patch_result", {})
    has_patch_result = isinstance(patch_result, dict) and patch_result
    repair_mode = "executable" if isinstance(action.get("patches"), list) and action.get("patches") else "manual"
    record = {
        "repair_action_id": str(action.get("id", "")).strip(),
        "action_id": str(action.get("id", "")).strip(),
        "label": str(action.get("label", "")).strip(),
        "status": _repair_record_status(repair_mode, patch_result if has_patch_result else {}),
        "repair_mode": repair_mode,
        "source_gate": source_gate,
        "related_diagnostic_codes": _related_diagnostic_codes(action, source_review),
        "related_diagnostic_categories": _related_diagnostic_categories(action, source_review),
        "created_at": now_iso(),
    }
    if has_patch_result:
        record["patch_result"] = patch_result
    return record


def _repair_record_status(repair_mode: str, patch_result: dict) -> str:
    if repair_mode == "manual":
        return "recorded"
    status = str(patch_result.get("status", "")).strip()
    if status in {"applied", "no_changes"}:
        return "applied"
    return "recorded"


def _source_gate_for_action(task: TaskContext, repair_action: dict) -> str:
    action_id = str(repair_action.get("id", "")).strip()
    gate_reviews = task.gate_reviews if isinstance(task.gate_reviews, list) else []
    for review in reversed(gate_reviews):
        if not isinstance(review, dict):
            continue
        action = review.get("next_repair_action", {})
        if isinstance(action, dict) and str(action.get("id", "")).strip() == action_id:
            return str(review.get("gate", "")).strip()
    return ""


def _source_review_for_action(task: TaskContext, repair_action: dict, source_gate: str) -> dict:
    action_id = str(repair_action.get("id", "")).strip()
    gate_reviews = task.gate_reviews if isinstance(task.gate_reviews, list) else []
    for review in reversed(gate_reviews):
        if not isinstance(review, dict):
            continue
        if source_gate and str(review.get("gate", "")).strip() != source_gate:
            continue
        action = review.get("next_repair_action", {})
        if isinstance(action, dict) and str(action.get("id", "")).strip() == action_id:
            return review
    return {}


def _related_diagnostic_codes(action: dict, review: dict) -> list[str]:
    explicit = action.get("related_diagnostic_codes", [])
    if isinstance(explicit, list):
        codes = [str(item).strip() for item in explicit if str(item).strip()]
        if codes:
            return sorted(set(codes))

    issues = review.get("issues", []) if isinstance(review, dict) else []
    return sorted({
        str(item.get("code", "")).strip()
        for item in issues
        if isinstance(item, dict) and str(item.get("code", "")).strip()
    })


def _related_diagnostic_categories(action: dict, review: dict) -> list[str]:
    explicit = action.get("related_diagnostic_categories", [])
    if isinstance(explicit, list):
        categories = [str(item).strip() for item in explicit if str(item).strip()]
        if categories:
            return sorted(set(categories))

    diagnostics = review.get("diagnostics", {}) if isinstance(review, dict) else {}
    categories = diagnostics.get("categories", []) if isinstance(diagnostics, dict) else []
    if isinstance(categories, list) and categories:
        return sorted({str(item).strip() for item in categories if str(item).strip()})

    return sorted({_category_for_code(code) for code in _related_diagnostic_codes(action, review)})


def _category_for_code(code: str) -> str:
    if code.startswith("result.mesh_"):
        return "mesh"
    if code.startswith("result.residual_") or code in {"result.courant_high", "result.divergence"}:
        return "numerics"
    if code.startswith("physics."):
        return "physics"
    if code in {"result.missing_boundary_field", "result.unknown_patch"} or code.startswith("boundary."):
        return "boundary"
    if code.startswith("validation."):
        return "validation"
    if code.startswith("result.missing_latest_"):
        return "output"
    if code.startswith("result.") or code.startswith("execution."):
        return "runtime"
    return "result"


def _attach_repair_run_follow_up(task: TaskContext, run_result: dict) -> None:
    record = _latest_repair_history_record(task)
    if not record:
        return

    follow_up = _repair_run_follow_up(record, run_result)
    record["repair_followup"] = follow_up
    task.plan["repair_history"] = task.repair_history


def _latest_repair_history_record(task: TaskContext) -> dict:
    history = task.repair_history if isinstance(task.repair_history, list) else []
    for record in reversed(history):
        if isinstance(record, dict):
            return record
    return {}


def _repair_run_follow_up(record: dict, run_result: dict) -> dict:
    related_codes = [
        str(item).strip()
        for item in record.get("related_diagnostic_codes", [])
        if str(item).strip()
    ] if isinstance(record.get("related_diagnostic_codes", []), list) else []
    diagnostics = run_result.get("diagnostics", {})
    if not related_codes or "diagnostics" not in run_result or not isinstance(diagnostics, dict):
        return {
            "status": "unknown",
            "resolved_codes": [],
            "unresolved_codes": [],
            "checked_at": now_iso(),
        }

    current_codes = _diagnostic_codes_from_run_result(run_result)
    unresolved = sorted(set(related_codes) & set(current_codes))
    resolved = sorted(set(related_codes) - set(unresolved))
    return {
        "status": "resolved" if not unresolved else "unresolved",
        "resolved_codes": resolved,
        "unresolved_codes": unresolved,
        "checked_at": now_iso(),
    }


def _diagnostic_codes_from_run_result(run_result: dict) -> list[str]:
    diagnostics = run_result.get("diagnostics", {})
    items = diagnostics.get("items", []) if isinstance(diagnostics, dict) else []
    if not isinstance(items, list):
        return []
    return sorted({
        str(item.get("code", "")).strip()
        for item in items
        if isinstance(item, dict) and str(item.get("code", "")).strip()
    })


def _attach_repair_validation_follow_up(task: TaskContext, validation_result: dict) -> None:
    if not isinstance(task.repair_history, list):
        return

    status = str(validation_result.get("status", "")).strip()
    if not status:
        return

    for record in reversed(task.repair_history):
        if not isinstance(record, dict) or record.get("follow_up"):
            continue
        record["follow_up"] = {
            "validation_status": status,
            "checked_at": now_iso(),
        }
        task.plan["repair_history"] = task.repair_history
        return
