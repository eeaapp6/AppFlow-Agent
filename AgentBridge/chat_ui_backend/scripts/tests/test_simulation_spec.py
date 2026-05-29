import unittest

from scripts.simagent_core.gates.spec_gate import review_simulation_spec
from scripts.simagent_core.spec import (
    BoundarySpec,
    GeometrySpec,
    MeshSpec,
    NumericsSpec,
    OutputsSpec,
    PhysicsSpec,
    SimulationSpec,
    SolverSpec,
)
from scripts.simagent_core.solvers.openfoam.plan import create_openfoam_plan


class FakePlanningLLM:
    def chat_json(self, messages: list[dict]) -> dict:
        return {
            "solver_family": "openfoam",
            "solver_name": "icoFoam",
            "physics_domain": "incompressible",
            "case_category": "cavity",
            "case_name": "cavity",
            "description": "cavity with explicit controls",
            "generation_mode": "reference_modify",
            "requested_changes": {
                "end_time": "2",
                "delta_t": "0.01",
                "write_interval": "25",
                "kinematic_viscosity": "0.001",
            },
            "parameters": {},
            "planned_files": [],
        }


class PollutedSpecLLM(FakePlanningLLM):
    def chat_json(self, messages: list[dict]) -> dict:
        data = super().chat_json(messages)
        data["parameters"] = {
            "simulation_spec": {
                "geometry": {"type": "arbitrary_cad_kernel"},
                "mesh": {"type": "snappyHexMesh"},
                "boundaries": [{"name": "badPatch", "role": "magic"}],
                "solver": {"name": "badSolver"},
                "physics": {"domain": "plasma_magic"},
            }
        }
        return data


class SimulationSpecTests(unittest.TestCase):
    def test_simulation_spec_round_trip_preserves_expanded_sections(self) -> None:
        spec = SimulationSpec(
            geometry=GeometrySpec("rect_channel", {"length": 4.0, "height": 1.0}),
            mesh=MeshSpec("blockMesh", {"cells": [80, 20, 1]}),
            boundaries=[
                BoundarySpec("inlet", "velocity_inlet", {"velocity": [1.0, 0.0, 0.0]}),
                BoundarySpec("outlet", "pressure_outlet", {"pressure": 0.0}),
            ],
            solver=SolverSpec("icoFoam", "transient_incompressible"),
            generation_mode="generated_case",
            physics=PhysicsSpec("incompressible", "newtonian", {"kinematic_viscosity": 0.01}),
            numerics=NumericsSpec("PISO", {"start_time": 0, "end_time": 1, "delta_t": 0.005}),
            outputs=OutputsSpec("openfoam", "vtk", ["U", "p"], {"write_interval": 20}),
        )

        round_tripped = SimulationSpec.from_dict(spec.to_dict())

        self.assertEqual(spec.to_dict(), round_tripped.to_dict())

    def test_simulation_spec_defaults_keep_legacy_dicts_compatible(self) -> None:
        spec = SimulationSpec.from_dict(
            {
                "geometry": {"type": "reference_case", "source": "cavity"},
                "mesh": {"type": "reference_mesh"},
                "boundaries": [{"name": "movingWall", "role": "wall"}],
                "solver": {"name": "icoFoam", "case_type": "transient_incompressible"},
            }
        )

        self.assertEqual("reference_modify", spec.generation_mode)
        self.assertEqual("", spec.physics.domain)
        self.assertEqual({}, spec.numerics.time)
        self.assertEqual("openfoam", spec.outputs.format)
        self.assertEqual([], spec.outputs.fields)

    def test_simulation_spec_gate_rejects_illegal_expanded_values(self) -> None:
        spec = SimulationSpec(
            geometry=GeometrySpec("rect_channel", {"length": 1.0}),
            mesh=MeshSpec("blockMesh"),
            boundaries=[BoundarySpec("inlet", "velocity_inlet")],
            solver=SolverSpec("icoFoam", "transient_incompressible"),
            physics=PhysicsSpec("plasma_magic"),
            numerics=NumericsSpec("PISO", {"delta_t": 0}),
            outputs=OutputsSpec("hdf5"),
        )

        result = review_simulation_spec(spec)
        codes = [item.code for item in result.issues]

        self.assertEqual("failed", result.status)
        self.assertIn("physics.unsupported_domain", codes)
        self.assertIn("numerics.non_positive_delta_t", codes)
        self.assertIn("outputs.unsupported_format", codes)

    def test_openfoam_plan_always_contains_canonical_simulation_spec(self) -> None:
        plan = create_openfoam_plan("create an icoFoam cavity case", llm=FakePlanningLLM())
        spec = plan.parameters.get("simulation_spec", {})

        self.assertEqual("reference_case", spec["geometry"]["type"])
        self.assertEqual("reference_mesh", spec["mesh"]["type"])
        self.assertEqual("incompressible", spec["physics"]["domain"])
        self.assertEqual("PISO", spec["numerics"]["algorithm"])
        self.assertEqual(["U", "p"], spec["outputs"]["fields"])
        self.assertEqual("vtk", spec["outputs"]["result_format"])

    def test_openfoam_reference_plan_ignores_llm_supplied_simulation_spec(self) -> None:
        plan = create_openfoam_plan("create an icoFoam cavity case", llm=PollutedSpecLLM())
        spec = plan.parameters.get("simulation_spec", {})

        self.assertEqual("reference_case", spec["geometry"]["type"])
        self.assertEqual("cavity", spec["geometry"]["source"])
        self.assertEqual("reference_mesh", spec["mesh"]["type"])
        self.assertEqual("icoFoam", spec["solver"]["name"])
        self.assertEqual("incompressible", spec["physics"]["domain"])


if __name__ == "__main__":
    unittest.main()
