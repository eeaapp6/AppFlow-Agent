from dataclasses import asdict, dataclass, field
from typing import Any


@dataclass
class GeometrySpec:
    type: str
    parameters: dict[str, Any] = field(default_factory=dict)
    source: str = ""

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "GeometrySpec":
        parameters = data.get("parameters", {})
        return cls(
            type=str(data.get("type", "")).strip(),
            parameters=parameters if isinstance(parameters, dict) else {},
            source=str(data.get("source", "")).strip(),
        )


@dataclass
class MeshSpec:
    type: str
    parameters: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "MeshSpec":
        parameters = data.get("parameters", {})
        return cls(
            type=str(data.get("type", "")).strip(),
            parameters=parameters if isinstance(parameters, dict) else {},
        )


@dataclass
class BoundarySpec:
    name: str
    role: str
    value: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "BoundarySpec":
        value = data.get("value", {})
        return cls(
            name=str(data.get("name", "")).strip(),
            role=str(data.get("role", "")).strip(),
            value=value if isinstance(value, dict) else {},
        )


@dataclass
class SolverSpec:
    name: str
    case_type: str = ""
    parameters: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "SolverSpec":
        parameters = data.get("parameters", {})
        return cls(
            name=str(data.get("name", "")).strip(),
            case_type=str(data.get("case_type", "")).strip(),
            parameters=parameters if isinstance(parameters, dict) else {},
        )


@dataclass
class SimulationSpec:
    geometry: GeometrySpec
    mesh: MeshSpec
    boundaries: list[BoundarySpec]
    solver: SolverSpec
    generation_mode: str = "reference_modify"

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "SimulationSpec":
        boundaries = data.get("boundaries", [])
        return cls(
            geometry=GeometrySpec.from_dict(data.get("geometry", {}) if isinstance(data.get("geometry"), dict) else {}),
            mesh=MeshSpec.from_dict(data.get("mesh", {}) if isinstance(data.get("mesh"), dict) else {}),
            boundaries=[
                BoundarySpec.from_dict(item)
                for item in boundaries
                if isinstance(item, dict)
            ],
            solver=SolverSpec.from_dict(data.get("solver", {}) if isinstance(data.get("solver"), dict) else {}),
            generation_mode=str(data.get("generation_mode", "reference_modify")).strip() or "reference_modify",
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)
