from typing import Any


def current_repair_action(task: Any) -> dict[str, Any]:
    gate_reviews = getattr(task, "gate_reviews", [])
    if not isinstance(gate_reviews, list):
        return {}

    for review in reversed(gate_reviews):
        if not isinstance(review, dict):
            continue
        if str(review.get("status", "")).strip() == "passed":
            continue

        action = review.get("next_repair_action", {})
        return action if isinstance(action, dict) else {}
    return {}


def repair_action_for_gate_review(gate_review: dict[str, Any]) -> dict[str, Any]:
    if str(gate_review.get("status", "")).strip() == "passed":
        return {}

    issues = gate_review.get("issues", [])
    issue_codes = [
        str(item.get("code", "")).strip()
        for item in issues
        if isinstance(item, dict) and str(item.get("code", "")).strip()
    ]
    if not issue_codes:
        action = _repair_action_for_gate_decision(gate_review)
        if action:
            return action
        return _action(
            "inspect_gate_failure",
            "Inspect gate failure",
            "The gate failed without a specific issue code. Inspect task_context.json and the generated files.",
        )

    for code in issue_codes:
        action = _repair_action_for_issue_code(code, gate_review)
        if action:
            return action

    return _action(
        "inspect_gate_failure",
        "Inspect gate failure",
        f"No repair proposal is registered for issue code: {issue_codes[0]}.",
    )


def _repair_action_for_issue_code(code: str, gate_review: dict[str, Any]) -> dict[str, Any]:
    if code in {"execution.not_ready", "result.run_incomplete"}:
        return _action(
            "configure_openfoam_runtime",
            "Configure OpenFOAM runtime",
            "OpenFOAM did not run. Source OpenFOAM and ensure WM_PROJECT_DIR is set before running again.",
        )
    if code in {"execution.missing_file", "validation.missing_file", "file.empty_path", "file.disallowed_root"}:
        return _action(
            "regenerate_case",
            "Regenerate case",
            "Required case files are missing or invalid. Generate the case again before validation or run.",
        )
    if code in {
        "validation.missing_boundary_field",
        "validation.extra_boundary_field",
        "validation.dictionary_error",
        "validation.failed",
    }:
        return _action(
            "repair_case_dictionaries",
            "Repair case dictionaries",
            "OpenFOAM dictionary validation failed. Review boundary fields and dictionary entries before running.",
        )
    if code in {"result.missing_latest_path", "result.missing_latest_dir"}:
        return _action(
            "review_solver_logs",
            "Review solver logs",
            "The run completed but no usable result time directory was found. Inspect solver logs and rerun if needed.",
        )
    if code in {"result.residual_nan", "result.residual_high"}:
        return _solver_numerics_action(code, gate_review)
    if code in {"result.mesh_quality_failed", "result.mesh_quality_warning"}:
        return _mesh_quality_action(gate_review)
    if code == "result.no_execution_time":
        return _action(
            "review_solver_logs",
            "Review solver logs",
            "The solver log has residual output but no normal completion marker. Inspect the log before trusting results.",
        )
    if code == "result.log_fatal":
        return _action(
            "inspect_fatal_log",
            "Inspect fatal log",
            "A solver log contains a fatal marker. Open the log, identify the failing command, then repair the case.",
        )
    if code in {"manifest.missing_field", "manifest.missing_path", "manifest.invalid"}:
        return _action(
            "rewrite_manifest",
            "Rewrite manifest",
            "The AppFlow manifest is incomplete or points to missing paths. Check host/backend path mapping and rewrite it.",
        )
    if code.startswith("pipeline."):
        return _action(
            "fix_run_pipeline",
            "Fix run pipeline",
            "The selected run pipeline is not executable under the execution gate. Review commands and arguments.",
        )
    if code.startswith("geometry.") or code.startswith("mesh.") or code.startswith("boundary.") or code.startswith("solver."):
        return _action(
            "revise_simulation_spec",
            "Revise simulation spec",
            "The simulation spec is unsupported or incomplete. Revise the plan before generation.",
        )
    return {}


