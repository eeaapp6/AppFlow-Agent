from dataclasses import asdict, dataclass, field
from typing import Any

from ...models import ReferenceCase


STATUS_SUPPORTED = "supported"
STATUS_PARTIALLY_SUPPORTED = "partially_supported"
STATUS_UNSUPPORTED = "unsupported"
STATUS_NEEDS_USER_INPUT = "needs_user_input"

GLOBAL_MODIFIERS = {
    "start_time",
    "end_time",
    "delta_t",
    "write_control",
    "write_interval",
    "write_format",
    "write_precision",
    "write_compression",
    "purge_write",
    "time_format",
    "time_precision",
    "adjust_time_step",
    "max_co",
    "max_delta_t",
    "run_time_modifiable",
}

DOMAIN_MODIFIERS = {
    "incompressible": {
        "density",
        "kinematic_viscosity",
    },
}

SOLVER_MODIFIERS = {
    "icofoam": set(),
}

CASE_MODIFIERS = {
    ("icofoam", "cavity"): {
        "lid_velocity",
    },
}

CASE_NAME_MODIFIERS = {
    ("simplefoam", "pitzdaily"): {
        "inlet_velocity",
        "outlet_pressure",
    },
    ("pimplefoam", "pitzdaily"): {
        "inlet_velocity",
        "outlet_pressure",
    },
}

GENERATED_RECT_CHANNEL_MODIFIERS = {
    "end_time",
    "delta_t",
    "write_interval",
    "kinematic_viscosity",
    "inlet_velocity",
    "outlet_pressure",
}

STATUS_PRIORITY = {
    STATUS_SUPPORTED: 0,
    STATUS_PARTIALLY_SUPPORTED: 1,
    STATUS_NEEDS_USER_INPUT: 2,
    STATUS_UNSUPPORTED: 3,
}


@dataclass
class CapabilityMatrixEntry:
    capability: str
    status: str
    reason: str
    supported: list[str] = field(default_factory=list)
    unsupported: list[str] = field(default_factory=list)
    needs_user_input: list[str] = field(default_factory=list)

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass
class CapabilityReport:
    status: str
    entries: list[CapabilityMatrixEntry]
    supported_changes: dict[str, Any] = field(default_factory=dict)
    unsupported_changes: dict[str, Any] = field(default_factory=dict)
    needs_user_input: list[str] = field(default_factory=list)

    def to_dict(self) -> dict[str, Any]:
        return {
            "status": self.status,
            "entries": [entry.to_dict() for entry in self.entries],
            "supported_changes": sorted(self.supported_changes),
            "unsupported_changes": sorted(self.unsupported_changes),
            "needs_user_input": list(self.needs_user_input),
        }


def _normalize(value: str) -> str:
    return str(value or "").strip().lower()


def capability_key(reference_case: ReferenceCase | None) -> tuple[str, str]:
    if not reference_case:
        return "", ""
    return _normalize(reference_case.case_solver), _normalize(reference_case.case_category)


def supported_change_keys(reference_case: ReferenceCase | None) -> set[str]:
    if not reference_case:
        return set()

    solver = _normalize(reference_case.case_solver)
    domain = _normalize(reference_case.case_domain)
    category = _normalize(reference_case.case_category)
    case_name = _normalize(reference_case.case_name)
    supported = set(GLOBAL_MODIFIERS)
    supported.update(DOMAIN_MODIFIERS.get(domain, set()))
    supported.update(SOLVER_MODIFIERS.get(solver, set()))
    supported.update(CASE_MODIFIERS.get((solver, category), set()))
    supported.update(CASE_NAME_MODIFIERS.get((solver, case_name), set()))
    return supported


def generated_change_keys(geometry_type: str) -> set[str]:
    if _normalize(geometry_type) == "rect_channel":
        return set(GENERATED_RECT_CHANNEL_MODIFIERS)
    return set()


def split_supported_changes(
    reference_case: ReferenceCase | None,
    requested_changes: dict[str, Any],
    *,
    generation_mode: str = "",
    geometry_type: str = "",
) -> tuple[dict[str, Any], dict[str, Any]]:
    supported_keys = (
        generated_change_keys(geometry_type)
        if generation_mode == "generated_case"
        else supported_change_keys(reference_case)
    )
    supported: dict[str, Any] = {}
    unsupported: dict[str, Any] = {}
    for key, value in requested_changes.items():
        if key in supported_keys:
            supported[key] = value
        else:
            unsupported[key] = value
    return supported, unsupported


