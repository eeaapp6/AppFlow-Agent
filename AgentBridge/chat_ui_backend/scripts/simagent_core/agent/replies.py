def plan_reply(plan, task) -> str:
    plan_data = plan.to_dict()
    requested_changes = plan_data.get("requested_changes", {})
    parameters = plan_data.get("parameters", {}) if isinstance(plan_data.get("parameters", {}), dict) else {}
    unsupported_changes = parameters.get("unsupported_changes", {})
    lines = [
        "Plan is ready.",
        f"Case: {plan_data.get('case_name', '')} ({plan_data.get('solver_name', '')})",
    ]
    if requested_changes:
        lines.append("Changes:")
        for key, value in requested_changes.items():
            lines.append(f"- {key}: {value}")
    mesh = parameters.get("mesh", {})
    if isinstance(mesh, dict) and mesh.get("mode") == "gmsh_external":
        source_path = str(mesh.get("source_path", "")).strip()
        lines.append("Mesh: GMSH .msh import")
        if source_path:
            lines.append(f"- source: {source_path}")
        else:
            lines.append("- source: missing .msh path")
    if unsupported_changes:
        lines.append("Some requested changes are not supported yet and will be skipped.")
    lines.append("Next: Generate Case.")
    return "\n".join(lines)


def generate_reply(task, generated_files: list[dict], reference_files: list[dict] | None = None) -> str:
    return "Case files generated.\nNext: Validate Case."


def replan_reply(replan_result: dict) -> str:
    if replan_result.get("status") == "updated":
        return "Parameters updated. Please regenerate the case."
    return "No supported parameter changes were detected."


def next_action(action_id: str) -> dict:
    actions = {
        "generate": {
            "id": "generate",
            "label": "Generate Case",
            "endpoint": "/foam/generate",
            "requires_confirmation": True,
        },
        "validate": {
            "id": "validate",
            "label": "Validate Case",
            "endpoint": "/foam/validate",
            "requires_confirmation": True,
        },
        "run": {
            "id": "run",
            "label": "Run OpenFOAM",
            "endpoint": "/foam/run",
            "requires_confirmation": True,
        },
        "import_manifest": {
            "id": "import_manifest",
            "label": "Import to AppFlow",
            "endpoint": "appflow://import_manifest",
            "requires_confirmation": True,
        },
    }
    return actions.get(action_id, {})


def validate_reply(task, validation_result: dict) -> str:
    missing_files = validation_result.get("missing_files", [])
    dictionary_errors = validation_result.get("dictionary_errors", [])
    missing_boundary_fields = validation_result.get("missing_boundary_fields", {})
    extra_boundary_fields = validation_result.get("extra_boundary_fields", {})
    has_errors = bool(missing_files or dictionary_errors or missing_boundary_fields or extra_boundary_fields)

    if not has_errors:
        return "Case validation passed.\nNext: Run Case."

    lines = ["Case validation failed."]
    if missing_files:
        lines.append("Missing files:")
        for item in missing_files[:5]:
            lines.append(f"- {item}")
    if missing_boundary_fields:
        lines.append("Missing boundary fields:")
        for field_name, patches in missing_boundary_fields.items():
            lines.append(f"- {field_name}: {', '.join(patches)}")
    if extra_boundary_fields:
        lines.append("Extra boundary fields:")
        for field_name, patches in extra_boundary_fields.items():
            lines.append(f"- {field_name}: {', '.join(patches)}")
    if dictionary_errors:
        lines.append("Dictionary errors:")
        for item in dictionary_errors[:5]:
            lines.append(f"- {item.get('file', '')}: {item.get('message', '')}")
    return "\n".join(lines)


