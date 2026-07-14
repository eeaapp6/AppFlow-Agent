from ...llm_client import DeepSeekClient
from ...models import PlannedFile, ReferenceCase, SimulationPlan
from ...gates.capability_gate import CapabilityDecision, review_openfoam_capability
from ...gates.spec_gate import review_simulation_spec
from ...router import CaseIntent, RouteDecision, parse_case_intent, select_generation_route
from ...spec import SimulationSpec
from ...spec.from_plan import simulation_spec_from_openfoam_plan
from .capabilities import (
    STATUS_NEEDS_USER_INPUT,
    STATUS_UNSUPPORTED,
    split_supported_changes,
    summarize_capabilities,
)
from .generated import create_rect_channel_plan
from .generated.rect_channel import validate_rect_channel_geometry
from .knowledge import AllrunScriptDatabase, CommandHelpDatabase, OpenFOAMCaseDatabase, TutorialCase
from .knowledge.tutorial_details import normalize_reference_file_path, reference_file_format
from .mesh import GMSH_MESH_CASE_PATH, detect_gmsh_mesh_request, is_block_mesh_dict, plan_uses_external_gmsh_mesh
from .reference_selector import select_reference
from .run_pipeline import parse_allrun_pipeline_preview


FALLBACK_CAVITY_FILES = [
    PlannedFile("mesh", "case/system/blockMeshDict", "openfoam-dict"),
    PlannedFile("control", "case/system/controlDict", "openfoam-dict"),
    PlannedFile("numerics", "case/system/fvSchemes", "openfoam-dict"),
    PlannedFile("linear_solver", "case/system/fvSolution", "openfoam-dict"),
    PlannedFile("physical_properties", "case/constant/physicalProperties", "openfoam-dict"),
    PlannedFile("initial_condition", "case/0/U", "openfoam-dict"),
    PlannedFile("initial_condition", "case/0/p", "openfoam-dict"),
]


def _role_for_openfoam_file(directory: str, file_name: str) -> str:
    if directory == "system":
        if file_name == "blockMeshDict":
            return "mesh"
        if file_name == "controlDict":
            return "control"
        if file_name == "fvSchemes":
            return "numerics"
        if file_name == "fvSolution":
            return "linear_solver"
        return "system"
    if directory == "constant":
        return "physical_properties"
    if directory == "0":
        return "initial_condition"
    return "case_file"


def _planned_files_from_reference(reference_case: TutorialCase | None) -> list[PlannedFile]:
    if not reference_case:
        return list(FALLBACK_CAVITY_FILES)

    planned_files: list[PlannedFile] = []
    for directory, file_names in reference_case.directory_structure.items():
        for file_name in file_names:
            relative_path = normalize_reference_file_path(directory, file_name)
            planned_files.append(
                PlannedFile(
                    role=_role_for_openfoam_file(directory, file_name),
                    path=relative_path,
                    format=reference_file_format(relative_path),
                    required=True,
                )
            )
    return planned_files or list(FALLBACK_CAVITY_FILES)


def _without_block_mesh_dict(planned_files: list[PlannedFile]) -> list[PlannedFile]:
    return [
        item
        for item in planned_files
        if not is_block_mesh_dict(item.path)
    ]


def _apply_mesh_request_to_plan(plan: SimulationPlan, mesh_request: dict) -> None:
    plan.parameters["mesh"] = mesh_request
    if not plan_uses_external_gmsh_mesh(plan.parameters):
        return

    plan.planned_files = _without_block_mesh_dict(plan.planned_files)
    plan.planned_files.append(
        PlannedFile("mesh_source", GMSH_MESH_CASE_PATH, "gmsh-msh", required=True)
    )


def _deduplicate_cases(cases: list[TutorialCase]) -> list[TutorialCase]:
    seen: set[tuple[str, str, str, str]] = set()
    unique_cases: list[TutorialCase] = []
    for case in cases:
        key = (
            case.case_domain.lower(),
            case.case_solver.lower(),
            case.case_category.lower(),
            case.case_name.lower(),
        )
        if key in seen:
            continue
        seen.add(key)
        unique_cases.append(case)
    return unique_cases