def generation_mode_for(
    reference_case: ReferenceCase | None,
    requested_changes: dict[str, Any],
) -> str:
    supported, _ = split_supported_changes(reference_case, requested_changes)
    return "reference_modify" if supported else "reference_copy"


def evaluate_capability_matrix(
    *,
    reference_case: ReferenceCase | None = None,
    requested_changes: dict[str, Any] | None = None,
    generation_mode: str = "",
    geometry_type: str = "",
    mesh_request: dict[str, Any] | None = None,
) -> CapabilityReport:
    changes = requested_changes if isinstance(requested_changes, dict) else {}
    supported_changes, unsupported_changes = split_supported_changes(
        reference_case,
        changes,
        generation_mode=generation_mode,
        geometry_type=geometry_type,
    )
    entries = [
        _case_generation_entry(
            reference_case,
            generation_mode,
            geometry_type,
            changes,
            supported_changes,
            unsupported_changes,
        ),
        _mesh_entry(mesh_request),
    ]
    entries.append(
        _requested_changes_entry(
            changes,
            supported_changes,
            unsupported_changes,
            generation_mode=generation_mode,
            geometry_type=geometry_type,
        )
    )

    needs_user_input = _collect_needs_user_input(entries)
    status = _overall_status(entries, changes, supported_changes, unsupported_changes)
    return CapabilityReport(
        status=status,
        entries=entries,
        supported_changes=supported_changes,
        unsupported_changes=unsupported_changes,
        needs_user_input=needs_user_input,
    )


def _case_generation_entry(
    reference_case: ReferenceCase | None,
    generation_mode: str,
    geometry_type: str,
    requested_changes: dict[str, Any],
    supported_changes: dict[str, Any],
    unsupported_changes: dict[str, Any],
) -> CapabilityMatrixEntry:
    if generation_mode == "generated_case":
        if _normalize(geometry_type) == "rect_channel":
            return CapabilityMatrixEntry(
                "generated_rect_channel",
                STATUS_SUPPORTED,
                "Generated rectangular channel cases are supported with blockMesh and icoFoam.",
                supported=["rect_channel", "blockMesh", "icoFoam"],
            )
        return CapabilityMatrixEntry(
            "generated_geometry",
            STATUS_UNSUPPORTED,
            "Only generated rect_channel geometry is supported.",
            unsupported=[geometry_type or "unknown_geometry"],
        )

    if not reference_case:
        return CapabilityMatrixEntry(
            "reference_case",
            STATUS_UNSUPPORTED,
            "No OpenFOAM reference case was selected.",
            unsupported=["reference_case"],
        )

    solver = _normalize(reference_case.case_solver)
    category = _normalize(reference_case.case_category)
    case_name = _normalize(reference_case.case_name)
    if not requested_changes:
        return CapabilityMatrixEntry(
            "reference_copy",
            STATUS_SUPPORTED,
            f"Copying selected tutorial reference {case_name or 'unknown'} is supported.",
            supported=[reference_case.case_solver, reference_case.case_name],
        )
    if supported_changes and unsupported_changes:
        return CapabilityMatrixEntry(
            "reference_modify",
            STATUS_PARTIALLY_SUPPORTED,
            "Only explicitly registered modifiers can be applied to the selected tutorial reference.",
            supported=sorted(supported_changes),
            unsupported=sorted(unsupported_changes),
        )
    if unsupported_changes:
        return CapabilityMatrixEntry(
            "reference_modify",
            STATUS_UNSUPPORTED,
            "No registered modifier can apply the requested changes to the selected tutorial reference.",
            unsupported=sorted(unsupported_changes),
        )
    if solver == "icofoam" and category == "cavity":
        return CapabilityMatrixEntry(
            "icoFoam_cavity",
            STATUS_SUPPORTED,
            "icoFoam cavity reference modification is supported for the requested changes.",
            supported=sorted(supported_changes),
        )

    return CapabilityMatrixEntry(
        "reference_modify",
        STATUS_SUPPORTED,
        f"Registered modifiers support the requested changes for selected tutorial case {case_name or 'unknown'}.",
        supported=sorted(supported_changes),
    )


