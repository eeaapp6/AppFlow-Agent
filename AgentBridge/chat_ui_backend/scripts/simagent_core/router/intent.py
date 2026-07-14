import re
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

CREATE_ACTION_MARKERS = [
    "generate",
    "create",
    "build",
    "\u751f\u6210",
    "\u521b\u5efa",
    "\u65b0\u5efa",
]

MODIFY_ACTION_MARKERS = [
    "modify",
    "change",
    "adjust",
    "set ",
    "\u4fee\u6539",
    "\u8c03\u6574",
    "\u6539\u4e3a",
]

COPY_ACTION_MARKERS = [
    "copy",
    "use ",
    "run ",
    "\u590d\u5236",
    "\u4f7f\u7528",
    "\u8fd0\u884c",
    "\u8ba1\u7b97",
]

NEW_CASE_TARGET_MARKERS = [
    "geometry",
    "case",
    "flow field",
    "\u51e0\u4f55",
    "\u7b97\u4f8b",
    "\u6d41\u573a",
]

RESULT_OUTPUT_MARKERS = [
    "result file",
    "results file",
    "output file",
    "\u7ed3\u679c\u6587\u4ef6",
    "\u8f93\u51fa\u6587\u4ef6",
]

REFERENCE_SOURCE_MARKERS = [
    "tutorial",
    "reference case",
    "\u53c2\u8003\u6848\u4f8b",
    "\u53c2\u8003\u7b97\u4f8b",
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
    action: str = "unknown"
    geometry_type: str = ""
    confidence: float = 0.0
    explicit_new_geometry: bool = False
    reference_case: str = ""
    reason: str = ""

    def to_dict(self) -> dict:
        return asdict(self)


def parse_case_intent(user_requirement: str) -> CaseIntent:
    text = str(user_requirement or "")
    normalized = text.lower()
    has_rect_channel = _has_rect_channel_geometry(text, normalized)
    action = _detect_action(text, normalized, has_rect_channel)
    explicit_new = action == "create"
    reference_case = _extract_reference_case(text)

    if has_rect_channel:
        confidence = 0.9 if explicit_new else 0.8
        return CaseIntent(
            intent="generated_case",
            action=action,
            geometry_type="rect_channel",
            confidence=confidence,
            explicit_new_geometry=explicit_new,
            reference_case=reference_case,
            reason="The request describes a supported rectangular or 2D channel geometry.",
        )

    if explicit_new:
        return CaseIntent(
            intent="generated_case",
            action=action,
            geometry_type="",
            confidence=0.55,
            explicit_new_geometry=True,
            reference_case=reference_case,
            reason="The request asks for generated geometry, but no supported geometry type was detected.",
        )

    return CaseIntent(
        intent="reference_copy" if action == "copy" else "reference_modify",
        action=action,
        geometry_type="",
        confidence=0.0,
        explicit_new_geometry=False,
        reference_case=reference_case,
        reason="No supported generated geometry was detected.",
    )


def is_explicit_supported_geometry_creation(
    intent: CaseIntent,
    supported_geometries: set[str],
) -> bool:
    return (
        intent.action == "create"
        and intent.explicit_new_geometry
        and intent.geometry_type in supported_geometries
    )


def _detect_action(text: str, normalized: str, has_supported_geometry: bool) -> str:
    if _contains_any(text, normalized, MODIFY_ACTION_MARKERS):
        return "modify"
    if _contains_any(text, normalized, COPY_ACTION_MARKERS):
        return "copy"
    if (
        _contains_any(text, normalized, CREATE_ACTION_MARKERS)
        and (
            _contains_any(text, normalized, REFERENCE_SOURCE_MARKERS)
            or re.search(r"\b[A-Za-z][A-Za-z0-9_-]*Foam\s+[A-Za-z][A-Za-z0-9_-]*\s+case\b", text)
        )
    ):
        return "copy"
    if not _contains_any(text, normalized, CREATE_ACTION_MARKERS):
        return "unknown"
    if has_supported_geometry:
        return "create"
    if _contains_any(text, normalized, RESULT_OUTPUT_MARKERS):
        return "unknown"
    if (
        _contains_any(text, normalized, EXPLICIT_NEW_GEOMETRY_MARKERS)
        or _contains_any(text, normalized, NEW_CASE_TARGET_MARKERS)
    ):
        return "create"
    return "unknown"


def _extract_reference_case(text: str) -> str:
    patterns = [
        r"(?:\u4fee\u6539|\u590d\u5236|\u4f7f\u7528|\u57fa\u4e8e)\s*([A-Za-z][A-Za-z0-9_-]*)",
        r"\b(?:modify|copy|use|run)\s+(?:the\s+)?([A-Za-z][A-Za-z0-9_-]*)",
        r"\b([A-Za-z][A-Za-z0-9_-]*)\s+(?:tutorial|case)\b",
    ]
    for pattern in patterns:
        match = re.search(pattern, text, flags=re.IGNORECASE)
        if match:
            return match.group(1)
    return ""


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