def _solver_numerics_action(code: str, gate_review: dict[str, Any]) -> dict[str, Any]:
    metrics = _diagnostic_metrics(gate_review)
    if code == "result.residual_nan" or bool(metrics.get("has_nan_or_inf", False)):
        return _action(
            "inspect_solver_numerics",
            "Inspect unstable numerics",
            "The solver log contains nan or inf. Check boundary conditions, field initialization, timestep/Courant number, and mesh quality before rerunning.",
        )

    field = str(metrics.get("worst_residual_field", "")).strip()
    final = _safe_float(metrics.get("max_final_residual"))
    final_text = f" {final:.3g}" if final is not None else ""
    field_key = field.lower()

    if field_key == "p":
        return _action(
            "inspect_solver_numerics",
            "Inspect pressure numerics",
            f"Pressure residual is the worst signal{final_text}. Check pressure boundary conditions, pressure solver tolerance, relaxation factors, and pressure-velocity coupling.",
        )
    if field_key in {"u", "ux", "uy", "uz"}:
        return _action(
            "inspect_solver_numerics",
            "Inspect velocity numerics",
            f"Velocity residual is the worst signal{final_text}. Check inlet/outlet velocity conditions, timestep/Courant number, mesh quality, and relaxation factors.",
        )
    if field:
        return _action(
            "inspect_solver_numerics",
            f"Inspect {field} numerics",
            f"{field} has the highest final residual{final_text}. Review its boundary conditions, solver settings, relaxation factors, and mesh quality.",
        )
    return _action(
        "inspect_solver_numerics",
        "Inspect solver numerics",
        "Solver residuals look unstable. Review boundary conditions, timestep, mesh quality, and numerics settings.",
    )


def _mesh_quality_action(gate_review: dict[str, Any]) -> dict[str, Any]:
    metrics = _diagnostic_metrics(gate_review)
    failed_checks = _safe_int(metrics.get("failed_mesh_checks"))
    non_orthogonality = _safe_float(metrics.get("max_non_orthogonality"))
    skewness = _safe_float(metrics.get("max_skewness"))
    min_volume = _safe_float(metrics.get("min_volume"))

    if failed_checks and failed_checks > 0:
        return _action(
            "review_mesh_quality",
            "Review failed mesh checks",
            f"checkMesh reported {failed_checks} failed mesh check(s). Inspect the checkMesh log, locate bad cells, and repair the mesh before trusting results.",
        )
    if min_volume is not None and min_volume <= 0:
        return _action(
            "review_mesh_quality",
            "Review invalid cell volumes",
            f"Minimum cell volume is {min_volume:.3g}. Fix zero or negative volume cells before running the solver again.",
        )
    if non_orthogonality is not None and non_orthogonality >= 70:
        return _action(
            "review_mesh_quality",
            "Review non-orthogonal mesh",
            f"Maximum non-orthogonality is {non_orthogonality:.3g}. Improve mesh quality or review nonOrthogonalCorrectors before trusting the result.",
        )
    if skewness is not None and skewness >= 4:
        return _action(
            "review_mesh_quality",
            "Review skewed mesh",
            f"Maximum skewness is {skewness:.3g}. Inspect skewed cells and improve the mesh before trusting solver convergence.",
        )
    return _action(
        "review_mesh_quality",
        "Review mesh quality",
        "Mesh quality checks reported issues. Inspect checkMesh or blockMesh logs before trusting solver results.",
    )


def _diagnostic_metrics(gate_review: dict[str, Any]) -> dict[str, Any]:
    diagnostics = gate_review.get("diagnostics", {})
    if not isinstance(diagnostics, dict):
        return {}

    metrics = diagnostics.get("metrics", {})
    return metrics if isinstance(metrics, dict) else {}


def _safe_float(value: Any) -> float | None:
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def _safe_int(value: Any) -> int | None:
    try:
        return int(value)
    except (TypeError, ValueError):
        return None


def _repair_action_for_gate_decision(gate_review: dict[str, Any]) -> dict[str, Any]:
    if str(gate_review.get("gate", "")).strip() != "capability":
        return {}

    decision = gate_review.get("decision", {})
    if not isinstance(decision, dict):
        return {}

    if str(decision.get("mode", "")).strip() == "unsupported":
        return _action(
            "revise_request_scope",
            "Revise request scope",
            "The requested case is outside current capabilities. Use a supported reference case or generated geometry.",
        )
    return {}


def _action(action_id: str, label: str, description: str) -> dict[str, Any]:
    return {
        "id": action_id,
        "label": label,
        "description": description,
        "automatic": False,
    }
