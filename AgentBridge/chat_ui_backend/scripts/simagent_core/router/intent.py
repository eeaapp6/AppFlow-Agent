from dataclasses import asdict, dataclass


EXPLICIT_NEW_GEOMETRY_MARKERS = [
    "new geometry",
    "generate geometry",
    "generated geometry",
    "not in database",
    "not in the database",
    "without reference",
    "from scratch",
    "\u751f\u6210\u65b0\u51e0\u4f55",
    "\u65b0\u51e0\u4f55",
    "\u6570\u636e\u5e93\u6ca1\u6709",
    "\u6848\u4f8b\u5e93\u6ca1\u6709",
    "\u4e0d\u53c2\u8003\u6848\u4f8b\u5e93",
    "\u4ece\u96f6\u751f\u6210",
]

RECT_CHANNEL_MARKERS = [
    "rect_channel",
    "rectangular channel",
    "rectangular pipe",
    "\u77e9\u5f62\u7ba1\u9053",
    "\u77e9\u5f62\u901a\u9053",
]

TWO_DIMENSIONAL_MARKERS = [
    "2d",
    "two-dimensional",
    "\u4e8c\u7ef4",
]

CHANNEL_MARKERS = [
    "channel",
    "pipe",
    "\u7ba1\u9053",
    "\u901a\u9053",
]


@dataclass
class CaseIntent:
    intent: str = "reference_modify"
    geometry_type: str = ""
    confidence: float = 0.0
    explicit_new_geometry: bool = False
    reason: str = ""

    def to_dict(self) -> dict:
        return asdict(self)


def parse_case_intent(user_requirement: str) -> CaseIntent:
    text = str(user_requirement or "")
    normalized = text.lower()
    explicit_new = _contains_any(text, normalized, EXPLICIT_NEW_GEOMETRY_MARKERS)

    if _has_rect_channel_geometry(text, normalized):
        confidence = 0.9 if explicit_new else 0.8
        return CaseIntent(
            intent="generated_case",
            geometry_type="rect_channel",
            confidence=confidence,
            explicit_new_geometry=explicit_new,
            reason="The request describes a supported rectangular or 2D channel geometry.",
        )

    if explicit_new:
        return CaseIntent(
            intent="generated_case",
            geometry_type="",
            confidence=0.55,
            explicit_new_geometry=True,
            reason="The request asks for generated geometry, but no supported geometry type was detected.",
        )

    return CaseIntent(
        intent="reference_modify",
        geometry_type="",
        confidence=0.0,
        explicit_new_geometry=False,
        reason="No supported generated geometry was detected.",
    )


def _has_rect_channel_geometry(text: str, normalized: str) -> bool:
    if _contains_any(text, normalized, RECT_CHANNEL_MARKERS):
        return True

    has_2d = _contains_any(text, normalized, TWO_DIMENSIONAL_MARKERS)
    has_channel = _contains_any(text, normalized, CHANNEL_MARKERS)
    return has_2d and has_channel


def _contains_any(text: str, normalized: str, markers: list[str]) -> bool:
    for marker in markers:
        target = normalized if marker.isascii() else text
        if marker.lower() in target:
            return True
    return False
