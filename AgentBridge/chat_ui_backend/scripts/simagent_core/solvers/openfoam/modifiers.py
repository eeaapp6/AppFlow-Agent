import re
from dataclasses import dataclass
from typing import Any

from ...models import ReferenceFile


@dataclass(frozen=True)
class NamedBlock:
    start: int
    open_brace: int
    close_brace: int
    end: int


CHANGE_ALIASES = {
    "start_time": ["start_time", "startTime"],
    "end_time": ["end_time", "endTime", "final_time", "finalTime"],
    "delta_t": ["delta_t", "deltaT", "time_step", "timestep"],
    "write_control": ["write_control", "writeControl"],
    "write_interval": ["write_interval", "writeInterval"],
    "write_format": ["write_format", "writeFormat", "output_format"],
    "write_precision": ["write_precision", "writePrecision", "output_precision"],
    "write_compression": ["write_compression", "writeCompression"],
    "purge_write": ["purge_write", "purgeWrite"],
    "time_format": ["time_format", "timeFormat"],
    "time_precision": ["time_precision", "timePrecision"],
    "adjust_time_step": ["adjust_time_step", "adjustTimeStep", "adaptive_time_step"],
    "max_co": ["max_co", "maxCo", "max_courant"],
    "max_delta_t": ["max_delta_t", "maxDeltaT", "max_time_step"],
    "run_time_modifiable": ["run_time_modifiable", "runTimeModifiable"],
    "lid_velocity": ["lid_velocity", "top_velocity", "velocity"],
    "inlet_velocity": ["inlet_velocity", "inletVelocity", "inflow_velocity", "inflowVelocity"],
    "outlet_pressure": ["outlet_pressure", "outletPressure", "pressure_outlet", "pressureOutlet"],
    "kinematic_viscosity": ["kinematic_viscosity", "viscosity", "nu"],
    "density": ["density", "rho"],
}


@dataclass(frozen=True)
class OpenFOAMModifier:
    change_key: str
    file_path: str

    def apply(self, content: str, value: str) -> tuple[str, list[str]]:
        raise NotImplementedError


@dataclass(frozen=True)
class AssignmentModifier(OpenFOAMModifier):
    foam_key: str

    def apply(self, content: str, value: str) -> tuple[str, list[str]]:
        return _replace_openfoam_assignment(content, self.foam_key, value, self.change_key)


@dataclass(frozen=True)
class DimensionedScalarModifier(OpenFOAMModifier):
    foam_key: str

    def apply(self, content: str, value: str) -> tuple[str, list[str]]:
        return _replace_dimensioned_scalar(content, self.foam_key, value, self.change_key)


@dataclass(frozen=True)
class BoundaryUniformValueModifier(OpenFOAMModifier):
    patch_name: str
    field_kind: str

    def apply(self, content: str, value: str) -> tuple[str, list[str]]:
        return _replace_boundary_uniform_value(
            content,
            self.patch_name,
            _format_uniform_value(value, self.field_kind),
            self.change_key,
        )


MODIFIER_REGISTRY: dict[str, OpenFOAMModifier] = {
    "start_time": AssignmentModifier("start_time", "case/system/controlDict", "startTime"),
    "end_time": AssignmentModifier("end_time", "case/system/controlDict", "endTime"),
    "delta_t": AssignmentModifier("delta_t", "case/system/controlDict", "deltaT"),
    "write_control": AssignmentModifier("write_control", "case/system/controlDict", "writeControl"),
    "write_interval": AssignmentModifier("write_interval", "case/system/controlDict", "writeInterval"),
    "write_format": AssignmentModifier("write_format", "case/system/controlDict", "writeFormat"),
    "write_precision": AssignmentModifier("write_precision", "case/system/controlDict", "writePrecision"),
    "write_compression": AssignmentModifier("write_compression", "case/system/controlDict", "writeCompression"),
    "purge_write": AssignmentModifier("purge_write", "case/system/controlDict", "purgeWrite"),
    "time_format": AssignmentModifier("time_format", "case/system/controlDict", "timeFormat"),
    "time_precision": AssignmentModifier("time_precision", "case/system/controlDict", "timePrecision"),
    "adjust_time_step": AssignmentModifier("adjust_time_step", "case/system/controlDict", "adjustTimeStep"),
    "max_co": AssignmentModifier("max_co", "case/system/controlDict", "maxCo"),
    "max_delta_t": AssignmentModifier("max_delta_t", "case/system/controlDict", "maxDeltaT"),
    "run_time_modifiable": AssignmentModifier(
        "run_time_modifiable",
        "case/system/controlDict",
        "runTimeModifiable",
    ),
    "kinematic_viscosity": DimensionedScalarModifier(
        "kinematic_viscosity",
        "case/constant/physicalProperties",
        "nu",
    ),
    "density": DimensionedScalarModifier(
        "density",
        "case/constant/physicalProperties",
        "rho",
    ),
    "lid_velocity": BoundaryUniformValueModifier("lid_velocity", "case/0/U", "movingWall", "vector"),
    "inlet_velocity": BoundaryUniformValueModifier("inlet_velocity", "case/0/U", "inlet", "vector"),
    "outlet_pressure": BoundaryUniformValueModifier("outlet_pressure", "case/0/p", "outlet", "scalar"),
}