def _summarize_reference_case(reference_case: TutorialCase) -> dict:
    return {
        "case_name": reference_case.case_name,
        "domain": reference_case.case_domain,
        "category": reference_case.case_category,
        "solver": reference_case.case_solver,
    }


def _find_reference_candidates(plan: SimulationPlan, limit: int = 5) -> tuple[list[TutorialCase], dict, str]:
    database = OpenFOAMCaseDatabase.from_default_data()
    if not database.is_available():
        return [], {
            "query": {},
        }, "knowledge_unavailable"

    query = {
        "domain": plan.physics_domain.strip(),
        "solver": plan.solver_name.strip(),
        "category": plan.case_category.strip(),
    }
    lookup_steps = [
        ("exact_domain_solver_category", query),
        ("solver_category", {"solver": query["solver"], "category": query["category"]}),
        ("domain_solver", {"domain": query["domain"], "solver": query["solver"]}),
        ("solver", {"solver": query["solver"]}),
        ("domain", {"domain": query["domain"]}),
        ("stable_cavity_fallback", {"domain": "incompressible", "solver": "icoFoam", "category": "cavity"}),
    ]

    lookup_strategy = ""
    candidates: list[TutorialCase] = []
    for strategy, step_query in lookup_steps:
        clean_query = {key: value for key, value in step_query.items() if value}
        if not clean_query:
            continue

        matches = database.find_cases(
            domain=clean_query.get("domain", ""),
            solver=clean_query.get("solver", ""),
            category=clean_query.get("category", ""),
            limit=limit,
        )
        if matches:
            candidates = _deduplicate_cases(matches)
            lookup_strategy = strategy
            break

    return candidates, query, lookup_strategy or "none"


