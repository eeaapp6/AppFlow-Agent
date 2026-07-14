import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.models import ReferenceCase
from scripts.simagent_core.solvers.openfoam.knowledge import OpenFOAMCaseDatabase, TutorialDetailsDatabase
from scripts.simagent_core.solvers.openfoam.plan import _planned_files_from_reference


def reference_case(file_names: list[str]) -> ReferenceCase:
    return ReferenceCase(
        case_name="parserCase",
        case_domain="incompressible",
        case_category="parser",
        case_solver="simpleFoam",
        directory_structure={".": file_names},
    )


def details_document(file_blocks: str) -> str:
    return f"""<case_begin>
<index>
case name: parserCase
case domain: incompressible
case category: parser
case solver: simpleFoam
</index>
<tutorials>
<directory_begin>directory name: .
{file_blocks}
</directory_end>
</tutorials>
</case_end>
"""


class TutorialDetailsTests(unittest.TestCase):
    def load_custom_files(self, document: str, file_names: list[str]):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "openfoam_tutorials_details.txt"
            path.write_text(document, encoding="utf-8")
            return TutorialDetailsDatabase(tmp).load_reference_files(reference_case(file_names)).files

    def test_loader_parses_content_after_newline(self) -> None:
        files = self.load_custom_files(
            details_document(
                """<file_begin>file name: Allrun
<file_content>
#!/bin/sh
simpleFoam
</file_content>
</file_end>"""
            ),
            ["Allrun"],
        )

        self.assertEqual(1, len(files))
        self.assertTrue(files[0].content.startswith("#!/bin/sh"))
        self.assertIn("simpleFoam", files[0].content)
        self.assertEqual("text", files[0].format)

    def test_loader_parses_inline_content(self) -> None:
        files = self.load_custom_files(
            details_document(
                """<file_begin>file name: Allclean
<file_content>#!/bin/sh
rm -rf constant/polyMesh
</file_content>
</file_end>"""
            ),
            ["Allclean"],
        )

        self.assertEqual("#!/bin/sh\nrm -rf constant/polyMesh\n", files[0].content)
        self.assertEqual("case/Allclean", files[0].path)
        self.assertEqual("text", files[0].format)

    def test_multiple_files_do_not_swallow_adjacent_content(self) -> None:
        files = self.load_custom_files(
            details_document(
                """<file_begin>file name: Allclean
<file_content>#!/bin/sh
echo clean
</file_content>
</file_end>
<file_begin>file name: Allrun
<file_content>#!/bin/sh
echo run
</file_content>
</file_end>"""
            ),
            ["Allclean", "Allrun"],
        )

        self.assertEqual(["case/Allclean", "case/Allrun"], [item.path for item in files])
        self.assertNotIn("echo run", files[0].content)
        self.assertNotIn("echo clean", files[1].content)

    def test_real_airfoil_details_include_canonical_allclean_script(self) -> None:
        tutorial_case = next(
            item
            for item in OpenFOAMCaseDatabase.from_default_data().load_tutorial_cases()
            if item.case_name == "airFoil2D" and item.case_solver == "simpleFoam"
        )
        case = ReferenceCase.from_dict(tutorial_case.to_dict())

        file_set = TutorialDetailsDatabase.from_default_data().load_reference_files(case)
        allclean = next(item for item in file_set.files if item.path == "case/Allclean")

        self.assertTrue(allclean.content.startswith("#!/bin/sh"))
        self.assertEqual("text", allclean.format)
        self.assertNotIn("case/./Allclean", [item.path for item in file_set.files])

    def test_planned_root_script_uses_same_canonical_path_and_format(self) -> None:
        tutorial_case = OpenFOAMCaseDatabase.from_default_data().load_tutorial_cases()[0]
        tutorial_case.directory_structure = {".": ["Allclean"]}

        planned_files = _planned_files_from_reference(tutorial_case)

        self.assertEqual(1, len(planned_files))
        self.assertEqual("case/Allclean", planned_files[0].path)
        self.assertEqual("text", planned_files[0].format)
        self.assertTrue(planned_files[0].required)


if __name__ == "__main__":
    unittest.main()
