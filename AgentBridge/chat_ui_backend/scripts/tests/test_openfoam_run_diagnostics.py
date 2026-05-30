import unittest

from scripts.simagent_core.solvers.openfoam.run_diagnostics import parse_openfoam_log


class OpenFOAMRunDiagnosticsTests(unittest.TestCase):
    def assertDiagnosticCode(self, diagnostics: dict, code: str) -> dict:
        for item in diagnostics["items"]:
            if item["code"] == code:
                return item
        self.fail(f"diagnostic code not found: {code}")

    def assertNoDiagnosticCode(self, diagnostics: dict, code: str) -> None:
        self.assertNotIn(code, [item["code"] for item in diagnostics["items"]])

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

    def test_parses_pressure_reference_missing(self) -> None:
        diagnostics = parse_openfoam_log(
            "Unable to set reference cell for field p\nPlease supply either pRefCell or pRefPoint\n",
            command="simpleFoam",
            log_file="logs/simpleFoam.log",
        )

        item = self.assertDiagnosticCode(diagnostics, "result.pressure_reference_missing")
        self.assertEqual("error", item["severity"])
        self.assertEqual("configuration", item["category"])
        self.assertEqual("p", diagnostics["metrics"]["pressure_reference_missing_field"])

    def test_clean_pressure_log_does_not_report_pressure_reference_missing(self) -> None:
        diagnostics = parse_openfoam_log(
            "pRefCell 0;\npRefValue 0;\nExecutionTime = 1 s\n",
            command="simpleFoam",
            log_file="logs/simpleFoam.log",
        )

        self.assertNoDiagnosticCode(diagnostics, "result.pressure_reference_missing")

    def test_parses_inconsistent_patch_type(self) -> None:
        diagnostics = parse_openfoam_log(
            "inconsistent patch and patchField types\n"
            "patch type wall and patchField type fixedValue\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        item = self.assertDiagnosticCode(diagnostics, "result.patch_type_inconsistent")
        self.assertEqual("boundary", item["category"])
        self.assertEqual("wall", diagnostics["metrics"]["patch_type"])
        self.assertEqual("fixedValue", diagnostics["metrics"]["patch_field_type"])

    def test_clean_patch_type_log_does_not_report_inconsistency(self) -> None:
        diagnostics = parse_openfoam_log(
            "patch type wall\npatchField type noSlip\nBoundary checks passed\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        self.assertNoDiagnosticCode(diagnostics, "result.patch_type_inconsistent")

    def test_parses_continuity_error_as_warning_or_error(self) -> None:
        warning = parse_openfoam_log(
            "time step continuity errors : sum local = 0.02, global = -0.003, cumulative = 0.02\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )
        error = parse_openfoam_log(
            "time step continuity errors : sum local = 1.2, global = 0.01, cumulative = 1.21\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        warning_item = self.assertDiagnosticCode(warning, "result.continuity_abnormal")
        error_item = self.assertDiagnosticCode(error, "result.continuity_abnormal")
        self.assertEqual("warning", warning_item["severity"])
        self.assertEqual("error", error_item["severity"])
        self.assertEqual(0.02, warning["metrics"]["max_continuity_local"])
        self.assertEqual(1.2, error["metrics"]["max_continuity_local"])
        self.assertEqual(1.21, error["metrics"]["max_continuity_cumulative"])

    def test_continuity_normal_small_does_not_report_abnormal(self) -> None:
        diagnostics = parse_openfoam_log(
            "time step continuity errors : sum local = 1e-04, global = -2e-05, cumulative = 3e-04\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        self.assertNoDiagnosticCode(diagnostics, "result.continuity_abnormal")
        self.assertEqual(1e-04, diagnostics["metrics"]["max_continuity_local"])
        self.assertEqual(2e-05, diagnostics["metrics"]["max_continuity_global"])

    def test_parses_checkmesh_severe_non_orthogonality(self) -> None:
        diagnostics = parse_openfoam_log(
            "Number of severely non-orthogonal (> 70 degrees) faces: 4\n"
            "Failed 1 mesh checks.\n",
            command="checkMesh",
            log_file="logs/checkMesh.log",
        )

        item = self.assertDiagnosticCode(diagnostics, "result.mesh_severe_non_orthogonality")
        self.assertEqual("error", item["severity"])
        self.assertEqual(4, diagnostics["metrics"]["severe_non_orthogonal_faces"])
        self.assertEqual(1, diagnostics["metrics"]["failed_mesh_checks"])

    def test_checkmesh_ok_does_not_report_mesh_failure(self) -> None:
        diagnostics = parse_openfoam_log(
            "Number of severely non-orthogonal (> 70 degrees) faces: 0\n"
            "Mesh OK.\n",
            command="checkMesh",
            log_file="logs/checkMesh.log",
        )

        self.assertNoDiagnosticCode(diagnostics, "result.mesh_severe_non_orthogonality")
        self.assertNoDiagnosticCode(diagnostics, "result.mesh_negative_volume")
        self.assertEqual(0, diagnostics["metrics"]["severe_non_orthogonal_faces"])

    def test_parses_negative_cell_volume(self) -> None:
        diagnostics = parse_openfoam_log(
            "Zero or negative cell volume detected\n"
            "Number of negative volume cells: 2\n"
            "Minimum negative volume = -3.5e-09\n",
            command="checkMesh",
            log_file="logs/checkMesh.log",
        )

        item = self.assertDiagnosticCode(diagnostics, "result.mesh_negative_volume")
        self.assertEqual("mesh", item["category"])
        self.assertEqual(2, diagnostics["metrics"]["negative_volume_cells"])
        self.assertEqual(-3.5e-09, diagnostics["metrics"]["min_volume"])

    def test_parses_turbulence_properties_missing(self) -> None:
        diagnostics = parse_openfoam_log(
            "FOAM FATAL IO ERROR:\ncannot find file turbulenceProperties\n",
            command="simpleFoam",
            log_file="logs/simpleFoam.log",
        )

        item = self.assertDiagnosticCode(diagnostics, "result.turbulence_properties_missing")
        self.assertEqual("configuration", item["category"])

    def test_clean_turbulence_log_does_not_report_missing_file(self) -> None:
        diagnostics = parse_openfoam_log(
            "Selecting turbulence model type laminar\nExecutionTime = 1 s\n",
            command="simpleFoam",
            log_file="logs/simpleFoam.log",
        )

        self.assertNoDiagnosticCode(diagnostics, "result.turbulence_properties_missing")

    def test_parses_fvsolution_solver_block_missing(self) -> None:
        diagnostics = parse_openfoam_log(
            'keyword H is undefined in dictionary "/case/system/fvSolution.solvers"\n',
            command="simpleFoam",
            log_file="logs/simpleFoam.log",
        )

        item = self.assertDiagnosticCode(diagnostics, "result.fvsolution_solver_block_missing")
        self.assertEqual("configuration", item["category"])
        self.assertEqual("H", diagnostics["metrics"]["missing_fvsolution_keyword"])

    def test_parses_fvsolution_solver_keyword_missing(self) -> None:
        diagnostics = parse_openfoam_log(
            "keyword solver is undefined in dictionary\n",
            command="simpleFoam",
            log_file="logs/simpleFoam.log",
        )

        item = self.assertDiagnosticCode(diagnostics, "result.fvsolution_solver_keyword_missing")
        self.assertEqual("configuration", item["category"])
        self.assertEqual("solver", diagnostics["metrics"]["missing_fvsolution_keyword"])

    def test_courant_level_tracks_warning_error_and_runaway(self) -> None:
        warning = parse_openfoam_log(
            "Courant Number mean: 0.2 max: 10\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )
        error = parse_openfoam_log(
            "Courant Number mean: 0.2 max: 100\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )
        runaway = parse_openfoam_log(
            "Courant Number mean: 0.2 max: 1e4\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        self.assertEqual("result.courant_high", warning["items"][0]["code"])
        self.assertEqual("warning", warning["metrics"]["courant_level"])
        self.assertEqual("error", error["metrics"]["courant_level"])
        self.assertEqual("runaway", runaway["metrics"]["courant_level"])

    def test_courant_normal_small_does_not_report_high(self) -> None:
        diagnostics = parse_openfoam_log(
            "Courant Number mean: 0.02 max: 0.4\n",
            command="icoFoam",
            log_file="logs/icoFoam.log",
        )

        self.assertNoDiagnosticCode(diagnostics, "result.courant_high")
        self.assertEqual(0.4, diagnostics["metrics"]["max_courant"])
        self.assertNotIn("courant_level", diagnostics["metrics"])


if __name__ == "__main__":
    unittest.main()
