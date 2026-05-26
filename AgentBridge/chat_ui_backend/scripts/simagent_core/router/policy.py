from dataclasses import asdict, dataclass

from .intent import CaseIntent


SUPPORTED_GENERATED_GEOMETRIES = {"rect_channel"}
GENERATED_CONFIDENCE_THRESHOLD = 0.75
STRONG_REFERENCE_THRESHOLD = 70


@dataclass
class RouteDecision:
    selected_mode: str
    geometry_type: str = ""
    confidence: float = 0.0
    reason: str = ""

    def to_dict(self) -> dict:
        return asdict(self)


def select_generation_route(intent: CaseIntent, reference_score: int | None = None) -> RouteDecision:
    if intent.intent != "generated_case":
        return RouteDecision(
            selected_mode="reference_modify",
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reason=intent.reason or "Use reference planning.",
        )

    if reference_score is not None and _has_strong_reference(reference_score) and not intent.explicit_new_geometry:
        return RouteDecision(
            selected_mode="reference_modify",
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reason=f"Use reference planning because reference score is {reference_score}.",
        )

    if intent.geometry_type in SUPPORTED_GENERATED_GEOMETRIES:
        if intent.confidence >= GENERATED_CONFIDENCE_THRESHOLD or intent.explicit_new_geometry:
            return RouteDecision(
                selected_mode="generated_case",
                geometry_type=intent.geometry_type,
                confidence=intent.confidence,
                reason=intent.reason or "Use supported generated geometry.",
            )

        return RouteDecision(
            selected_mode="reference_modify",
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reason="Generated geometry confidence is below threshold; use reference planning.",
        )

    if intent.explicit_new_geometry:
        return RouteDecision(
            selected_mode="unsupported",
            geometry_type=intent.geometry_type,
            confidence=intent.confidence,
            reason=(
                "The request asks for a generated case, but this geometry is not supported yet. "
                "Supported generated geometry: rect_channel."
            ),
        )

    return RouteDecision(
        selected_mode="reference_modify",
        geometry_type=intent.geometry_type,
        confidence=intent.confidence,
        reason="Unsupported or unclear generated geometry; use reference planning.",
    )


def _has_strong_reference(reference_score: int) -> bool:
    return reference_score >= STRONG_REFERENCE_THRESHOLD
