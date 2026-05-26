import os
from dataclasses import dataclass


@dataclass
class Config:
    default_solver_family: str = "openfoam"
    default_output_root: str = "FoamAgentOutputs"
    deepseek_base_url: str = "https://api.deepseek.com"
    deepseek_model: str = "deepseek-v4-flash"
    request_timeout_seconds: int = 90

    @classmethod
    def from_environment(cls) -> "Config":
        return cls(
            default_solver_family=os.environ.get("SIMAGENT_DEFAULT_SOLVER", "openfoam").strip().lower() or "openfoam",
            default_output_root=os.environ.get("SIMAGENT_OUTPUT_ROOT", "FoamAgentOutputs").strip() or "FoamAgentOutputs",
            deepseek_base_url=os.environ.get("PROVIDER_BASE_URL", "https://api.deepseek.com").strip().rstrip("/")
            or "https://api.deepseek.com",
            deepseek_model=os.environ.get("PROVIDER_MODEL", "deepseek-v4-flash").strip() or "deepseek-v4-flash",
        )
