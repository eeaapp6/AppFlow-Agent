import unittest

from scripts.simagent_core.gates.capability_gate import review_openfoam_capability
from scripts.simagent_core.models import ReferenceCase
from scripts.simagent_core.router import parse_case_intent
from scripts.simagent_core.solvers.openfoam.capabilities import (
    STATUS_NEEDS_USER_INPUT,
    STATUS_PARTIALLY_SUPPORTED,
    STATUS_SUPPORTED,
    STATUS_UNSUPPORTED,
    evaluate_capability_matrix,
)
from scripts.simagent_core.solvers.openfoam.mesh import external_gmsh_mesh_request
from scripts.simagent_core.solvers.openfoam.plan import create_openfoam_plan


class FakePlanningLLM:
    def __init__(self, requested_changes: dict):
        self.requested_changes = requested_changes

    def chat_json(self, messages: list[dict]) -> dict:
        return {
            "solver_family": "openfoam",
            "solver_name": "icoFoam",
            "physics_domain": "incompressible",
            "case_category": "cavity",
            "case_name": "cavity",
            "description": "cavity capability test",
            "generation_mode": "reference_modify",
            "requested_changes": self.requested_changes,
            "parameters": {},
            "planned_files": [],
        }


def cavity_reference() -> ReferenceCase:
    return ReferenceCase(
        case_name="cavity",
        case_domain="incompressible",
        case_category="cavity",
        case_solver="icoFoam",
    )


def arbitrary_reference() -> ReferenceCase:
    return ReferenceCase(
        case_name="motorBike",
        case_domain="incompressible",
        case_category="externalAerodynamics",
        case_solver="simpleFoam",
    )


def pitz_daily_reference() -> ReferenceCase:
    return ReferenceCase(
        case_name="pitzDaily",
        case_domain="incompressible",
        case_category="None",
        case_solver="simpleFoam",
    )


