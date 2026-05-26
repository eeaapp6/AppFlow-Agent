from dataclasses import asdict, dataclass, field
from typing import Any

from ..models import ReferenceCase
from ..router import CaseIntent
from ..router.policy import GENERATED_CONFIDENCE_THRESHOLD, STRONG_REFERENCE_THRESHOLD, SUPPORTED_GENERATED_GEOMETRIES


@dataclass
class CapabilityDecision:
    status: str
    mode: str
    boundary: str
    reason: str
    geometry_type: str = ""
    confidence: float = 0.0
    reference_score: int = 0
    supported_changes: dict[str, Any] = field(default_factory=dict)
    unsupported_changes: dict[str, Any] = field(default_factory=dict)

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)

    def to_gate_review(self) -> dict[str, Any]:
        return {
            "gate": "capability",
            "status": self.status,
            "issues": [],
            "decision": self.to_dict(),
        }


def review_openfoam_capability(
    *,
    intent: CaseIntent,
    reference_case: ReferenceCase | None = None,
    reference_selection: dict[str, Any] | None = None,
    supported_changes: dict[str, Any] | None = None,
    unsupported_changes: dict[str, Any] | None = None,
) -> CapabilityDecision:
    reference_score = _reference_score(reference_selection)
    supported = supported_changes or {}
    unsupported = unsupported_changes or {}

    if intent.explicit_new_geometry:
        if intent.geometry_type in SUPPORTED_GENERATED_GEOMETRIES:
            return CapabilityDecision(
                status="passed",
                mode="generated_case",
                boundary="controlled",
                reason="User explicitly requested new geometry and the geometry type is supported.",
                geometry_type=intent.geometry_type,
                confidence=intent.confidence,
                reference_score=reference_score,
                supported_changes=supported,
                unsupported_changes=unsupported,
            )
        return CapabilityDecision(
            status="unsupported",
            mode="unsupported",
            boundary="unsupported",
            reason=(
                "The request asks for a generated case, but this geometry is not supported yet. "
                "Supported generated geometry: rect_channel."
            ),
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
        )

    if reference_case and reference_score >= STRONG_REFERENCE_THRESHOLD:
        return CapabilityDecision(
            status="passed",
            mode="reference_modify" if supported else "reference_copy",
            boundary="controlled",
            reason=f"Reference score is {reference_score}, so use the official tutorial reference.",
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
        )

    if (
        intent.geometry_type in SUPPORTED_GENERATED_GEOMETRIES
        and intent.confidence >= GENERATED_CONFIDENCE_THRESHOLD
    ):
        return CapabilityDecision(
            status="passed",
            mode="generated_case",
            boundary="controlled",
            reason="Reference match is weak, but the detected geometry type is supported.",
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
        )

    if reference_case:
        return CapabilityDecision(
            status="passed",
            mode="reference_modify" if supported else "reference_copy",
            boundary="controlled",
            reason="Use the best available tutorial reference because no supported generated geometry was selected.",
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reference_score=reference_score,
            supported_changes=supported,
            unsupported_changes=unsupported,
        )

    return CapabilityDecision(
        status="unsupported",
        mode="unsupported",
        boundary="unsupported",
        reason="No reliable reference case or supported generated geometry is available.",
        geometry_type=intent.geometry_type,
        confidence=intent.confidence,
        reference_score=reference_score,
        supported_changes=supported,
        unsupported_changes=unsupported,
    )


def _reference_score(reference_selection: dict[str, Any] | None) -> int:
    if not isinstance(reference_selection, dict):
        return 0
    try:
        return int(reference_selection.get("selected_score", 0) or 0)
    except (TypeError, ValueError):
        return 0
