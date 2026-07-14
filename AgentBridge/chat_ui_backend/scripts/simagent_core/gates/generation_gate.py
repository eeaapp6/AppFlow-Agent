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


def review_generated_files(
    files: list[PlannedFile],
    planned_files: list[PlannedFile] | None = None,
) -> GateResult:
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

    if planned_files is not None:
        missing_required: set[str] = set()
        for item in planned_files:
            if not item.required:
                continue
            normalized = _normalize_relative_path(item.path)
            if normalized and normalized not in seen:
                missing_required.add(normalized)
        for path in sorted(missing_required):
            result.add_error(
                "generation.missing_required_file",
                f"Required planned file was not generated: {path}.",
            )
    return result


def _normalize_relative_path(path: str) -> str:
    cleaned = str(path or "").replace("\\", "/").strip()
    if not cleaned:
        return ""
    candidate = PurePosixPath(cleaned)
    if candidate.is_absolute() or ".." in candidate.parts:
        return ""
    return str(candidate)
