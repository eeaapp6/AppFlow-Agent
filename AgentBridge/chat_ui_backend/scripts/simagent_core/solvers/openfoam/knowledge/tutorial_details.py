import re
from pathlib import Path, PurePosixPath

from ....models import ReferenceCase, ReferenceFile, ReferenceFileSet


OPENFOAM_REFERENCE_SCRIPTS = {"allclean", "allrun"}


def normalize_reference_file_path(directory_name: str, file_name: str) -> str:
    directory = str(directory_name or "").strip().replace("\\", "/")
    name = str(file_name or "").strip().replace("\\", "/")
    directory_path = PurePosixPath(directory or ".")
    file_path = PurePosixPath(name)
    if directory_path.is_absolute() or file_path.is_absolute():
        raise ValueError("Reference file path must be relative.")
    if ".." in directory_path.parts or ".." in file_path.parts:
        raise ValueError("Reference file path must not escape the case directory.")

    parts = [part for part in directory_path.parts if part not in {"", "."}]
    file_parts = [part for part in file_path.parts if part not in {"", "."}]
    if not file_parts:
        raise ValueError("Reference file name must not be empty.")
    return str(PurePosixPath("case", *parts, *file_parts))


def is_reference_script_path(path: str) -> bool:
    normalized = str(path or "").strip().replace("\\", "/")
    return PurePosixPath(normalized).name.lower() in OPENFOAM_REFERENCE_SCRIPTS


def reference_file_format(path: str) -> str:
    return "text" if is_reference_script_path(path) else "openfoam-dict"


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
        expected_files = set(reference_case.directory_structure.get(directory_name, []))
        for file_block_match in re.finditer(
            r"<file_begin>(.*?)</file_end>",
            directory_body,
            flags=re.DOTALL,
        ):
            file_block = file_block_match.group(1)
            file_name_match = re.search(r"file name:\s*([^\r\n]+)", file_block)
            content_start = file_block.find("<file_content>")
            if not file_name_match or content_start == -1:
                continue
            content_start += len("<file_content>")
            content_end = file_block.find("</file_content>", content_start)
            if content_end == -1:
                continue

            file_name = file_name_match.group(1).strip()
            if expected_files and file_name not in expected_files:
                continue
            content = file_block[content_start:content_end]
            if content.startswith("\r\n"):
                content = content[2:]
            elif content.startswith("\n"):
                content = content[1:]
            path = normalize_reference_file_path(directory_name, file_name)
            files.append(
                ReferenceFile(
                    path=path,
                    role=self._role_for_openfoam_file(directory_name, file_name),
                    format=reference_file_format(path),
                    content=content,
                )
            )
        return files

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