def _mesh_entry(mesh_request: dict[str, Any] | None) -> CapabilityMatrixEntry:
    mesh = mesh_request if isinstance(mesh_request, dict) else {}
    mode = _normalize(mesh.get("mode", "reference_mesh")) or "reference_mesh"
    if mode == "gmsh_external":
        if str(mesh.get("source_path", "")).strip():
            return CapabilityMatrixEntry(
                "external_gmsh_mesh",
                STATUS_SUPPORTED,
                "External GMSH .msh import is supported when a source path is provided.",
                supported=["gmsh_external"],
            )
        return CapabilityMatrixEntry(
            "external_gmsh_mesh",
            STATUS_NEEDS_USER_INPUT,
            "External GMSH mesh import requires a .msh source path.",
            needs_user_input=["mesh.source_path"],
        )

    if mode in {"reference_mesh", "blockmesh", "gmsh"}:
        return CapabilityMatrixEntry(
            "mesh",
            STATUS_SUPPORTED,
            f"Mesh mode {mode} is supported.",
            supported=[mode],
        )

    return CapabilityMatrixEntry(
        "mesh",
        STATUS_UNSUPPORTED,
        f"Mesh mode {mode} is not supported.",
        unsupported=[mode],
    )


def _requested_changes_entry(
    requested_changes: dict[str, Any],
    supported_changes: dict[str, Any],
    unsupported_changes: dict[str, Any],
    *,
    generation_mode: str,
    geometry_type: str,
) -> CapabilityMatrixEntry:
    if not requested_changes:
        return CapabilityMatrixEntry(
            "requested_changes",
            STATUS_SUPPORTED,
            "No parameter modifications were requested.",
        )
    if supported_changes and unsupported_changes:
        return CapabilityMatrixEntry(
            "requested_changes",
            STATUS_PARTIALLY_SUPPORTED,
            "Some requested modifications are supported; unsupported ones must not be applied silently.",
            supported=sorted(supported_changes),
            unsupported=sorted(unsupported_changes),
        )
    if unsupported_changes:
        return CapabilityMatrixEntry(
            "requested_changes",
            STATUS_UNSUPPORTED,
            "None of the requested modifications are supported by this capability matrix.",
            unsupported=sorted(unsupported_changes),
        )
    return CapabilityMatrixEntry(
        "requested_changes",
        STATUS_SUPPORTED,
        _requested_changes_reason(generation_mode, geometry_type),
        supported=sorted(supported_changes),
    )


def _requested_changes_reason(generation_mode: str, geometry_type: str) -> str:
    if generation_mode == "generated_case":
        return f"Requested changes are supported for generated {geometry_type or 'case'}."
    return "Requested changes are supported for the selected reference case."


def _collect_needs_user_input(entries: list[CapabilityMatrixEntry]) -> list[str]:
    needs: list[str] = []
    for entry in entries:
        needs.extend(entry.needs_user_input)
    return sorted(set(needs))


def _overall_status(
    entries: list[CapabilityMatrixEntry],
    requested_changes: dict[str, Any],
    supported_changes: dict[str, Any],
    unsupported_changes: dict[str, Any],
) -> str:
    if any(entry.status == STATUS_NEEDS_USER_INPUT for entry in entries):
        return STATUS_NEEDS_USER_INPUT
    if any(entry.status == STATUS_UNSUPPORTED for entry in entries if entry.capability != "requested_changes"):
        return STATUS_UNSUPPORTED
    if unsupported_changes and supported_changes:
        return STATUS_PARTIALLY_SUPPORTED
    if unsupported_changes and requested_changes:
        return STATUS_UNSUPPORTED
    return max((entry.status for entry in entries), key=lambda status: STATUS_PRIORITY[status])


def summarize_capabilities(
    reference_case: ReferenceCase | None,
    requested_changes: dict[str, Any],
    *,
    generation_mode: str = "",
    geometry_type: str = "",
    mesh_request: dict[str, Any] | None = None,
) -> dict[str, Any]:
    report = evaluate_capability_matrix(
        reference_case=reference_case,
        requested_changes=requested_changes,
        generation_mode=generation_mode,
        geometry_type=geometry_type,
        mesh_request=mesh_request,
    )
    data = report.to_dict()
    data.update({
        "reference_selection": _reference_selection_status(reference_case, generation_mode),
        "case_generation": generation_mode or ("reference_modify" if report.supported_changes else "reference_copy"),
        "run_pipeline": "basic_blockMesh_solver_or_optional_allrun_safe",
    })
    return data


def _reference_selection_status(reference_case: ReferenceCase | None, generation_mode: str) -> str:
    if generation_mode == "generated_case":
        return "generated"
    return "supported" if reference_case else "fallback"
