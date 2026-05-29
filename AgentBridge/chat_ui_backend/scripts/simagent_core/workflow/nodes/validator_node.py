from ...manifest.writer import ManifestWriter
from ...gates.models import GateIssue, GateResult
from ...models import SimulationPlan, TaskContext
from ...state.task_store import TaskStore
from ...services.validate import validate_case


def validator_node(
    task: TaskContext,
    task_store: TaskStore | None = None,
    manifest_writer: ManifestWriter | None = None,
) -> dict:
    if not task.plan:
        raise ValueError("Task has no plan. Run planner before validation.")

    store = task_store or TaskStore()
    writer = manifest_writer or ManifestWriter()
    plan = SimulationPlan.from_dict(task.plan)

    store.update_status(task, "validating")
    validation_result = validate_case(task, plan)
    store.attach_gate_review(task, _static_validation_gate_review(validation_result), save=False)
    store.attach_validation_result(task, validation_result)
    writer.write_validation_result(task, plan, validation_result)
    return validation_result


def _static_validation_gate_review(validation_result: dict) -> dict:
    result = GateResult("static_validation")
    validation_warnings = validation_result.get("boundary_condition_warnings", [])
    if str(validation_result.get("status", "")).strip() == "validated":
        _append_boundary_condition_warnings(result, validation_warnings)
        return result.to_dict()

    result.status = "failed"
    for item in validation_result.get("missing_files", [])[:10]:
        result.issues.append(GateIssue("validation.missing_file", f"Missing required file: {item}."))
    for field_name, patches in validation_result.get("missing_boundary_fields", {}).items():
        result.issues.append(
            GateIssue("validation.missing_boundary_field", f"{field_name} missing patches: {', '.join(patches)}.")
        )
    for field_name, patches in validation_result.get("extra_boundary_fields", {}).items():
        result.issues.append(
            GateIssue("validation.extra_boundary_field", f"{field_name} has extra patches: {', '.join(patches)}.")
        )
    for field_name in validation_result.get("empty_boundary_fields", [])[:10]:
        result.issues.append(
            GateIssue("validation.empty_boundary_field", f"{field_name} boundaryField is empty or missing.")
        )
    for item in validation_result.get("boundary_condition_errors", [])[:10]:
        result.issues.append(
            GateIssue(
                item.get("code", "validation.boundary_condition"),
                f"{item.get('file', '')}: {item.get('message', '')}",
            )
        )
    _append_boundary_condition_warnings(result, validation_warnings)
    for item in validation_result.get("dictionary_errors", [])[:10]:
        result.issues.append(
            GateIssue(
                "validation.dictionary_error",
                f"{item.get('file', '')}: {item.get('message', '')}",
            )
        )
    if not result.issues:
        result.issues.append(GateIssue("validation.failed", "Case validation failed."))
    result.metadata["diagnostics"] = _validation_diagnostics(validation_result)
    return result.to_dict()


def _append_boundary_condition_warnings(result: GateResult, warnings: list) -> None:
    for item in warnings[:10]:
        result.add_warning(
            item.get("code", "validation.boundary_condition_warning"),
            f"{item.get('file', '')}: {item.get('message', '')}",
        )


def _validation_diagnostics(validation_result: dict) -> dict:
    return {
        "validation": {
            "mesh_patch_types": validation_result.get("mesh_patch_types", {}),
            "missing_boundary_fields": validation_result.get("missing_boundary_fields", {}),
            "boundary_field_types": validation_result.get("boundary_field_types", {}),
        }
    }