def apply_reference_modifications(reference_file: ReferenceFile, changes: dict[str, Any]) -> tuple[str, list[str]]:
    content = reference_file.content
    modifications: list[str] = []

    for change_key, modifier in MODIFIER_REGISTRY.items():
        if modifier.file_path != reference_file.path:
            continue
        content, changed = modifier.apply(content, _change_value(changes, change_key))
        modifications.extend(changed)

    return content, modifications


def merged_requested_changes(plan_parameters: dict[str, Any], requested_changes: dict[str, Any]) -> dict[str, Any]:
    merged = {}
    if isinstance(plan_parameters, dict):
        merged.update(plan_parameters)
    if isinstance(requested_changes, dict):
        merged.update(requested_changes)
    return merged


def _change_value(changes: dict[str, Any], canonical_key: str) -> str:
    for key in CHANGE_ALIASES[canonical_key]:
        value = changes.get(key)
        if value is not None and str(value).strip():
            return str(value).strip()
    return ""


def _replace_openfoam_assignment(content: str, key: str, value: str, change_key: str) -> tuple[str, list[str]]:
    if not value:
        return content, []
    pattern = re.compile(rf"(^\s*{re.escape(key)}\s+)([^;]+)(;)", flags=re.MULTILINE)
    new_content, count = pattern.subn(rf"\g<1>{value}\3", content, count=1)
    return new_content, [change_key] if count else []


def _replace_dimensioned_scalar(content: str, key: str, value: str, change_key: str) -> tuple[str, list[str]]:
    if not value:
        return content, []
    pattern = re.compile(rf"(^\s*{re.escape(key)}\s+\[[^\]]+\]\s+)([^;]+)(;)", flags=re.MULTILINE)
    new_content, count = pattern.subn(rf"\g<1>{value}\3", content, count=1)
    return new_content, [change_key] if count else []


def _replace_boundary_uniform_value(
    content: str,
    patch_name: str,
    value: str,
    change_key: str,
) -> tuple[str, list[str]]:
    if not value:
        return content, []
    boundary_block = _find_named_block(content, "boundaryField")
    if not boundary_block:
        return content, []
    patch_block = _find_named_block(
        content,
        patch_name,
        start=boundary_block.open_brace + 1,
        end=boundary_block.close_brace,
    )
    if not patch_block:
        return content, []

    patch_content = content[patch_block.open_brace + 1:patch_block.close_brace]
    pattern = re.compile(r"(^\s*value\s+uniform\s+)(\([^\)]*\)|[^;]+)(;)", flags=re.MULTILINE)
    match = pattern.search(patch_content)
    if not match or not _is_literal_uniform_value(match.group(2)):
        return content, []
    new_patch_content = pattern.sub(rf"\g<1>{value}\3", patch_content, count=1)
    new_content = (
        content[:patch_block.open_brace + 1]
        + new_patch_content
        + content[patch_block.close_brace:]
    )
    return new_content, [change_key]


def _format_uniform_value(value: str, field_kind: str) -> str:
    if not value:
        return ""
    clean_value = value.strip()
    if field_kind == "vector" and not clean_value.startswith("("):
        return f"({clean_value} 0 0)"
    return clean_value


def _is_literal_uniform_value(value: str) -> bool:
    clean_value = value.strip()
    return bool(clean_value) and not clean_value.startswith("$") and not clean_value.startswith("nonuniform")


def _find_named_block(content: str, name: str, start: int = 0, end: int | None = None) -> NamedBlock | None:
    search_end = len(content) if end is None else end
    pattern = re.compile(rf"(?m)^[ \t]*{re.escape(name)}\s*\{{")
    match = pattern.search(content, start, search_end)
    if not match:
        return None
    open_brace = content.find("{", match.start(), match.end())
    if open_brace < 0 or open_brace >= search_end:
        return None
    close_brace = _find_matching_brace(content, open_brace, search_end)
    if close_brace is None:
        return None
    return NamedBlock(match.start(), open_brace, close_brace, close_brace + 1)


def _find_matching_brace(content: str, open_brace: int, end: int) -> int | None:
    depth = 0
    for index in range(open_brace, end):
        char = content[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
    return None
