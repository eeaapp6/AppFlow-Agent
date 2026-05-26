import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.models import SimulationPlan, TaskContext
from scripts.simagent_core.solvers.openfoam.input_writer import generate_openfoam_files
from scripts.simagent_core.solvers.openfoam.mesh import detect_gmsh_mesh_request
from scripts.simagent_core.solvers.openfoam.run_pipeline import build_basic_run_pipeline
from scripts.simagent_core.solvers.openfoam.validate import required_openfoam_files


def make_task(root: Path) -> TaskContext:
    task_dir = root / "task_gmsh"
    return TaskContext(
        version=1,
        task_id="task_gmsh",
        status="planned",
        user_requirement="use gmsh mesh",
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


def make_gmsh_plan(source_path: str) -> SimulationPlan:
    return SimulationPlan(
        solver_family="openfoam",
        solver_name="icoFoam",
        physics_domain="incompressible",
        case_category="cavity",
        case_name="cavity",
        description="test gmsh import",
        parameters={
            "mesh": {
                "mode": "gmsh_external",
                "source_path": source_path,
                "case_path": "case/geometry.msh",
            }
        },
    )


class OpenFOAMGmshMeshTests(unittest.TestCase):
    def test_detect_gmsh_mesh_request_extracts_windows_path(self) -> None:
        mesh = detect_gmsh_mesh_request(r"use gmsh mesh C:\cases\mesh\wing.msh")

        self.assertEqual("gmsh_external", mesh["mode"])
        self.assertEqual(r"C:\cases\mesh\wing.msh", mesh["source_path"])
        self.assertEqual("case/geometry.msh", mesh["case_path"])

    def test_generate_openfoam_files_copies_external_gmsh_mesh_and_skips_blockmesh(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "input.msh"
            source.write_text("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n", encoding="utf-8")
            task = make_task(root)
            plan = make_gmsh_plan(str(source))

            generated = generate_openfoam_files(task, plan)

            generated_paths = {item.path for item in generated}
            self.assertIn("case/geometry.msh", generated_paths)
            self.assertNotIn("case/system/blockMeshDict", generated_paths)
            self.assertEqual(source.read_bytes(), (Path(task.case_dir) / "geometry.msh").read_bytes())

    def test_gmsh_plan_requires_geometry_mesh_not_blockmesh_dict(self) -> None:
        plan = make_gmsh_plan("mesh.msh")

        required = required_openfoam_files(plan)

        self.assertIn("case/geometry.msh", required)
        self.assertNotIn("case/system/blockMeshDict", required)

    def test_basic_pipeline_imports_gmsh_mesh_before_solver(self) -> None:
        plan = make_gmsh_plan("mesh.msh")

        pipeline = build_basic_run_pipeline(plan)

        self.assertEqual(["gmshToFoam", "checkMesh", "icoFoam"], [step["command"] for step in pipeline])
        self.assertEqual(["geometry.msh"], pipeline[0]["args"])


if __name__ == "__main__":
    unittest.main()
