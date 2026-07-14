from dataclasses import asdict, dataclass, field
from typing import Any

from ..models import ReferenceCase
from ..router import CaseIntent, RouteDecision, select_generation_route
from ..solvers.openfoam.capabilities import (
    STATUS_NEEDS_USER_INPUT,
    STATUS_PARTIALLY_SUPPORTED,
    STATUS_SUPPORTED,
    STATUS_UNSUPPORTED,
    evaluate_capability_matrix,
)


@dataclass
class CapabilityDecision:
    status: str
    mode: str
    boundary: str
    reason: str
    support_status: str = STATUS_SUPPORTED
    geometry_type: str = ""
    confidence: float = 0.0
    reference_score: int = 0
    supported_changes: dict[str, Any] = field(default_factory=dict)
    unsupported_changes: dict[str, Any] = field(default_factory=dict)
    needs_user_input: list[str] = field(default_factory=list)
    capability_matrix: dict[str, Any] = field(default_factory=dict)

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)

    def to_gate_review(self) -> dict[str, Any]:
        return {
            "gate": "capability",
            "status": self.status,
            "issues": _decision_issues(self),
            "decision": self.to_dict(),
        }


def review_openfoam_capability(
    *,
    intent: CaseIntent,
    reference_case: ReferenceCase | None = None,
    reference_selection: dict[str, Any] | None = None,
    supported_changes: dict[str, Any] | None = None,
    unsupported_changes: dict[str, Any] | None = None,
    mesh_request: dict[str, Any] | None = None,
    route_decision: RouteDecision | None = None,
) -> CapabilityDecision:
    reference_score = _reference_score(reference_selection)
    supported = supported_changes or {}
    unsupported = unsupported_changes or {}
    route = route_decision or select_generation_route(intent, reference_score=reference_score)

    if route.selected_mode == "unsupported":
        return CapabilityDecision(
            status="unsupported",
            mode="unsupported",
            boundary="unsupported",
            reason=route.reason,
            support_status=STATUS_UNSUPPORTED,
            geometry_type=route.geometry_type,
            confidence=route.confidence,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
            capability_matrix=evaluate_capability_matrix(
                reference_case=reference_case,
                requested_changes={**supported, **unsupported},
                generation_mode="generated_case",
                geometry_type=route.geometry_type,
                mesh_request=mesh_request,
            ).to_dict(),
        )

    return _decision_from_matrix(
        mode=route.selected_mode,
        reason=route.reason,
        intent=intent,
        reference_case=reference_case,
        reference_score=reference_score,
        supported_changes=supported,
        unsupported_changes=unsupported,
        mesh_request=mesh_request,
    )


def _decision_from_matrix(
    *,
    mode: str,
    reason: str,
    intent: CaseIntent,
    reference_case: ReferenceCase | None,
    reference_score: int,
    supported_changes: dict[str, Any],
    unsupported_changes: dict[str, Any],
    mesh_request: dict[str, Any] | None,
) -> CapabilityDecision:
    requested_changes = {**supported_changes, **unsupported_changes}
    matrix_mode = mode if mode != "unsupported" else ""
    report = evaluate_capability_matrix(
        reference_case=reference_case,
        requested_changes=requested_changes,
        generation_mode=matrix_mode,
        geometry_type=intent.geometry_type,
        mesh_request=mesh_request,
    )
    support_status = report.status
    decision_mode = mode
    boundary = "controlled"
    status = "passed"

    if support_status == STATUS_PARTIALLY_SUPPORTED:
        status = "warning"
        boundary = "partial"
    elif support_status == STATUS_NEEDS_USER_INPUT:
        status = "warning"
        decision_mode = "needs_user_input"
        boundary = "needs_user_input"
    elif support_status == STATUS_UNSUPPORTED:
        status = "unsupported"
        decision_mode = "unsupported"
        boundary = "unsupported"

    return CapabilityDecision(
        status=status,
        mode=decision_mode,
        boundary=boundary,
        reason=_reason_with_matrix_status(reason, support_status),
        support_status=support_status,
        geometry_type=intent.geometry_type,
        confidence=intent.confidence,
        reference_score=reference_score,
        supported_changes=report.supported_changes,
        unsupported_changes=report.unsupported_changes,
        needs_user_input=report.needs_user_input,
        capability_matrix=report.to_dict(),
    )


def _reason_with_matrix_status(reason: str, support_status: str) -> str:
    if support_status == STATUS_SUPPORTED:
        return reason
    if support_status == STATUS_PARTIALLY_SUPPORTED:
        return f"{reason} Some requested changes are not supported and will not be applied."
    if support_status == STATUS_NEEDS_USER_INPUT:
        return f"{reason} More input is required before generating the case."
    return f"{reason} The request is outside the current OpenFOAM capability matrix."


def _decision_issues(decision: CapabilityDecision) -> list[dict[str, Any]]:
    issues: list[dict[str, Any]] = []
    for key in sorted(decision.unsupported_changes):
        severity = "warning" if decision.support_status == STATUS_PARTIALLY_SUPPORTED else "error"
        issues.append(
            {
                "code": "capability.unsupported_change",
                "message": f"Unsupported requested change: {key}.",
                "severity": severity,
            }
        )
    for field_name in decision.needs_user_input:
        issues.append(
            {
                "code": "capability.needs_user_input",
                "message": f"Additional input is required: {field_name}.",
                "severity": "warning",
            }
        )
    if decision.support_status == STATUS_UNSUPPORTED and not issues:
        issues.append(
            {
                "code": "capability.unsupported_request",
                "message": decision.reason,
                "severity": "error",
            }
        )
    return issues


def _reference_score(reference_selection: dict[str, Any] | None) -> int:
    if not isinstance(reference_selection, dict):
        return 0
    try:
        return int(reference_selection.get("selected_score", 0) or 0)
    except (TypeError, ValueError):
        return 0
