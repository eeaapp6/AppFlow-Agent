from ...manifest.validator import validate_appflow_manifest
from ...manifest.writer import ManifestWriter
from ...gates.execution_gate import review_run_pipeline
from ...gates.models import GateIssue, GateResult
from ...gates.result_review_gate import review_run_results
from ...models import SimulationPlan, TaskContext
from ...state.task_store import TaskStore
from ...services.run_local import run_case


def local_runner_node(
    task: TaskContext,
    task_store: TaskStore | None = None,
    manifest_writer: ManifestWriter | None = None,
) -> dict:
    if not task.plan:
        raise ValueError("Task has no plan. Run planner before run readiness check.")

    store = task_store or TaskStore()
    writer = manifest_writer or ManifestWriter()
    plan = SimulationPlan.from_dict(task.plan)

    store.update_status(task, "running")
    run_result = run_case(task, plan)
    store.attach_gate_reviews(
        task,
        [
            _execution_gate_review(run_result),
            review_run_results(task, run_result).to_dict(),
        ],
        save=False,
    )
    store.attach_run_result(task, run_result)
    manifest = writer.write_run_result(task, plan, run_result)
    run_result["manifest_validation"] = validate_appflow_manifest(manifest, task)
    store.attach_gate_review(task, _manifest_gate_review(run_result["manifest_validation"]), save=False)
    store.attach_run_result(task, run_result)
    writer.write_run_result(task, plan, run_result)
    return run_result


def _execution_gate_review(run_result: dict) -> dict:
    pipeline = run_result.get("pipeline", [])
    if isinstance(pipeline, list) and pipeline:
        return review_run_pipeline(pipeline).to_dict()

    result = GateResult("execution")
    status = str(run_result.get("status", "")).strip()
    if status in {"run_completed", "run_ready"}:
        return result.to_dict()

    result.status = "failed"
    reason = str(run_result.get("reason", "")).strip() or "Run did not reach execution."
    result.issues.append(GateIssue("execution.not_ready", reason))
    for item in run_result.get("missing_files", [])[:10]:
        result.issues.append(GateIssue("execution.missing_file", f"Missing required file: {item}."))
    return result.to_dict()


def _manifest_gate_review(manifest_validation: dict) -> dict:
    result = GateResult("manifest")
    if str(manifest_validation.get("status", "")).strip() == "valid":
        return result.to_dict()

    result.status = "failed"
    for item in manifest_validation.get("missing_fields", [])[:20]:
        result.issues.append(GateIssue("manifest.missing_field", f"Missing manifest field: {item}."))
    for item in manifest_validation.get("missing_paths", [])[:20]:
        field = item.get("field", "")
        path = item.get("path", "")
        result.issues.append(GateIssue("manifest.missing_path", f"{field} path does not exist: {path}."))
    if not result.issues:
        result.issues.append(GateIssue("manifest.invalid", "AppFlow manifest validation failed."))
    return result.to_dict()
