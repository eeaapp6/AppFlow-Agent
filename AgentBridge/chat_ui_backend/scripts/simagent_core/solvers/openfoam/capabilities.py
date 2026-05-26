from typing import Any

from ...models import ReferenceCase


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


def split_supported_changes(
    reference_case: ReferenceCase | None,
    requested_changes: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    supported_keys = supported_change_keys(reference_case)
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


def summarize_capabilities(
    reference_case: ReferenceCase | None,
    requested_changes: dict[str, Any],
) -> dict[str, Any]:
    supported, unsupported = split_supported_changes(reference_case, requested_changes)
    return {
        "reference_selection": "supported" if reference_case else "fallback",
        "case_generation": "reference_modify" if supported else "reference_copy",
        "supported_changes": sorted(supported),
        "unsupported_changes": sorted(unsupported),
        "run_pipeline": "basic_blockMesh_solver_or_optional_allrun_safe",
    }