class OpenFOAMCapabilityMatrixTests(unittest.TestCase):
    def test_matrix_marks_arbitrary_reference_copy_as_supported_without_changes(self) -> None:
        report = evaluate_capability_matrix(
            reference_case=arbitrary_reference(),
            requested_changes={},
            generation_mode="reference_copy",
        )

        reference_entry = next(entry for entry in report.entries if entry.capability == "reference_copy")
        self.assertEqual(STATUS_SUPPORTED, report.status)
        self.assertEqual(STATUS_SUPPORTED, reference_entry.status)

    def test_matrix_does_not_report_arbitrary_unsupported_reference_change_as_supported(self) -> None:
        report = evaluate_capability_matrix(
            reference_case=arbitrary_reference(),
            requested_changes={"turbulence_model": "kOmega"},
            generation_mode="reference_modify",
        )

        reference_entry = next(entry for entry in report.entries if entry.capability == "reference_modify")
        self.assertEqual(STATUS_UNSUPPORTED, report.status)
        self.assertEqual(STATUS_UNSUPPORTED, reference_entry.status)
        self.assertEqual({"turbulence_model": "kOmega"}, report.unsupported_changes)
        self.assertEqual({}, report.supported_changes)

    def test_matrix_marks_icofoam_cavity_as_supported(self) -> None:
        report = evaluate_capability_matrix(
            reference_case=cavity_reference(),
            requested_changes={"lid_velocity": "2"},
            generation_mode="reference_modify",
        )

        self.assertEqual(STATUS_SUPPORTED, report.status)
        self.assertEqual({"lid_velocity": "2"}, report.supported_changes)
        self.assertEqual([], report.needs_user_input)
        self.assertIn("icoFoam_cavity", [entry.capability for entry in report.entries])

    def test_matrix_marks_pitz_daily_inlet_and_outlet_modifiers_as_supported(self) -> None:
        report = evaluate_capability_matrix(
            reference_case=pitz_daily_reference(),
            requested_changes={"inlet_velocity": "10", "outlet_pressure": "0"},
            generation_mode="reference_modify",
        )

        self.assertEqual(STATUS_SUPPORTED, report.status)
        self.assertEqual(
            {"inlet_velocity": "10", "outlet_pressure": "0"},
            report.supported_changes,
        )

    def test_matrix_marks_generated_rect_channel_as_supported(self) -> None:
        report = evaluate_capability_matrix(
            requested_changes={"inlet_velocity": "(1 0 0)", "end_time": "1"},
            generation_mode="generated_case",
            geometry_type="rect_channel",
        )

        self.assertEqual(STATUS_SUPPORTED, report.status)
        self.assertEqual({"inlet_velocity": "(1 0 0)", "end_time": "1"}, report.supported_changes)
        self.assertIn("generated_rect_channel", [entry.capability for entry in report.entries])

    def test_matrix_marks_external_gmsh_mesh_as_supported_when_source_exists_in_request(self) -> None:
        report = evaluate_capability_matrix(
            reference_case=cavity_reference(),
            requested_changes={},
            generation_mode="reference_copy",
            mesh_request=external_gmsh_mesh_request("C:/mesh/channel.msh"),
        )

        self.assertEqual(STATUS_SUPPORTED, report.status)
        mesh_entry = next(entry for entry in report.entries if entry.capability == "external_gmsh_mesh")
        self.assertEqual(STATUS_SUPPORTED, mesh_entry.status)

    def test_capability_decision_reports_partial_support(self) -> None:
        decision = review_openfoam_capability(
            intent=parse_case_intent("modify cavity"),
            reference_case=cavity_reference(),
            reference_selection={"selected_score": 100},
            supported_changes={"lid_velocity": "2"},
            unsupported_changes={"turbulence_model": "kOmega"},
        )

        self.assertEqual("warning", decision.status)
        self.assertEqual(STATUS_PARTIALLY_SUPPORTED, decision.support_status)
        self.assertEqual("reference_modify", decision.mode)
        self.assertEqual({"lid_velocity": "2"}, decision.supported_changes)
        self.assertEqual({"turbulence_model": "kOmega"}, decision.unsupported_changes)
        self.assertEqual("capability.unsupported_change", decision.to_gate_review()["issues"][0]["code"])

    def test_capability_decision_reports_unsupported_when_no_requested_change_can_apply(self) -> None:
        decision = review_openfoam_capability(
            intent=parse_case_intent("modify turbulence model"),
            reference_case=cavity_reference(),
            reference_selection={"selected_score": 100},
            unsupported_changes={"turbulence_model": "kOmega"},
        )

        self.assertEqual("unsupported", decision.status)
        self.assertEqual(STATUS_UNSUPPORTED, decision.support_status)
        self.assertEqual("unsupported", decision.mode)

    def test_capability_decision_reports_needs_user_input_for_missing_gmsh_source(self) -> None:
        decision = review_openfoam_capability(
            intent=parse_case_intent("use gmsh mesh"),
            reference_case=cavity_reference(),
            reference_selection={"selected_score": 100},
            mesh_request={"mode": "gmsh_external", "missing_source": True},
        )

        self.assertEqual("warning", decision.status)
        self.assertEqual(STATUS_NEEDS_USER_INPUT, decision.support_status)
        self.assertEqual("needs_user_input", decision.mode)
        self.assertEqual(["mesh.source_path"], decision.needs_user_input)

    def test_openfoam_plan_persists_partial_capability_decision_and_gate_review(self) -> None:
        plan = create_openfoam_plan(
            "set lid velocity and turbulence model",
            llm=FakePlanningLLM({"lid_velocity": "2", "turbulence_model": "kOmega"}),
        )

        decision = plan.parameters["capability_decision"]
        capability_gate = next(item for item in plan.parameters["gate_reviews"] if item["gate"] == "capability")

        self.assertEqual(STATUS_PARTIALLY_SUPPORTED, decision["support_status"])
        self.assertEqual("warning", capability_gate["status"])
        self.assertEqual({"lid_velocity": "2"}, plan.requested_changes)
        self.assertEqual({"turbulence_model": "kOmega"}, plan.parameters["unsupported_changes"])

    def test_openfoam_plan_rejects_fully_unsupported_requested_changes(self) -> None:
        with self.assertRaisesRegex(ValueError, "outside the current OpenFOAM capability matrix"):
            create_openfoam_plan(
                "set turbulence model",
                llm=FakePlanningLLM({"turbulence_model": "kOmega"}),
            )


if __name__ == "__main__":
    unittest.main()
