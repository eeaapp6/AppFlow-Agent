import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.simagent_core.models import (
    PlannedFile,
    ReferenceCase,
    ReferenceFile,
    ReferenceFileSet,
    SimulationPlan,
    TaskContext,
)
from scripts.simagent_core.solvers.openfoam.validate import validate_openfoam_case
from scripts.simagent_core.workflow.nodes.generate_case_node import generate_case_node


def make_task(root: Path, plan: SimulationPlan) -> TaskContext:
    task_dir = root / "task_generate_case"
    return TaskContext(
        version=1,
        task_id="task_generate_case",
        status="planned",
        user_requirement="test",
        solver_family="openfoam",
        output_root=str(root),
        host_output_root=str(root),
        task_dir=str(task_dir),
        case_dir=str(task_dir / "case"),
        mesh_dir=str(task_dir / "mesh"),
        results_dir=str(task_dir / "results"),
        logs_dir=str(task_dir / "logs"),
        context_path=str(task_dir / "task_context.json"),
        manifest_path=str(task_dir / "agent_manifest_v1.json"),
        plan=plan.to_dict(),
    )


def generated_plan() -> SimulationPlan:
    return SimulationPlan(
        solver_family="openfoam",
        solver_name="icoFoam",
        physics_domain="incompressible",
        case_category="rect_channel",
        case_name="rect_channel",
        description="generated",
        generation_mode="generated_case",
    )


def reference_plan() -> SimulationPlan:
    return SimulationPlan(
        solver_family="openfoam",
        solver_name="icoFoam",
        physics_domain="incompressible",
        case_category="cavity",
        case_name="cavity",
        description="reference",
        generation_mode="reference_modify",
        reference_case=ReferenceCase(
            case_name="cavity",
            case_domain="incompressible",
            case_category="cavity",
            case_solver="icoFoam",
        ),
    )


class GenerateCaseNodeTests(unittest.TestCase):
    def test_generated_case_skips_reference_loading(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp), generated_plan())
            generated_files = [{"path": "case/system/controlDict"}]

            with (
                patch(
                    "scripts.simagent_core.workflow.nodes.generate_case_node.reference_loader_node"
                ) as reference_loader_node,
                patch(
                    "scripts.simagent_core.workflow.nodes.generate_case_node.input_writer_node",
                    return_value=generated_files,
                ) as input_writer_node,
            ):
                reference_files, actual_generated_files = generate_case_node(task)

        reference_loader_node.assert_not_called()
        input_writer_node.assert_called_once()
        self.assertEqual([], reference_files)
        self.assertEqual(generated_files, actual_generated_files)

    def test_reference_case_loads_reference_before_writing_inputs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = make_task(Path(tmp), reference_plan())
            reference_files = [{"path": "case/0/U"}]
            generated_files = [{"path": "case/0/U"}]

            with (
                patch(
                    "scripts.simagent_core.workflow.nodes.generate_case_node.reference_loader_node",
                    return_value=reference_files,
                ) as reference_loader_node,
                patch(
                    "scripts.simagent_core.workflow.nodes.generate_case_node.input_writer_node",
                    return_value=generated_files,
                ) as input_writer_node,
            ):
                actual_reference_files, actual_generated_files = generate_case_node(task)

        reference_loader_node.assert_called_once()
        input_writer_node.assert_called_once()
        self.assertEqual(reference_files, actual_reference_files)
        self.assertEqual(generated_files, actual_generated_files)

    def test_regenerate_reloads_and_restores_canonical_allclean(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            plan = reference_plan()
            plan.reference_case.directory_structure = {".": ["Allclean"]}
            plan.planned_files = [
                PlannedFile("case_file", "case/Allclean", "text", required=True),
            ]
            task = make_task(root, plan)
            reference_file_set = ReferenceFileSet(
                reference_case=plan.reference_case,
                files=[
                    ReferenceFile(
                        path="case/Allclean",
                        role="case_file",
                        format="text",
                        content="#!/bin/sh\nrm -rf constant/polyMesh\n",
                    )
                ],
            )

            class ReferenceDatabase:
                def load_reference_files(self, _reference_case):
                    return reference_file_set

            with patch(
                "scripts.simagent_core.workflow.nodes.reference_loader_node."
                "TutorialDetailsDatabase.from_default_data",
                return_value=ReferenceDatabase(),
            ):
                _, first_generated = generate_case_node(task)
                allclean_path = Path(task.task_dir) / "case/Allclean"
                self.assertTrue(allclean_path.exists())
                allclean_path.unlink()
                _, second_generated = generate_case_node(task)

            validation = validate_openfoam_case(task, plan)
            generated_paths = [item["path"] for item in second_generated]
            allclean_content = allclean_path.read_text(encoding="utf-8")

        self.assertEqual(["case/Allclean"], [item["path"] for item in first_generated])
        self.assertEqual(["case/Allclean"], generated_paths)
        self.assertEqual("text", second_generated[0]["format"])
        self.assertTrue(allclean_content.startswith("#!/bin/sh"))
        self.assertEqual("validated", validation["status"])
        self.assertNotIn("case/./Allclean", generated_paths)

    def test_validator_skips_legacy_script_dictionary_check_but_keeps_required_check(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            plan = reference_plan()
            plan.planned_files = [
                PlannedFile("case_file", "case/./Allclean", "openfoam-dict", required=True),
                PlannedFile("control", "case/system/controlDict", "openfoam-dict", required=False),
            ]
            task = make_task(root, plan)
            allclean_path = Path(task.task_dir) / "case/Allclean"
            control_path = Path(task.task_dir) / "case/system/controlDict"
            allclean_path.parent.mkdir(parents=True, exist_ok=True)
            control_path.parent.mkdir(parents=True, exist_ok=True)
            allclean_path.write_text("#!/bin/sh\n", encoding="utf-8")
            control_path.write_text("not an OpenFOAM dictionary\n", encoding="utf-8")

            validation = validate_openfoam_case(task, plan)
            dictionary_error_files = [item["file"] for item in validation["dictionary_errors"]]
            allclean_path.unlink()
            missing_validation = validate_openfoam_case(task, plan)

        self.assertNotIn("case/./Allclean", dictionary_error_files)
        self.assertIn("case/system/controlDict", dictionary_error_files)
        self.assertEqual(["case/./Allclean"], missing_validation["missing_files"])


if __name__ == "__main__":
    unittest.main()
