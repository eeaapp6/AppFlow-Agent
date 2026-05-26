import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.models import SimulationPlan
from scripts.simagent_core.solvers.openfoam.run_pipeline import (
    build_basic_run_pipeline,
    parse_allrun_pipeline_preview,
    prepare_executable_pipeline,
)


ALLOWED_COMMANDS = {
    "blockMesh",
    "checkMesh",
    "simpleFoam",
    "snappyHexMesh",
}


class OpenFOAMRunPipelineTests(unittest.TestCase):
    def test_basic_pipeline_uses_plan_solver(self) -> None:
        plan = SimulationPlan(
            solver_family="openfoam",
            solver_name="simpleFoam",
            physics_domain="incompressible",
            case_category="externalFlow",
            case_name="airFoil2D",
            description="test",
        )

        pipeline = build_basic_run_pipeline(plan)

        self.assertEqual(["blockMesh", "simpleFoam"], [step["command"] for step in pipeline])
        self.assertEqual(["mesh", "solve"], [step["stage"] for step in pipeline])

    def test_allrun_application_variable_maps_to_solver_name(self) -> None:
        preview = parse_allrun_pipeline_preview(
            "runApplication $application",
            ALLOWED_COMMANDS,
            solver_name="simpleFoam",
        )

        self.assertTrue(preview["available"])
        self.assertEqual("simpleFoam", preview["steps"][0]["command"])
        self.assertEqual([], preview["warnings"])

    def test_denied_shell_command_is_not_added_to_steps(self) -> None:
        preview = parse_allrun_pipeline_preview(
            "runApplication rm -rf case",
            ALLOWED_COMMANDS,
            solver_name="simpleFoam",
        )

        self.assertFalse(preview["available"])
        self.assertEqual([], preview["steps"])
        self.assertIn("denied command rm", preview["warnings"][0])

    def test_shell_operator_is_rejected(self) -> None:
        preview = parse_allrun_pipeline_preview(
            "runApplication blockMesh ; rm -rf case",
            ALLOWED_COMMANDS,
            solver_name="simpleFoam",
        )

        self.assertFalse(preview["available"])
        self.assertEqual([], preview["steps"])
        self.assertIn("skipped shell operator", preview["warnings"][0])

    def test_allowed_environment_path_arg_is_normalized(self) -> None:
        preview = parse_allrun_pipeline_preview(
            "runApplication blockMesh -dict $FOAM_TUTORIALS/resources/blockMesh/pitzDaily",
            ALLOWED_COMMANDS,
            solver_name="simpleFoam",
        )

        self.assertTrue(preview["available"])
        step = preview["steps"][0]
        self.assertEqual("blockMesh", step["command"])
        self.assertEqual(["-dict", "${FOAM_TUTORIALS}/resources/blockMesh/pitzDaily"], step["args"])
        self.assertEqual(["FOAM_TUTORIALS"], step["requires_env"])

    def test_unknown_environment_variable_is_rejected(self) -> None:
        preview = parse_allrun_pipeline_preview(
            "runApplication blockMesh -dict $UNKNOWN_ROOT/resources/blockMesh/pitzDaily",
            ALLOWED_COMMANDS,
            solver_name="simpleFoam",
        )

        self.assertFalse(preview["available"])
        self.assertEqual([], preview["steps"])
        self.assertIn("unknown environment variable UNKNOWN_ROOT", preview["warnings"][0])

    def test_prepare_executable_pipeline_expands_existing_path_argument(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tutorials_root = Path(tmp)
            dict_path = tutorials_root / "resources" / "blockMesh" / "pitzDaily"
            dict_path.parent.mkdir(parents=True)
            dict_path.write_text("FoamFile{}\n", encoding="utf-8")
            preview = {
                "warnings": [],
                "steps": [
                    {
                        "command": "blockMesh",
                        "args": ["-dict", "${FOAM_TUTORIALS}/resources/blockMesh/pitzDaily"],
                        "stage": "mesh",
                        "source": "allrun",
                    }
                ],
            }

            executable = prepare_executable_pipeline(
                preview,
                env={"FOAM_TUTORIALS": str(tutorials_root)},
            )

            self.assertTrue(executable["executable"])
            self.assertEqual(str(dict_path), executable["steps"][0]["args"][1])
            self.assertEqual("allrun_safe", executable["steps"][0]["source"])

    def test_prepare_executable_pipeline_rejects_missing_path_argument(self) -> None:
        preview = {
            "warnings": [],
            "steps": [
                {
                    "command": "blockMesh",
                    "args": ["-dict", "${FOAM_TUTORIALS}/missingDict"],
                    "stage": "mesh",
                    "source": "allrun",
                }
            ],
        }

        executable = prepare_executable_pipeline(
            preview,
            env={"FOAM_TUTORIALS": "C:/definitely/missing"},
        )

        self.assertFalse(executable["executable"])
        self.assertIn("path does not exist", executable["warnings"][0])


if __name__ == "__main__":
    unittest.main()
