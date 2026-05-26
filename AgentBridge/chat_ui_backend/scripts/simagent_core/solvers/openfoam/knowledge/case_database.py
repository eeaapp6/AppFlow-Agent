import json
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


@dataclass
class TutorialCase:
    case_name: str
    case_domain: str
    case_category: str
    case_solver: str
    directory_structure: dict[str, list[str]] = field(default_factory=dict)

    def to_dict(self) -> dict[str, Any]:
        return {
            "case_name": self.case_name,
            "case_domain": self.case_domain,
            "case_category": self.case_category,
            "case_solver": self.case_solver,
            "directory_structure": self.directory_structure,
        }


class OpenFOAMCaseDatabase:
    def __init__(self, data_root: str | Path):
        self.data_root = Path(data_root)
        self._case_stats: dict[str, list[str]] | None = None
        self._tutorial_cases: list[TutorialCase] | None = None

    @classmethod
    def from_default_data(cls) -> "OpenFOAMCaseDatabase":
        return cls(Path(__file__).resolve().parent / "data")

    @classmethod
    def from_project_root(cls, project_root: str | Path | None = None) -> "OpenFOAMCaseDatabase":
        root = Path(project_root) if project_root else Path(__file__).resolve().parents[5]
        return cls(root / "scripts" / "simagent_core" / "solvers" / "openfoam" / "knowledge" / "data")

    def is_available(self) -> bool:
        return (self.data_root / "openfoam_case_stats.json").exists() and (
            self.data_root / "openfoam_tutorials_structure.txt"
        ).exists()

    def load_case_stats(self) -> dict[str, list[str]]:
        if self._case_stats is not None:
            return self._case_stats

        path = self.data_root / "openfoam_case_stats.json"
        if not path.exists():
            raise FileNotFoundError(f"OpenFOAM case stats not found: {path}")

        data = json.loads(path.read_text(encoding="utf-8"))
        self._case_stats = {
            key: sorted(str(item) for item in value)
            for key, value in data.items()
            if isinstance(value, list)
        }
        return self._case_stats

    def load_tutorial_cases(self) -> list[TutorialCase]:
        if self._tutorial_cases is not None:
            return self._tutorial_cases

        path = self.data_root / "openfoam_tutorials_structure.txt"
        if not path.exists():
            raise FileNotFoundError(f"OpenFOAM tutorial structure not found: {path}")

        content = path.read_text(encoding="utf-8", errors="replace")
        blocks = re.findall(r"<case_begin>(.*?)</case_end>", content, flags=re.DOTALL)
        self._tutorial_cases = [case for block in blocks if (case := self._parse_case_block(block))]
        return self._tutorial_cases

    def find_cases(
        self,
        *,
        domain: str = "",
        solver: str = "",
        category: str = "",
        limit: int = 10,
    ) -> list[TutorialCase]:
        domain = domain.strip().lower()
        solver = solver.strip().lower()
        category = category.strip().lower()

        scored: list[tuple[int, TutorialCase]] = []
        for case in self.load_tutorial_cases():
            score = 0
            if domain and case.case_domain.lower() == domain:
                score += 4
            elif domain:
                continue

            if solver and case.case_solver.lower() == solver:
                score += 3
            elif solver:
                continue

            if category and case.case_category.lower() == category:
                score += 2
            elif category:
                continue

            scored.append((score, case))

        scored.sort(key=lambda item: (-item[0], item[1].case_name, item[1].case_solver))
        return [case for _, case in scored[:limit]]

    def summarize(self) -> dict[str, Any]:
        stats = self.load_case_stats()
        cases = self.load_tutorial_cases()
        return {
            "data_root": str(self.data_root),
            "case_count": len(cases),
            "case_domain_count": len(stats.get("case_domain", [])),
            "case_category_count": len(stats.get("case_category", [])),
            "case_solver_count": len(stats.get("case_solver", [])),
            "available_domains": stats.get("case_domain", []),
            "available_solvers": stats.get("case_solver", []),
        }

    def _parse_case_block(self, block: str) -> TutorialCase | None:
        index_match = re.search(r"<index>(.*?)</index>", block, flags=re.DOTALL)
        structure_match = re.search(r"<directory_structure>(.*?)</directory_structure>", block, flags=re.DOTALL)
        if not index_match or not structure_match:
            return None

        index = index_match.group(1)
        return TutorialCase(
            case_name=self._extract_index_value(index, "case name"),
            case_domain=self._extract_index_value(index, "case domain"),
            case_category=self._extract_index_value(index, "case category"),
            case_solver=self._extract_index_value(index, "case solver"),
            directory_structure=self._parse_directory_structure(structure_match.group(1)),
        )

    def _extract_index_value(self, index: str, field_name: str) -> str:
        match = re.search(rf"{re.escape(field_name)}:\s*(.*)", index)
        return match.group(1).strip() if match else ""

    def _parse_directory_structure(self, structure: str) -> dict[str, list[str]]:
        result: dict[str, list[str]] = {}
        dir_matches = re.findall(
            r"<dir>directory name:\s*(.*?)\.\s*File names in this directory:\s*\[(.*?)\]</dir>",
            structure,
            flags=re.DOTALL,
        )
        for directory_name, files_text in dir_matches:
            files = [item.strip() for item in files_text.split(",") if item.strip()]
            result[directory_name.strip()] = files
        return result