REQUESTED_CHANGE_ALIASES = {
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
REQUESTED_CHANGE_ALIAS_LOOKUP = {
    alias.lower(): canonical_key
    for canonical_key, aliases in REQUESTED_CHANGE_ALIASES.items()
    for alias in aliases
}


def _normalize_requested_changes(requested_changes: dict) -> dict:
    normalized = {}
    source = requested_changes if isinstance(requested_changes, dict) else {}

    for canonical_key, aliases in REQUESTED_CHANGE_ALIASES.items():
        for alias in aliases:
            value = source.get(alias)
            if value is not None and str(value).strip():
                normalized[canonical_key] = str(value).strip()
                break

    if isinstance(requested_changes, dict):
        for key, value in requested_changes.items():
            if value is None or not str(value).strip():
                continue
            if REQUESTED_CHANGE_ALIAS_LOOKUP.get(str(key).strip().lower()):
                continue
            normalized_key = _normalize_requested_change_key(str(key))
            if normalized_key in REQUESTED_CHANGE_ALIASES:
                continue
            normalized[normalized_key] = str(value).strip()
    return normalized


def _normalize_requested_change_key(key: str) -> str:
    return key.strip().replace("-", "_").replace(" ", "_").lower()


def _allrun_metadata(reference_case: ReferenceCase | None, solver_name: str) -> dict:
    allrun_database = AllrunScriptDatabase.from_default_data()
    allrun_reference = allrun_database.summarize_reference(reference_case)
    metadata = {"allrun_reference": allrun_reference}
    if allrun_reference.get("available") and reference_case:
        allowed_commands = CommandHelpDatabase.from_default_data().load_command_names()
        allrun_script = allrun_database.load_script(reference_case)
        metadata["allrun_pipeline_preview"] = parse_allrun_pipeline_preview(
            allrun_script,
            allowed_commands,
            solver_name=solver_name,
        )
    return metadata


def _attach_capability_decision(
    plan: SimulationPlan,
    decision: CapabilityDecision,
    route: RouteDecision,
) -> None:
    plan.parameters["capability_decision"] = decision.to_dict()
    plan.parameters["route_decision"] = route.to_dict()
    gate_reviews = plan.parameters.get("gate_reviews", [])
    if not isinstance(gate_reviews, list):
        gate_reviews = []
    gate_reviews.append(decision.to_gate_review())
    plan.parameters["gate_reviews"] = gate_reviews


def _finalize_generated_case_capability(
    plan: SimulationPlan,
    *,
    intent: CaseIntent,
    mesh_request: dict,
    route: RouteDecision,
) -> CapabilityDecision:
    requested_changes = _normalize_requested_changes(plan.requested_changes)
    geometry_type = intent.geometry_type or plan.case_name
    supported_changes, unsupported_changes = split_supported_changes(
        None,
        requested_changes,
        generation_mode="generated_case",
        geometry_type=geometry_type,
    )
    decision = review_openfoam_capability(
        intent=intent,
        supported_changes=supported_changes,
        unsupported_changes=unsupported_changes,
        mesh_request=mesh_request,
        route_decision=route,
    )

    plan.requested_changes = requested_changes
    plan.parameters["requested_changes"] = requested_changes
    plan.parameters["unsupported_changes"] = unsupported_changes
    plan.parameters["capabilities"] = summarize_capabilities(
        None,
        requested_changes,
        generation_mode="generated_case",
        geometry_type=geometry_type,
        mesh_request=mesh_request,
    )
    _attach_capability_decision(plan, decision, route)
    return decision


def create_openfoam_plan(user_requirement: str, llm: DeepSeekClient | None = None) -> SimulationPlan:
    intent = parse_case_intent(user_requirement)
    if intent.geometry_type == "rect_channel":
        validate_rect_channel_geometry(user_requirement)
    mesh_request = detect_gmsh_mesh_request(user_requirement)
    if intent.explicit_new_geometry:
        route = select_generation_route(intent)
        if route.selected_mode == "unsupported":
            raise ValueError(route.reason)
        plan = create_rect_channel_plan(user_requirement)
        plan.parameters["case_intent"] = intent.to_dict()
        _finalize_generated_case_capability(
            plan,
            intent=intent,
            mesh_request=mesh_request,
            route=route,
        )
        _attach_spec_gate_review(plan, preserve_generated_spec=True)
        return plan

    client = llm or DeepSeekClient()
    messages = [
        {
            "role": "system",
            "content": (
                "You are an OpenFOAM planning assistant. Convert the user requirement into one JSON object. "
                "Do not generate OpenFOAM file contents. Only plan the case. "
                "Return JSON with exactly these keys: solver_family, solver_name, physics_domain, "
                "case_category, case_name, description, generation_mode, requested_changes, parameters, planned_files. "
                "Use generation_mode='reference_modify'. "
                "requested_changes must contain only user-requested modifications to the reference case. "
                "Supported requested_changes keys are: start_time, end_time, delta_t, write_control, "
                "write_interval, write_format, write_precision, write_compression, purge_write, "
                "time_format, time_precision, adjust_time_step, max_co, max_delta_t, "
                "run_time_modifiable, lid_velocity, inlet_velocity, outlet_pressure, "
                "kinematic_viscosity, density. "
                "planned_files must be a list of objects with role, path, format, required. "
                "Use solver_family='openfoam'. Paths must be relative to the task directory and usually start with case/. "
                "Use openfoam-dict as the format for OpenFOAM dictionary files."
            ),
        },
        {
            "role": "user",
            "content": user_requirement,
        },
    ]
    raw_plan = client.chat_json(messages)
    plan = SimulationPlan.from_dict(raw_plan)

    if not plan.solver_family:
        plan.solver_family = "openfoam"
    if plan.solver_family != "openfoam":
        raise ValueError(f"OpenFOAM planner returned unsupported solver_family: {plan.solver_family}")

    reference_candidates, reference_query, lookup_strategy = _find_reference_candidates(plan, limit=5)
    reference_case, reference_selection = select_reference(
        plan,
        reference_candidates,
        query=reference_query,
        lookup_strategy=lookup_strategy,
        user_requirement=user_requirement,
    )
    provisional_changes = _normalize_requested_changes(plan.requested_changes)
    provisional_reference_case = (
        ReferenceCase.from_dict(reference_case.to_dict())
        if reference_case
        else None
    )
    supported_changes, unsupported_changes = split_supported_changes(provisional_reference_case, provisional_changes)
    route = select_generation_route(
        intent,
        reference_score=_selected_reference_score(reference_selection),
    )
    decision = review_openfoam_capability(
        intent=intent,
        reference_case=provisional_reference_case,
        reference_selection=reference_selection,
        supported_changes=supported_changes,
        unsupported_changes=unsupported_changes,
        mesh_request=mesh_request,
        route_decision=route,
    )
    if decision.mode == "generated_case":
        generated_plan = create_rect_channel_plan(user_requirement)
        generated_plan.parameters["case_intent"] = intent.to_dict()
        _finalize_generated_case_capability(
            generated_plan,
            intent=intent,
            mesh_request=mesh_request,
            route=route,
        )
        generated_plan.parameters["reference_selection"] = reference_selection
        generated_plan.parameters["reference_candidates"] = [
            _summarize_reference_case(item) for item in reference_candidates
        ]
        _attach_spec_gate_review(generated_plan, preserve_generated_spec=True)
        return generated_plan
    if decision.support_status in {STATUS_UNSUPPORTED, STATUS_NEEDS_USER_INPUT}:
        raise ValueError(decision.reason)

    plan.case_name = reference_case.case_name if reference_case else "cavity"
    plan.solver_name = reference_case.case_solver if reference_case else "icoFoam"
    plan.physics_domain = reference_case.case_domain if reference_case else "incompressible"
    plan.case_category = reference_case.case_category if reference_case else "cavity"
    if not plan.description:
        plan.description = user_requirement
    plan.planned_files = _planned_files_from_reference(reference_case)
    _apply_mesh_request_to_plan(plan, mesh_request)
    if reference_case:
        plan.reference_case = provisional_reference_case
    requested_changes = provisional_changes
    plan.requested_changes = supported_changes
    plan.generation_mode = decision.mode
    template_solver = (plan.solver_name or "openfoam").lower()
    template_case = (plan.case_name or "reference").lower()
    plan.parameters["case_template"] = f"{template_solver}_{template_case}"
    plan.parameters["reference_candidates"] = [
        _summarize_reference_case(item) for item in reference_candidates
    ]
    plan.parameters["reference_selection"] = reference_selection
    plan.parameters.update(_allrun_metadata(plan.reference_case, plan.solver_name))
    plan.parameters["generation_mode"] = plan.generation_mode
    plan.parameters["case_intent"] = intent.to_dict()
    _attach_capability_decision(plan, decision, route)
    plan.parameters["requested_changes"] = supported_changes
    plan.parameters["unsupported_changes"] = unsupported_changes
    plan.parameters["capabilities"] = summarize_capabilities(
        plan.reference_case,
        requested_changes,
        generation_mode=plan.generation_mode,
        geometry_type=decision.geometry_type,
        mesh_request=mesh_request,
    )
    _attach_spec_gate_review(plan)

    return plan


def _selected_reference_score(reference_selection: dict) -> int:
    try:
        return int(reference_selection.get("selected_score", 0) or 0)
    except (AttributeError, TypeError, ValueError):
        return 0


def _attach_spec_gate_review(plan: SimulationPlan, *, preserve_generated_spec: bool = False) -> None:
    existing_spec = plan.parameters.get("simulation_spec", {})
    can_preserve_generated_spec = (
        preserve_generated_spec
        and plan.generation_mode == "generated_case"
        and plan.case_name == "rect_channel"
        and isinstance(existing_spec, dict)
        and bool(existing_spec)
    )
    spec = (
        SimulationSpec.from_dict(existing_spec)
        if can_preserve_generated_spec
        else simulation_spec_from_openfoam_plan(plan)
    )
    gate_review = review_simulation_spec(spec)
    gate_reviews = plan.parameters.get("gate_reviews", [])
    if not isinstance(gate_reviews, list):
        gate_reviews = []
    gate_reviews.append(gate_review.to_dict())
    plan.parameters["simulation_spec"] = spec.to_dict()
    plan.parameters["gate_reviews"] = gate_reviews
