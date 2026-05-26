from pathlib import PurePosixPath

from .models import GateResult
from ..models import PlannedFile


ALLOWED_CASE_ROOTS = {"case", "logs", "results", "mesh"}
ALLOWED_FILE_FORMATS = {
    "openfoam-dict",
    "gmsh-msh",
    "gmsh-geo",
    "stl",
    "text",
}


def review_generated_files(files: list[PlannedFile]) -> GateResult:
    result = GateResult("generation")
    seen: set[str] = set()
    for item in files:
        normalized = _normalize_relative_path(item.path)
        if not normalized:
            result.add_error("file.empty_path", "Generated file path must not be empty.")
            continue
        if normalized in seen:
            result.add_error("file.duplicate_path", f"Duplicate generated file path: {normalized}.")
        seen.add(normalized)

        root = normalized.split("/", 1)[0]
        if root not in ALLOWED_CASE_ROOTS:
            result.add_error("file.disallowed_root", f"Generated file path has unsupported root: {normalized}.")
        if item.format and item.format not in ALLOWED_FILE_FORMATS:
            result.add_warning("file.unknown_format", f"Generated file format is not registered: {item.format}.")
    return result


def _normalize_relative_path(path: str) -> str:
    cleaned = str(path or "").replace("\\", "/").strip().strip("/")
    if not cleaned:
        return ""
    normalized = str(PurePosixPath(cleaned))
    if normalized.startswith("../") or normalized == ".." or normalized.startswith("/"):
        return ""
    return normalized
