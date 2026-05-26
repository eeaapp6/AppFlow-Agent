from ...models import SimulationPlan
from .knowledge import TutorialCase


def _normalize(value: str) -> str:
    return str(value or "").strip().lower()


def _required_directories_present(reference_case: TutorialCase) -> bool:
    directories = {_normalize(item) for item in reference_case.directory_structure}
    return {"0", "system", "constant"}.issubset(directories)


def _score_reference(
    plan: SimulationPlan,
    reference_case: TutorialCase,
    user_requirement: str,
) -> tuple[int, list[str]]:
    score = 0
    reasons: list[str] = []

    plan_solver = _normalize(plan.solver_name)
    plan_domain = _normalize(plan.physics_domain)
    plan_category = _normalize(plan.case_category)
    case_solver = _normalize(reference_case.case_solver)
    case_domain = _normalize(reference_case.case_domain)
    case_category = _normalize(reference_case.case_category)

    if plan_solver and plan_solver == case_solver:
        score += 40
        reasons.append("solver exact match")

    if plan_domain and plan_domain == case_domain:
        score += 25
        reasons.append("domain exact match")

    if plan_category and plan_category == case_category:
        score += 15
        reasons.append("category exact match")

    requirement_text = _normalize(user_requirement)
    case_name = _normalize(reference_case.case_name)
    if case_name and case_name in requirement_text:
        score += 10
        reasons.append("case name mentioned")

    if _required_directories_present(reference_case):
        score += 10
        reasons.append("required directories present")

    return score, reasons


def select_reference(
    plan: SimulationPlan,
    candidates: list[TutorialCase],
    *,
    query: dict,
    lookup_strategy: str,
    user_requirement: str,
) -> tuple[TutorialCase | None, dict]:
    if not candidates:
        return None, {
            "query": query,
            "selected_strategy": "no_candidate",
            "selected_case": "fallback_cavity",
            "candidate_count": 0,
            "selected_score": 0,
            "score_reasons": [],
        }

    plan_solver = _normalize(plan.solver_name)
    solver_matched_candidates = [
        item for item in candidates if plan_solver and _normalize(item.case_solver) == plan_solver
    ]
    scoring_pool = solver_matched_candidates or candidates
    selected_strategy = "rule_score_solver_filtered" if solver_matched_candidates else "rule_score_fallback"

    scored: list[tuple[int, list[str], TutorialCase]] = []
    for candidate in scoring_pool:
        score, reasons = _score_reference(plan, candidate, user_requirement)
        scored.append((score, reasons, candidate))

    scored.sort(key=lambda item: (-item[0], item[2].case_name.lower(), item[2].case_solver.lower()))
    selected_score, score_reasons, selected_case = scored[0]
    return selected_case, {
        "query": query,
        "lookup_strategy": lookup_strategy,
        "selected_strategy": selected_strategy,
        "selected_case": selected_case.case_name,
        "candidate_count": len(candidates),
        "selected_score": selected_score,
        "score_reasons": score_reasons,
    }
