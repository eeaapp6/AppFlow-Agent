import re
import tempfile
import unittest
from pathlib import Path

from scripts.agent_service import has_validation_errors
from scripts.simagent_core.agent.replies import validate_reply
from scripts.simagent_core.models import TaskContext
from scripts.simagent_core.solvers.openfoam.generated import create_rect_channel_plan
from scripts.simagent_core.solvers.openfoam.input_writer import generate_openfoam_files
from scripts.simagent_core.solvers.openfoam.validate import validate_openfoam_case
from scripts.simagent_core.workflow.nodes.validator_node import _static_validation_gate_review


RECT_CHANNEL_PROMPT = "generate a 2d rectangular channel flow, length 5m, height 1m, inlet velocity 1m/s"


def make_task(root: Path) -> TaskContext:
    task_dir = root / "task_boundary_validation"
    return TaskContext(
        version=1,
        task_id="task_boundary_validation",
        status="planned",
        user_requirement=RECT_CHANNEL_PROMPT,
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
    )


def generated_rect_channel(root: Path) -> tuple[TaskContext, object]:
    task = make_task(root)
    plan = create_rect_channel_plan(RECT_CHANNEL_PROMPT)
    generate_openfoam_files(task, plan)
    return task, plan


def replace_text(path: Path, pattern: str, replacement: str) -> None:
    content = path.read_text(encoding="utf-8")
    updated = re.sub(pattern, replacement, content, flags=re.DOTALL)
    path.write_text(updated, encoding="utf-8")


def append_boundary_patch(path: Path, patch_block: str) -> None:
    content = path.read_text(encoding="utf-8").rstrip()
    path.write_text(f"{content[:-1]}\n{patch_block}\n}}\n", encoding="utf-8")


def error_codes(validation: dict) -> list[str]:
    return [item["code"] for item in validation.get("boundary_condition_errors", [])]


def warning_codes(validation: dict) -> list[str]:
    return [item["code"] for item in validation.get("boundary_condition_warnings", [])]


class OpenFOAMBoundaryValidationTests(unittest.TestCase):
    def test_missing_patch_returns_missing_boundary_fields(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            u_path = Path(task.task_dir) / "case/0/U"
            replace_text(u_path, r"\n\s*inlet\s*\{.*?\n\s*\}\n", "\n")

            validation = validate_openfoam_case(task, plan)
            gate_review = _static_validation_gate_review(validation)

        self.assertEqual("validation_failed", validation["status"])
        self.assertEqual(["inlet"], validation["missing_boundary_fields"]["U"])
        self.assertEqual("failed", gate_review["status"])

    def test_extra_patch_returns_extra_boundary_fields(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            p_path = Path(task.task_dir) / "case/0/p"
            append_boundary_patch(
                p_path,
                """
    ghostPatch
    {
        type            zeroGradient;
    }""",
            )

            validation = validate_openfoam_case(task, plan)

        self.assertEqual("validation_failed", validation["status"])
        self.assertEqual(["ghostPatch"], validation["extra_boundary_fields"]["p"])

    def test_empty_boundary_field_returns_empty_boundary_fields(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            u_path = Path(task.task_dir) / "case/0/U"
            replace_text(u_path, r"boundaryField\s*\{.*\}\s*$", "boundaryField\n{\n}\n")

            validation = validate_openfoam_case(task, plan)

        self.assertEqual("validation_failed", validation["status"])
        self.assertIn("U", validation["empty_boundary_fields"])
        self.assertEqual(["frontAndBack", "inlet", "outlet", "walls"], validation["missing_boundary_fields"]["U"])

    def test_poly_mesh_boundary_takes_precedence_over_block_mesh_dict(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            boundary_dir = Path(task.task_dir) / "case/constant/polyMesh"
            boundary_dir.mkdir(parents=True)
            (boundary_dir / "boundary").write_text(
                """FoamFile
{
    version 2.0;
    format ascii;
    class polyBoundaryMesh;
    object boundary;
}

3
(
    inlet
    {
        type patch;
        nFaces 1;
        startFace 0;
    }
    outlet
    {
        type patch;
        nFaces 1;
        startFace 1;
    }
    walls
    {
        type wall;
        nFaces 2;
        startFace 2;
    }
)
""",
                encoding="utf-8",
            )

            validation = validate_openfoam_case(task, plan)

        self.assertEqual(["inlet", "outlet", "walls"], validation["mesh_patches"])
        self.assertEqual(["frontAndBack"], validation["extra_boundary_fields"]["U"])
        self.assertEqual(["frontAndBack"], validation["extra_boundary_fields"]["p"])

    def test_existing_turbulence_field_files_are_checked_against_mesh_patches(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            (Path(task.task_dir) / "case/0/nut").write_text(
                """FoamFile
{
    version 2.0;
    format ascii;
    class volScalarField;
    object nut;
}

dimensions      [0 2 -1 0 0 0 0];
internalField   uniform 0;

boundaryField
{
    inlet
    {
        type calculated;
    }
}
""",
                encoding="utf-8",
            )

            validation = validate_openfoam_case(task, plan)

        self.assertEqual(["frontAndBack", "outlet", "walls"], validation["missing_boundary_fields"]["nut"])

    def test_inlet_velocity_zero_gradient_is_structured_boundary_error(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            u_path = Path(task.task_dir) / "case/0/U"
            replace_text(u_path, r"(inlet\s*\{\s*type\s+)fixedValue;", r"\1zeroGradient;")

            validation = validate_openfoam_case(task, plan)

        self.assertIn("boundary.inlet_velocity_zero_gradient", error_codes(validation))
        error = next(item for item in validation["boundary_condition_errors"] if item["patch"] == "inlet")
        self.assertEqual("case/0/U", error["file"])
        self.assertEqual("U", error["field"])

    def test_outlet_pressure_fixed_value_without_value_is_structured_boundary_error(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            p_path = Path(task.task_dir) / "case/0/p"
            replace_text(p_path, r"\n\s*value\s+uniform\s+0;", "")

            validation = validate_openfoam_case(task, plan)

        self.assertIn("boundary.outlet_pressure_fixed_value_without_value", error_codes(validation))

    def test_wall_velocity_zero_gradient_is_structured_boundary_error(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            u_path = Path(task.task_dir) / "case/0/U"
            replace_text(u_path, r"(walls\s*\{\s*type\s+)noSlip;", r"\1zeroGradient;")

            validation = validate_openfoam_case(task, plan)

        self.assertIn("boundary.wall_velocity_unphysical", error_codes(validation))

    def test_wall_pressure_fixed_value_is_warning_and_does_not_block_run(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task, plan = generated_rect_channel(Path(tmp))
            p_path = Path(task.task_dir) / "case/0/p"
            replace_text(
                p_path,
                r"(walls\s*\{\s*type\s+)zeroGradient;",
                "walls\n    {\n        type            fixedValue;\n        value           uniform 0;",
            )

            validation = validate_openfoam_case(task, plan)
            gate_review = _static_validation_gate_review(validation)
            reply = validate_reply(task, validation)

        self.assertEqual("validated", validation["status"])
        self.assertFalse(has_validation_errors(validation))
        self.assertEqual("warning", gate_review["status"])
        self.assertIn("boundary.wall_pressure_fixed_value", warning_codes(validation))
        self.assertEqual(["warning"], [item["severity"] for item in gate_review["issues"]])
        self.assertIn("passed with warnings", reply)


if __name__ == "__main__":
    unittest.main()
