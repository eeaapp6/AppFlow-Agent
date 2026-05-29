from dataclasses import asdict, dataclass, field
from typing import Any


@dataclass
class GeometrySpec:
    type: str
    parameters: dict[str, Any] = field(default_factory=dict)
    source: str = ""

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "GeometrySpec":
        data = _dict_data(data)
        return cls(
            type=_string(data.get("type", "")),
            parameters=_dict_value(data, "parameters"),
            source=_string(data.get("source", "")),
        )


@dataclass
class MeshSpec:
    type: str
    parameters: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "MeshSpec":
        data = _dict_data(data)
        return cls(
            type=_string(data.get("type", "")),
            parameters=_dict_value(data, "parameters"),
        )


@dataclass
class BoundarySpec:
    name: str
    role: str
    value: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "BoundarySpec":
        data = _dict_data(data)
        return cls(
            name=_string(data.get("name", "")),
            role=_string(data.get("role", "")),
            value=_dict_value(data, "value"),
        )


@dataclass
class SolverSpec:
    name: str
    case_type: str = ""
    parameters: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "SolverSpec":
        data = _dict_data(data)
        return cls(
            name=_string(data.get("name", "")),
            case_type=_string(data.get("case_type", "")),
            parameters=_dict_value(data, "parameters"),
        )


@dataclass
class PhysicsSpec:
    domain: str = ""
    model: str = ""
    properties: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "PhysicsSpec":
        data = _dict_data(data)
        return cls(
            domain=_string(data.get("domain", "")),
            model=_string(data.get("model", "")),
            properties=_dict_value(data, "properties"),
        )


@dataclass
class NumericsSpec:
    algorithm: str = ""
    time: dict[str, Any] = field(default_factory=dict)
    schemes: dict[str, Any] = field(default_factory=dict)
    linear_solvers: dict[str, Any] = field(default_factory=dict)
    controls: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "NumericsSpec":
        data = _dict_data(data)
        return cls(
            algorithm=_string(data.get("algorithm", "")),
            time=_dict_value(data, "time"),
            schemes=_dict_value(data, "schemes"),
            linear_solvers=_dict_value(data, "linear_solvers"),
            controls=_dict_value(data, "controls"),
        )


@dataclass
class OutputsSpec:
    format: str = "openfoam"
    result_format: str = ""
    fields: list[str] = field(default_factory=list)
    controls: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "OutputsSpec":
        data = _dict_data(data)
        return cls(
            format=_string(data.get("format", "openfoam")) or "openfoam",
            result_format=_string(data.get("result_format", "")),
            fields=_string_list(data.get("fields", [])),
            controls=_dict_value(data, "controls"),
        )


@dataclass
class SimulationSpec:
    geometry: GeometrySpec
    mesh: MeshSpec
    boundaries: list[BoundarySpec]
    solver: SolverSpec
    generation_mode: str = "reference_modify"
    physics: PhysicsSpec = field(default_factory=PhysicsSpec)
    numerics: NumericsSpec = field(default_factory=NumericsSpec)
    outputs: OutputsSpec = field(default_factory=OutputsSpec)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "SimulationSpec":
        data = _dict_data(data)
        boundaries = data.get("boundaries", [])
        return cls(
            geometry=GeometrySpec.from_dict(_dict_value(data, "geometry")),
            mesh=MeshSpec.from_dict(_dict_value(data, "mesh")),
            boundaries=[
                BoundarySpec.from_dict(item)
                for item in boundaries
                if isinstance(item, dict)
            ],
            solver=SolverSpec.from_dict(_dict_value(data, "solver")),
            generation_mode=_string(data.get("generation_mode", "reference_modify")) or "reference_modify",
            physics=PhysicsSpec.from_dict(_dict_value(data, "physics")),
            numerics=NumericsSpec.from_dict(_dict_value(data, "numerics")),
            outputs=OutputsSpec.from_dict(_dict_value(data, "outputs")),
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


def _dict_value(data: dict[str, Any], key: str) -> dict[str, Any]:
    value = data.get(key, {})
    return value if isinstance(value, dict) else {}


def _dict_data(data: Any) -> dict[str, Any]:
    return data if isinstance(data, dict) else {}


def _string(value: Any) -> str:
    return str(value).strip()


def _string_list(value: Any) -> list[str]:
    if not isinstance(value, list):
        return []
    items = []
    for item in value:
        if item is None:
            continue
        text = _string(item)
        if text:
            items.append(text)
    return items
