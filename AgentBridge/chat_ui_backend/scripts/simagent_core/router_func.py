from .config import Config


def select_solver_family(user_requirement: str, config: Config | None = None) -> str:
    text = user_requirement.lower()
    if "phenglei" in text or "pheng lei" in text:
        return "phenglei"
    return (config or Config.from_environment()).default_solver_family
