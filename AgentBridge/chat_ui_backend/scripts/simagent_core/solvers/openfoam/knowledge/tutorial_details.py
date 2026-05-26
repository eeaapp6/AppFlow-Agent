import re
from pathlib import Path

from ....models import ReferenceCase, ReferenceFile, ReferenceFileSet


class TutorialDetailsDatabase:
    def __init__(self, data_root: str | Path):
        self.data_root = Path(data_root)
        self.details_path = self.data_root / "openfoam_tutorials_details.txt"
        self._content: str | None = None

    @classmethod
    def from_default_data(cls) -> "TutorialDetailsDatabase":
        return cls(Path(__file__).resolve().parent / "data")

    def is_available(self) -> bool:
        return self.details_path.exists()

    def load_reference_files(self, reference_case: ReferenceCase) -> ReferenceFileSet:
        block = self._find_case_block(reference_case)
        if not block:
            raise ValueError(
                "Reference case details not found: "
                f"{reference_case.case_name} / {reference_case.case_domain} / "
                f"{reference_case.case_category} / {reference_case.case_solver}"
            )

        files = self._parse_files(block, reference_case)
        return ReferenceFileSet(reference_case=reference_case, files=files)

    def _read_content(self) -> str:
        if self._content is None:
            if not self.details_path.exists():
                raise FileNotFoundError(f"OpenFOAM tutorial details not found: {self.details_path}")
            self._content = self.details_path.read_text(encoding="utf-8", errors="replace")
        return self._content

    def _find_case_block(self, reference_case: ReferenceCase) -> str:
        content = self._read_content()
        for block in re.findall(r"<case_begin>(.*?)</case_end>", content, flags=re.DOTALL):
            index_match = re.search(r"<index>(.*?)</index>", block, flags=re.DOTALL)
            if not index_match:
                continue
            index = index_match.group(1)
            if self._index_matches(index, reference_case):
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

    def _parse_files(self, block: str, reference_case: ReferenceCase) -> list[ReferenceFile]:
        tutorials_match = re.search(r"<tutorials>(.*?)</tutorials>", block, flags=re.DOTALL)
        if not tutorials_match:
            return []

        files: list[ReferenceFile] = []
        tutorials = tutorials_match.group(1)
        directory_blocks = re.findall(
            r"<directory_begin>directory name:\s*(.*?)(?=<directory_begin>directory name:|\Z)",
            tutorials,
            flags=re.DOTALL,
        )
        for directory_block in directory_blocks:
            first_line, _, rest = directory_block.partition("\n")
            directory_name = first_line.strip()
            files.extend(self._parse_directory_files(directory_name, rest, reference_case))
        return files

    def _parse_directory_files(
        self,
        directory_name: str,
        directory_body: str,
        reference_case: ReferenceCase,
    ) -> list[ReferenceFile]:
        files: list[ReferenceFile] = []
        file_matches = re.findall(
            r"<file_begin>file name:\s*(.*?)\n<file_content>\n(.*?)\n</file_content>\n</file_end>",
            directory_body,
            flags=re.DOTALL,
        )
        expected_files = set(reference_case.directory_structure.get(directory_name, []))
        for file_name, content in file_matches:
            file_name = file_name.strip()
            if expected_files and file_name not in expected_files:
                continue
            files.append(
                ReferenceFile(
                    path=self._case_relative_path(directory_name, file_name),
                    role=self._role_for_openfoam_file(directory_name, file_name),
                    format="openfoam-dict",
                    content=content.strip() + "\n",
                )
            )
        return files

    def _case_relative_path(self, directory_name: str, file_name: str) -> str:
        directory_name = directory_name.strip().strip("/")
        if directory_name in {"", "."}:
            return f"case/{file_name}"
        return f"case/{directory_name}/{file_name}".replace("//", "/")

    def _role_for_openfoam_file(self, directory: str, file_name: str) -> str:
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
