from dataclasses import asdict, dataclass, field
from typing import Any

from ..models import ReferenceCase
from ..router import CaseIntent
from ..router.policy import GENERATED_CONFIDENCE_THRESHOLD, STRONG_REFERENCE_THRESHOLD, SUPPORTED_GENERATED_GEOMETRIES
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
) -> CapabilityDecision:
    reference_score = _reference_score(reference_selection)
    supported = supported_changes or {}
    unsupported = unsupported_changes or {}

    if intent.explicit_new_geometry:
        if intent.geometry_type in SUPPORTED_GENERATED_GEOMETRIES:
            return _decision_from_matrix(
                mode="generated_case",
                reason="User explicitly requested new geometry and the geometry type is supported.",
                intent=intent,
                reference_case=reference_case,
                reference_score=reference_score,
                supported_changes=supported,
                unsupported_changes=unsupported,
                mesh_request=mesh_request,
            )
        return CapabilityDecision(
            status="unsupported",
            mode="unsupported",
            boundary="unsupported",
            reason=(
                "The request asks for a generated case, but this geometry is not supported yet. "
                "Supported generated geometry: rect_channel."
            ),
            support_status=STATUS_UNSUPPORTED,
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
            capability_matrix=evaluate_capability_matrix(
                reference_case=reference_case,
                requested_changes={**supported, **unsupported},
                generation_mode="generated_case",
                geometry_type=intent.geometry_type,
                mesh_request=mesh_request,
            ).to_dict(),
        )

    if reference_case and reference_score >= STRONG_REFERENCE_THRESHOLD:
        return _decision_from_matrix(
            mode="reference_modify" if supported else "reference_copy",
            reason=f"Reference score is {reference_score}, so use the official tutorial reference.",
            intent=intent,
            reference_case=reference_case,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
            mesh_request=mesh_request,
        )

    if (
        intent.geometry_type in SUPPORTED_GENERATED_GEOMETRIES
        and intent.confidence >= GENERATED_CONFIDENCE_THRESHOLD
    ):
        return _decision_from_matrix(
            mode="generated_case",
            reason="Reference match is weak, but the detected geometry type is supported.",
            intent=intent,
            reference_case=reference_case,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
            mesh_request=mesh_request,
        )

    if reference_case:
        return _decision_from_matrix(
            mode="reference_modify" if supported else "reference_copy",
            reason="Use the best available tutorial reference because no supported generated geometry was selected.",
            intent=intent,
            reference_case=reference_case,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
            mesh_request=mesh_request,
        )

    return _decision_from_matrix(
        mode="unsupported",
        reason="No reliable reference case or supported generated geometry is available.",
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
