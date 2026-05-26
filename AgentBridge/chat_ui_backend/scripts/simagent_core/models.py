from dataclasses import asdict, dataclass, field
from typing import Any


@dataclass
class PlannedFile:
    role: str
    path: str
    format: str
    required: bool = True
    source: str = ""
    modifications: list[str] = field(default_factory=list)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "PlannedFile":
        return cls(
            role=str(data.get("role", "")).strip(),
            path=str(data.get("path", "")).strip(),
            format=str(data.get("format", "")).strip(),
            required=bool(data.get("required", True)),
            source=str(data.get("source", "")).strip(),
            modifications=[str(item) for item in data.get("modifications", [])]
            if isinstance(data.get("modifications", []), list)
            else [],
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass
class ReferenceCase:
    case_name: str
    case_domain: str
    case_category: str
    case_solver: str
    directory_structure: dict[str, list[str]] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "ReferenceCase":
        directory_structure = data.get("directory_structure", {})
        if not isinstance(directory_structure, dict):
            directory_structure = {}

        normalized_structure: dict[str, list[str]] = {}
        for directory, file_names in directory_structure.items():
            if isinstance(file_names, list):
                normalized_structure[str(directory)] = [str(item) for item in file_names]

        return cls(
            case_name=str(data.get("case_name", "")).strip(),
            case_domain=str(data.get("case_domain", "")).strip(),
            case_category=str(data.get("case_category", "")).strip(),
            case_solver=str(data.get("case_solver", "")).strip(),
            directory_structure=normalized_structure,
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass
class ReferenceFile:
    path: str
    role: str
    format: str
    content: str

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "ReferenceFile":
        return cls(
            path=str(data.get("path", "")).strip(),
            role=str(data.get("role", "")).strip(),
            format=str(data.get("format", "")).strip(),
            content=str(data.get("content", "")),
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass
class ReferenceFileSet:
    reference_case: ReferenceCase
    files: list[ReferenceFile] = field(default_factory=list)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "ReferenceFileSet":
        reference_case_data = data.get("reference_case", {})
        files = [
            ReferenceFile.from_dict(item)
            for item in data.get("files", [])
            if isinstance(item, dict)
        ]
        return cls(
            reference_case=ReferenceCase.from_dict(reference_case_data if isinstance(reference_case_data, dict) else {}),
            files=files,
        )

    def to_dict(self) -> dict[str, Any]:
        return {
            "reference_case": self.reference_case.to_dict(),
            "files": [item.to_dict() for item in self.files],
        }


@dataclass
class GeneratedFile:
    path: str
    role: str
    format: str
    source: str = ""
    modifications: list[str] = field(default_factory=list)
    required: bool = True

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "GeneratedFile":
        modifications = data.get("modifications", [])
        return cls(
            path=str(data.get("path", "")).strip(),
            role=str(data.get("role", "")).strip(),
            format=str(data.get("format", "")).strip(),
            source=str(data.get("source", "")).strip(),
            modifications=[str(item) for item in modifications] if isinstance(modifications, list) else [],
            required=bool(data.get("required", True)),
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass
class GeneratedFileSet:
    files: list[GeneratedFile] = field(default_factory=list)

    @classmethod
    def from_list(cls, items: list[dict[str, Any]]) -> "GeneratedFileSet":
        return cls(
            files=[
                GeneratedFile.from_dict(item)
                for item in items
                if isinstance(item, dict)
            ]
        )

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "GeneratedFileSet":
        return cls(
            files=[
                GeneratedFile.from_dict(item)
                for item in data.get("files", [])
                if isinstance(item, dict)
            ]
        )

    def to_dict(self) -> dict[str, Any]:
        return {"files": [item.to_dict() for item in self.files]}

    def to_list(self) -> list[dict[str, Any]]:
        return [item.to_dict() for item in self.files]


@dataclass
class ValidationResult:
    status: str
    checked_files: list[str] = field(default_factory=list)
    missing_files: list[str] = field(default_factory=list)
    dictionary_errors: list[dict[str, Any]] = field(default_factory=list)
    mesh_patches: list[str] = field(default_factory=list)
    field_patches: dict[str, list[str]] = field(default_factory=dict)
    missing_boundary_fields: dict[str, list[str]] = field(default_factory=dict)
    extra_boundary_fields: dict[str, list[str]] = field(default_factory=dict)
    warnings: list[str] = field(default_factory=list)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "ValidationResult":
        return cls(
            status=str(data.get("status", "")).strip(),
            checked_files=[str(item) for item in data.get("checked_files", [])]
            if isinstance(data.get("checked_files", []), list)
            else [],
            missing_files=[str(item) for item in data.get("missing_files", [])]
            if isinstance(data.get("missing_files", []), list)
            else [],
            dictionary_errors=data.get("dictionary_errors", [])
            if isinstance(data.get("dictionary_errors", []), list)
            else [],
            mesh_patches=[str(item) for item in data.get("mesh_patches", [])]
            if isinstance(data.get("mesh_patches", []), list)
            else [],
            field_patches=_dict_of_string_lists(data.get("field_patches", {})),
            missing_boundary_fields=_dict_of_string_lists(data.get("missing_boundary_fields", {})),
            extra_boundary_fields=_dict_of_string_lists(data.get("extra_boundary_fields", {})),
            warnings=[str(item) for item in data.get("warnings", [])]
            if isinstance(data.get("warnings", []), list)
            else [],
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass
class RunResult:
    status: str
    reason: str = ""
    steps: list[dict[str, Any]] = field(default_factory=list)
    pipeline: list[dict[str, Any]] = field(default_factory=list)
    pipeline_info: dict[str, Any] = field(default_factory=dict)
    logs: list[dict[str, Any]] = field(default_factory=list)
    outputs: dict[str, Any] = field(default_factory=dict)
    wm_project_dir: str = ""
    missing_files: list[str] = field(default_factory=list)
    manifest_validation: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "RunResult":
        return cls(
            status=str(data.get("status", "")).strip(),
            reason=str(data.get("reason", "")).strip(),
            steps=data.get("steps", []) if isinstance(data.get("steps", []), list) else [],
            pipeline=data.get("pipeline", []) if isinstance(data.get("pipeline", []), list) else [],
            pipeline_info=data.get("pipeline_info", {}) if isinstance(data.get("pipeline_info", {}), dict) else {},
            logs=data.get("logs", []) if isinstance(data.get("logs", []), list) else [],
            outputs=data.get("outputs", {}) if isinstance(data.get("outputs", {}), dict) else {},
            wm_project_dir=str(data.get("wm_project_dir", "")).strip(),
            missing_files=[str(item) for item in data.get("missing_files", [])]
            if isinstance(data.get("missing_files", []), list)
            else [],
            manifest_validation=data.get("manifest_validation", {})
            if isinstance(data.get("manifest_validation", {}), dict)
            else {},
        )

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass
class SimulationPlan:
    solver_family: str
    solver_name: str
    physics_domain: str
    case_category: str
    case_name: str
    description: str
    generation_mode: str = "reference_modify"
    reference_case: ReferenceCase | None = None
    requested_changes: dict[str, Any] = field(default_factory=dict)
    unsupported_changes: dict[str, Any] = field(default_factory=dict)
    reference_candidates: list[dict[str, Any]] = field(default_factory=list)
    reference_selection: dict[str, Any] = field(default_factory=dict)
    capabilities: dict[str, Any] = field(default_factory=dict)
    allrun_metadata: dict[str, Any] = field(default_factory=dict)
    parameters: dict[str, Any] = field(default_factory=dict)
    planned_files: list[PlannedFile] = field(default_factory=list)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "SimulationPlan":
        planned_files = [
            PlannedFile.from_dict(item)
            for item in data.get("planned_files", [])
            if isinstance(item, dict)
        ]
        parameters = data.get("parameters", {}) if isinstance(data.get("parameters", {}), dict) else {}
        reference_case_data = data.get("reference_case")
        if not isinstance(reference_case_data, dict):
            reference_case_data = parameters.get("reference_case", {})
        reference_case = (
            ReferenceCase.from_dict(reference_case_data)
            if isinstance(reference_case_data, dict) and reference_case_data
            else None
        )
        return cls(
            solver_family=str(data.get("solver_family", "")).strip().lower(),
            solver_name=str(data.get("solver_name", "")).strip(),
            physics_domain=str(data.get("physics_domain", "")).strip(),
            case_category=str(data.get("case_category", "")).strip(),
            case_name=str(data.get("case_name", "")).strip(),
            description=str(data.get("description", "")).strip(),
            generation_mode=str(data.get("generation_mode", "reference_modify")).strip() or "reference_modify",
            reference_case=reference_case,
            requested_changes=data.get("requested_changes", {})
            if isinstance(data.get("requested_changes", {}), dict)
            else {},
            unsupported_changes=_dict_with_fallback(data, parameters, "unsupported_changes"),
            reference_candidates=_list_with_fallback(data, parameters, "reference_candidates"),
            reference_selection=_dict_with_fallback(data, parameters, "reference_selection"),
            capabilities=_dict_with_fallback(data, parameters, "capabilities"),
            allrun_metadata=_allrun_metadata_from_dict(data, parameters),
            parameters=parameters,
            planned_files=planned_files,
        )

    def to_dict(self) -> dict[str, Any]:
        data = asdict(self)
        data["planned_files"] = [item.to_dict() for item in self.planned_files]
        data["reference_case"] = self.reference_case.to_dict() if self.reference_case else None
        return data


@dataclass
class TaskContext:
    version: int
    task_id: str
    status: str
    user_requirement: str
    solver_family: str
    output_root: str
    host_output_root: str
    task_dir: str
    case_dir: str
    mesh_dir: str
    results_dir: str
    logs_dir: str
    context_path: str
    manifest_path: str
    reference_files_path: str = ""
    plan: dict[str, Any] = field(default_factory=dict)
    generated_files: list[dict[str, Any]] = field(default_factory=list)
    validation: dict[str, Any] = field(default_factory=dict)
    run: dict[str, Any] = field(default_factory=dict)
    reference_files: list[dict[str, Any]] = field(default_factory=list)
    reference_files_removed: bool = False
    gate_reviews: list[dict[str, Any]] = field(default_factory=list)
    repair_history: list[dict[str, Any]] = field(default_factory=list)
    error_message: str = ""
    created_at: str = ""
    updated_at: str = ""

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)

    def effective_reference_files_path(self) -> str:
        return self.reference_files_path or str(self.plan.get("reference_files_path", "")).strip()

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> "TaskContext":
        plan = data.get("plan", {}) if isinstance(data.get("plan", {}), dict) else {}
        return cls(
            version=int(data.get("version", 1)),
            task_id=str(data.get("task_id", "")).strip(),
            status=str(data.get("status", "")).strip(),
            user_requirement=str(data.get("user_requirement", "")).strip(),
            solver_family=str(data.get("solver_family", "")).strip().lower(),
            output_root=str(data.get("output_root", "")).strip(),
            host_output_root=str(data.get("host_output_root", "")).strip(),
            task_dir=str(data.get("task_dir", "")).strip(),
            case_dir=str(data.get("case_dir", "")).strip(),
            mesh_dir=str(data.get("mesh_dir", "")).strip(),
            results_dir=str(data.get("results_dir", "")).strip(),
            logs_dir=str(data.get("logs_dir", "")).strip(),
            context_path=str(data.get("context_path", "")).strip(),
            manifest_path=str(data.get("manifest_path", "")).strip(),
            reference_files_path=str(data.get("reference_files_path", "") or plan.get("reference_files_path", "")).strip(),
            plan=plan,
            generated_files=_list_with_plan_fallback(data, plan, "generated_files"),
            validation=_dict_with_plan_fallback(data, plan, "validation"),
            run=_dict_with_plan_fallback(data, plan, "run"),
            reference_files=_list_with_plan_fallback(data, plan, "reference_files"),
            reference_files_removed=bool(data.get("reference_files_removed", plan.get("reference_files_removed", False))),
            gate_reviews=_list_with_plan_fallback(data, plan, "gate_reviews"),
            repair_history=_list_with_plan_fallback(data, plan, "repair_history"),
            error_message=str(data.get("error_message", "")),
            created_at=str(data.get("created_at", "")),
            updated_at=str(data.get("updated_at", "")),
        )


def _dict_of_string_lists(value: Any) -> dict[str, list[str]]:
    if not isinstance(value, dict):
        return {}
    result: dict[str, list[str]] = {}
    for key, items in value.items():
        if isinstance(items, list):
            result[str(key)] = [str(item) for item in items]
    return result


def _dict_with_fallback(data: dict[str, Any], fallback: dict[str, Any], key: str) -> dict[str, Any]:
    value = data.get(key)
    if isinstance(value, dict):
        return value
    fallback_value = fallback.get(key)
    return fallback_value if isinstance(fallback_value, dict) else {}


def _list_with_fallback(data: dict[str, Any], fallback: dict[str, Any], key: str) -> list[dict[str, Any]]:
    value = data.get(key)
    if isinstance(value, list):
        return [item for item in value if isinstance(item, dict)]
    fallback_value = fallback.get(key)
    if isinstance(fallback_value, list):
        return [item for item in fallback_value if isinstance(item, dict)]
    return []


def _allrun_metadata_from_dict(data: dict[str, Any], parameters: dict[str, Any]) -> dict[str, Any]:
    value = data.get("allrun_metadata")
    if isinstance(value, dict):
        return value

    metadata = {}
    for key in ["allrun_reference", "allrun_pipeline_preview"]:
        if isinstance(parameters.get(key), dict):
            metadata[key] = parameters[key]
    return metadata


def _dict_with_plan_fallback(data: dict[str, Any], plan: dict[str, Any], key: str) -> dict[str, Any]:
    value = data.get(key)
    if isinstance(value, dict):
        return value
    fallback = plan.get(key)
    return fallback if isinstance(fallback, dict) else {}


def _list_with_plan_fallback(data: dict[str, Any], plan: dict[str, Any], key: str) -> list[dict[str, Any]]:
    value = data.get(key)
    if isinstance(value, list):
        return [item for item in value if isinstance(item, dict)]
    fallback = plan.get(key)
    if isinstance(fallback, list):
        return [item for item in fallback if isinstance(item, dict)]
    return []
