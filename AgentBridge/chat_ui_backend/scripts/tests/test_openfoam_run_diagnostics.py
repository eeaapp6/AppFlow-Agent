import unittest

from scripts.simagent_core.solvers.openfoam.run_diagnostics import parse_openfoam_log


class OpenFOAMRunDiagnosticsTests(unittest.TestCase):
    def test_parses_blockmesh_failure(self) -> None:
        diagnostics = parse_openfoam_log(
            "Creating block mesh\n***Error in blockMeshDict\n",
            command="blockMesh",
            log_file="logs/blockMesh.log",
        )

        self.assertEqual("failed", diagnostics["severity"])
        self.assertEqual("result.mesh_quality_failed", diagnostics["items"][0]["code"])
        self.assertEqual("blockMesh", diagnostics["items"][0]["source"])

    def test_parses_missing_boundary_field_with_repair_input(self) -> None:
        diagnostics = parse_openfoam_log(
            "file: /tmp/case/0/U/boundaryField\nCannot find patchField entry for inlet\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        item = diagnostics["items"][0]
        self.assertEqual("result.missing_boundary_field", item["code"])
        self.assertEqual("U", item["field"])
        self.assertEqual("inlet", item["patch"])
        self.assertEqual({"U": ["inlet"]}, diagnostics["metrics"]["missing_boundary_fields"])
        self.assertEqual("add_missing_boundary_field", item["repair_action_input"]["op"])

    def test_parses_unknown_patch(self) -> None:
        diagnostics = parse_openfoam_log(
            "Unknown patch outlet2\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        self.assertEqual("result.unknown_patch", diagnostics["items"][0]["code"])
        self.assertEqual("outlet2", diagnostics["items"][0]["patch"])

    def test_parses_high_courant_number(self) -> None:
        diagnostics = parse_openfoam_log(
            "Courant Number mean: 0.2 max: 3.4\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        self.assertEqual("result.courant_high", diagnostics["items"][0]["code"])
        self.assertEqual(3.4, diagnostics["metrics"]["max_courant"])

    def test_parses_nan_divergence_and_high_residual(self) -> None:
        diagnostics = parse_openfoam_log(
            "smoothSolver:  Solving for p, Initial residual = 0.1, Final residual = 2.5, No Iterations 2\n"
            "solution diverged\n"
            "ExecutionTime = nan s\n",
            command="simpleFoam",
            log_file="logs/simpleFoam.log",
        )
        codes = [item["code"] for item in diagnostics["items"]]

        self.assertIn("result.residual_nan", codes)
        self.assertIn("result.divergence", codes)
        self.assertIn("result.residual_high", codes)
        self.assertEqual("p", diagnostics["metrics"]["worst_residual_field"])

    def test_warning_only_log_is_not_failed(self) -> None:
        diagnostics = parse_openfoam_log(
            "--> FOAM Warning : optional model warning\nEnd\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        self.assertEqual("warning", diagnostics["severity"])
        self.assertEqual("result.log_warning", diagnostics["items"][0]["code"])


if __name__ == "__main__":
    unittest.main()
