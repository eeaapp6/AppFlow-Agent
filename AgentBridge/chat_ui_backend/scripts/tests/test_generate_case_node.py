import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from scripts.simagent_core.models import ReferenceCase, SimulationPlan, TaskContext
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


if __name__ == "__main__":
    unittest.main()