def pipeline_summary(run_result: dict) -> str:
    pipeline_info = run_result.get("pipeline_info", {})
    if not isinstance(pipeline_info, dict):
        return ""

    mode = str(pipeline_info.get("mode", "")).strip()
    selected = str(pipeline_info.get("selected", "")).strip()
    fallback_used = bool(pipeline_info.get("fallback_used", False))
    if not mode and not selected:
        return ""

    selected_text = selected or mode
    if fallback_used and mode and selected and mode != selected:
        return f"Run mode: {mode} -> {selected} fallback"
    if mode and selected and mode != selected:
        return f"Run mode: {mode} -> {selected}"
    return f"Run mode: {selected_text}"


def run_reply(task, run_result: dict) -> str:
    status = str(run_result.get("status", "")).strip()
    if status == "run_completed":
        lines = ["OpenFOAM run completed."]
        summary = pipeline_summary(run_result)
        if summary:
            lines.append(summary)
        lines.extend(result_review_summary_lines(task))
        lines.append("AppFlow manifest is ready.")
        return "\n".join(lines)

    reason = str(run_result.get("reason", "")).strip()
    lines = ["OpenFOAM run failed."]
    summary = pipeline_summary(run_result)
    if summary:
        lines.append(summary)
    if reason:
        lines.append(f"Reason: {reason}")

    missing_files = run_result.get("missing_files", [])
    if missing_files:
        lines.append("Missing files:")
        for item in missing_files[:5]:
            lines.append(f"- {item}")

    steps = run_result.get("steps", [])
    failed_steps = [
        item for item in steps
        if isinstance(item, dict) and int(item.get("return_code", 0) or 0) != 0
    ]
    if failed_steps:
        lines.append("Failed step:")
        item = failed_steps[0]
        lines.append(f"- {item.get('command', '')}, log: {item.get('log', '')}")
    lines.extend(result_review_summary_lines(task))
    return "\n".join(lines)


def result_review_summary_lines(task) -> list[str]:
    review = latest_gate_review(task, "result_review")
    if not review:
        return []

    status = str(review.get("status", "")).strip()
    if status == "passed":
        return ["Result review passed."]

    diagnostics = review.get("diagnostics", {})
    diagnostics = diagnostics if isinstance(diagnostics, dict) else {}
    primary_issue = diagnostics.get("primary_issue", {})
    primary_issue = primary_issue if isinstance(primary_issue, dict) else {}
    categories = diagnostics.get("categories", [])
    categories = categories if isinstance(categories, list) else []
    evidence = diagnostics.get("evidence", {})
    evidence = evidence if isinstance(evidence, dict) else {}

    lines = [result_review_heading(status)]
    category_text = ", ".join(str(item) for item in categories if str(item).strip())
    if category_text:
        lines.append(f"Main area: {category_text}.")

    message = str(primary_issue.get("message", "")).strip()
    if not message:
        message = str(diagnostics.get("summary", "")).strip()
    if message:
        lines.append(f"Main issue: {message}")
    evidence_line = result_review_evidence_line(evidence)
    if evidence_line:
        lines.append(evidence_line)
    return lines


def latest_gate_review(task, gate_name: str) -> dict:
    gate_reviews = getattr(task, "gate_reviews", [])
    if not isinstance(gate_reviews, list):
        return {}

    for review in reversed(gate_reviews):
        if isinstance(review, dict) and str(review.get("gate", "")).strip() == gate_name:
            return review
    return {}


def result_review_heading(status: str) -> str:
    if status == "warning":
        return "Result review warning."
    if status == "failed":
        return "Result review failed."
    return "Result review needs attention."


def result_review_evidence_line(evidence: dict) -> str:
    path = str(evidence.get("log_path", "") or evidence.get("path", "")).strip()
    if not path:
        return ""

    line = evidence.get("line", "")
    try:
        line_number = int(line)
    except (TypeError, ValueError):
        line_number = 0

    location = f"{path}:{line_number}" if line_number > 0 else path
    excerpt = str(evidence.get("excerpt", "")).strip()
    if excerpt:
        return f"Evidence: {location} - {excerpt}"
    return f"Evidence: {location}"
