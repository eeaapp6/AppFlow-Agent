import unittest

from scripts.simagent_core.gates.capability_gate import review_openfoam_capability
from scripts.simagent_core.gates.execution_gate import review_run_pipeline
from scripts.simagent_core.gates.generation_gate import review_generated_files
from scripts.simagent_core.gates.spec_gate import review_simulation_spec
from scripts.simagent_core.models import PlannedFile, ReferenceCase
from scripts.simagent_core.router import parse_case_intent
from scripts.simagent_core.spec import BoundarySpec, GeometrySpec, MeshSpec, SimulationSpec, SolverSpec


class GateTests(unittest.TestCase):
    def test_capability_gate_prefers_strong_reference_when_new_geometry_is_not_explicit(self) -> None:
        decision = review_openfoam_capability(
            intent=parse_case_intent("\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u7ba1\u9053\u6d41"),
            reference_case=ReferenceCase("cavity", "incompressible", "cavity", "icoFoam"),
            reference_selection={"selected_score": 70},
            supported_changes={"end_time": "2"},
        )

        self.assertEqual("passed", decision.status)
        self.assertEqual("reference_modify", decision.mode)
        self.assertEqual("controlled", decision.boundary)

    def test_capability_gate_uses_generated_case_when_reference_is_weak(self) -> None:
        decision = review_openfoam_capability(
            intent=parse_case_intent("\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u7ba1\u9053\u6d41"),
            reference_case=ReferenceCase("cavity", "incompressible", "cavity", "icoFoam"),
            reference_selection={"selected_score": 69},
        )

        self.assertEqual("passed", decision.status)
        self.assertEqual("generated_case", decision.mode)
        self.assertEqual("rect_channel", decision.geometry_type)

    def test_capability_gate_rejects_explicit_unsupported_new_geometry(self) -> None:
        decision = review_openfoam_capability(
            intent=parse_case_intent("\u6570\u636e\u5e93\u6ca1\u6709\u4e5f\u751f\u6210\u4e00\u4e2a\u65b0\u51e0\u4f55"),
            reference_selection={"selected_score": 100},
        )

        self.assertEqual("unsupported", decision.status)
        self.assertEqual("unsupported", decision.mode)
        self.assertEqual("unsupported", decision.boundary)

    def test_spec_gate_accepts_reference_openfoam_spec(self) -> None:
        spec = SimulationSpec(
            geometry=GeometrySpec(type="reference_case", source="cavity"),
            mesh=MeshSpec(type="reference_mesh"),
            boundaries=[BoundarySpec("movingWall", "wall")],
            solver=SolverSpec("icoFoam", "transient_incompressible"),
        )

        result = review_simulation_spec(spec)

        self.assertEqual("passed", result.status)

    def test_spec_gate_rejects_unsupported_geometry_and_boundary_role(self) -> None:
        spec = SimulationSpec(
            geometry=GeometrySpec(type="arbitrary_cad_kernel"),
            mesh=MeshSpec(type="reference_mesh"),
            boundaries=[BoundarySpec("inlet", "magic_inlet")],
            solver=SolverSpec("icoFoam"),
        )

        result = review_simulation_spec(spec)

        self.assertEqual("failed", result.status)
        self.assertIn("geometry.unsupported_type", [issue.code for issue in result.issues])
        self.assertIn("boundary.unsupported_role", [issue.code for issue in result.issues])

    def test_generation_gate_rejects_path_escape(self) -> None:
        result = review_generated_files([
            PlannedFile("control", "../outside", "openfoam-dict"),
        ])

        self.assertEqual("failed", result.status)
        self.assertIn("file.empty_path", [issue.code for issue in result.issues])

    def test_execution_gate_rejects_shell_syntax_and_unknown_command(self) -> None:
        result = review_run_pipeline([
            {"command": "bash", "args": ["-lc", "rm -rf case"], "stage": "bad"},
            {"command": "blockMesh", "args": [";", "rm"], "stage": "bad"},
        ])

        self.assertEqual("failed", result.status)
        self.assertIn("pipeline.command_not_allowed", [issue.code for issue in result.issues])
        self.assertIn("pipeline.shell_syntax", [issue.code for issue in result.issues])


if __name__ == "__main__":
    unittest.main()
