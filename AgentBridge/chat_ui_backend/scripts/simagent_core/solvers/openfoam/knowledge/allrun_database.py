import re
from pathlib import Path

from ....models import ReferenceCase


class AllrunScriptDatabase:
    def __init__(self, data_root: str | Path):
        self.data_root = Path(data_root)
        self.allrun_path = self.data_root / "openfoam_allrun_scripts.txt"
        self._content: str | None = None

    @classmethod
    def from_default_data(cls) -> "AllrunScriptDatabase":
        return cls(Path(__file__).resolve().parent / "data")

    def is_available(self) -> bool:
        return self.allrun_path.exists()

    def summarize_reference(self, reference_case: ReferenceCase | None) -> dict:
        if not reference_case:
            return {
                "available": False,
                "source": "openfoam_allrun_scripts.txt",
                "reason": "no_reference_case",
            }
        if not self.is_available():
            return {
                "available": False,
                "source": "openfoam_allrun_scripts.txt",
                "reason": "allrun_database_missing",
            }

        block = self._find_case_block(reference_case)
        if not block:
            return {
                "available": False,
                "source": "openfoam_allrun_scripts.txt",
                "reason": "reference_case_not_found",
                "case_name": reference_case.case_name,
                "solver": reference_case.case_solver,
            }

        script = self._extract_allrun_script(block)
        return {
            "available": bool(script.strip()),
            "source": "openfoam_allrun_scripts.txt",
            "case_name": reference_case.case_name,
            "domain": reference_case.case_domain,
            "category": reference_case.case_category,
            "solver": reference_case.case_solver,
            "script_length": len(script),
            "has_run_application": "runApplication" in script,
            "has_get_application": "getApplication" in script,
        }

    def load_script(self, reference_case: ReferenceCase) -> str:
        block = self._find_case_block(reference_case)
        return self._extract_allrun_script(block) if block else ""

    def _read_content(self) -> str:
        if self._content is None:
            if not self.allrun_path.exists():
                raise FileNotFoundError(f"OpenFOAM Allrun script data not found: {self.allrun_path}")
            self._content = self.allrun_path.read_text(encoding="utf-8", errors="replace")
        return self._content

    def _find_case_block(self, reference_case: ReferenceCase) -> str:
        content = self._read_content()
        for block in re.findall(r"<case_begin>(.*?)</case_end>", content, flags=re.DOTALL):
            index_match = re.search(r"<index>(.*?)</index>", block, flags=re.DOTALL)
            if not index_match:
                continue
            if self._index_matches(index_match.group(1), reference_case):
                return block
        return ""

    def _index_matches(self, index: str, reference_case: ReferenceCase) -> bool:
        return (
            self._extract_index_value(index, "case name") == reference_case.case_name
            and self._extract_index_value(index, "case domain") == reference_case.case_domain
            and self._extract_index_value(index, "case category") == reference_case.case_category
            and self._extract_index_value(index, "case solver") == reference_case.case_solver
        )

    def _extract_index_value(self, index: str, field_name: str) -> str:
        match = re.search(rf"{re.escape(field_name)}:\s*(.*)", index)
        return match.group(1).strip() if match else ""

    def _extract_allrun_script(self, block: str) -> str:
        match = re.search(r"<allrun_script>\n?(.*?)\n?</allrun_script>", block, flags=re.DOTALL)
        return match.group(1).strip() if match else ""
