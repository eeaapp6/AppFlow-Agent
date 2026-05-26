import json
import tempfile
import unittest
from pathlib import Path

from scripts.simagent_core.models import (
    GeneratedFileSet,
    RunResult,
    SimulationPlan,
    TaskContext,
    ValidationResult,
)
from scripts.simagent_core.state.task_store import TaskStore


def make_task_data(root: Path) -> dict:
    task_dir = root / "task_compat"
    return {
        "version": 1,
        "task_id": "task_compat",
        "status": "planned",
        "user_requirement": "test",
        "solver_family": "openfoam",
        "output_root": str(root),
        "host_output_root": str(root),
        "task_dir": str(task_dir),
        "case_dir": str(task_dir / "case"),
        "mesh_dir": str(task_dir / "mesh"),
        "results_dir": str(task_dir / "results"),
        "logs_dir": str(task_dir / "logs"),
        "context_path": str(task_dir / "task_context.json"),
        "manifest_path": str(task_dir / "agent_manifest_v1.json"),
            "reference_files_path": str(task_dir / "reference_files.json"),
            "created_at": "2026-05-17T00:00:00+08:00",
            "updated_at": "2026-05-17T00:00:00+08:00",
            "gate_reviews": [{"gate": "capability", "status": "passed", "issues": []}],
            "plan": {
                "solver_family": "openfoam",
                "solver_name": "icoFoam",
            "physics_domain": "incompressible",
            "case_category": "cavity",
            "case_name": "cavity",
            "description": "test",
            "generated_files": [{"path": "case/system/controlDict", "role": "control", "format": "openfoam-dict"}],
            "reference_files": [{"path": "case/0/U", "role": "initial_condition", "format": "openfoam-dict"}],
                "reference_files_removed": True,
                "validation": {"status": "validated", "checked_files": ["case/system/controlDict"]},
                "run": {"status": "run_completed", "pipeline_info": {"selected": "basic"}},
                "gate_reviews": [{"gate": "legacy", "status": "passed", "issues": []}],
            },
        }


class StateModelsCompatibilityTests(unittest.TestCase):
    def test_task_context_loads_legacy_plan_embedded_state(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            task = TaskContext.from_dict(make_task_data(Path(tmp)))

        self.assertEqual("case/system/controlDict", task.generated_files[0]["path"])
        self.assertEqual("case/0/U", task.reference_files[0]["path"])
        self.assertTrue(task.reference_files_removed)
        self.assertEqual("validated", task.validation["status"])
        self.assertEqual("run_completed", task.run["status"])
        self.assertEqual("run_completed", task.plan["run"]["status"])
        self.assertEqual("capability", task.gate_reviews[0]["gate"])

    def test_task_context_top_level_state_takes_priority_over_legacy_plan_state(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            data = make_task_data(Path(tmp))
            data["run"] = {"status": "run_blocked", "reason": "top-level"}
            data["gate_reviews"] = [{"gate": "top-level", "status": "passed", "issues": []}]

            task = TaskContext.from_dict(data)

        self.assertEqual("run_blocked", task.run["status"])
        self.assertEqual("run_completed", task.plan["run"]["status"])
        self.assertEqual("top-level", task.gate_reviews[0]["gate"])

    def test_effective_reference_files_path_reads_legacy_plan_path(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            data = make_task_data(Path(tmp))
            legacy_path = data["reference_files_path"]
            data["reference_files_path"] = ""
            data["plan"]["reference_files_path"] = legacy_path

            task = TaskContext.from_dict(data)

        self.assertEqual(legacy_path, task.effective_reference_files_path())

    def test_task_store_double_writes_new_and_legacy_state_fields(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            data = make_task_data(root)
            task_dir = Path(data["task_dir"])
            task_dir.mkdir(parents=True)
            task = TaskContext.from_dict(data)
            store = TaskStore()

            generated = [{"path": "case/0/p", "role": "initial_condition", "format": "openfoam-dict"}]
            reference = [{"path": "case/0/U", "role": "initial_condition", "format": "openfoam-dict"}]
            validation = {"status": "validated", "missing_files": []}
            run = {"status": "run_completed", "pipeline_info": {"mode": "basic"}}
            gate_review = {"gate": "generation", "status": "passed", "issues": []}

            store.attach_reference_files(task, reference)
            store.mark_reference_files_removed(task, True)
            store.attach_generated_files(task, generated)
            store.attach_validation_result(task, validation)
            store.attach_run_result(task, run)
            store.attach_gate_review(task, gate_review)

            reloaded = TaskContext.from_dict(json.loads(Path(task.context_path).read_text(encoding="utf-8")))

        self.assertEqual(generated, reloaded.generated_files)
        self.assertEqual(generated, reloaded.plan["generated_files"])
        self.assertEqual(reference, reloaded.reference_files)
        self.assertEqual(reference, reloaded.plan["reference_files"])
        self.assertTrue(reloaded.reference_files_removed)
        self.assertTrue(reloaded.plan["reference_files_removed"])
        self.assertEqual(validation, reloaded.validation)
        self.assertEqual(validation, reloaded.plan["validation"])
        self.assertEqual(run, reloaded.run)
        self.assertEqual(run, reloaded.plan["run"])
        self.assertEqual(gate_review, reloaded.gate_reviews[-1])
        self.assertEqual(reloaded.gate_reviews, reloaded.plan["gate_reviews"])

    def test_run_result_round_trip_preserves_current_runtime_fields(self) -> None:
        data = {
            "status": "run_completed",
            "reason": "done",
            "steps": [{"command": "blockMesh", "return_code": 0}],
            "pipeline": [{"command": "blockMesh", "args": []}],
            "pipeline_info": {"mode": "allrun_safe", "selected": "basic", "fallback_used": True},
            "logs": [{"path": "logs/blockMesh.log", "role": "blockMesh", "format": "text"}],
            "outputs": {"results": {"latest_path": "case/0.5"}},
            "wm_project_dir": "/opt/openfoam10",
            "missing_files": ["case/system/controlDict"],
            "manifest_validation": {"status": "valid"},
        }

        result = RunResult.from_dict(data).to_dict()

        self.assertEqual(data["pipeline_info"], result["pipeline_info"])
        self.assertEqual(data["pipeline"], result["pipeline"])
        self.assertEqual(data["manifest_validation"], result["manifest_validation"])
        self.assertEqual(data["missing_files"], result["missing_files"])

    def test_validation_result_round_trip_preserves_current_validation_fields(self) -> None:
        data = {
            "status": "validation_failed",
            "checked_files": ["case/0/U"],
            "missing_files": ["case/0/p"],
            "dictionary_errors": [{"file": "case/0/U", "message": "bad object"}],
            "mesh_patches": ["movingWall"],
            "field_patches": {"U": ["movingWall"]},
            "missing_boundary_fields": {"p": ["movingWall"]},
            "extra_boundary_fields": {"U": ["unused"]},
            "warnings": ["test warning"],
        }

        result = ValidationResult.from_dict(data).to_dict()

        self.assertEqual(data, result)

    def test_generated_file_set_list_helpers_preserve_existing_list_shape(self) -> None:
        items = [
            {
                "path": "case/system/controlDict",
                "role": "control",
                "format": "openfoam-dict",
                "source": "reference_modify",
                "modifications": ["end_time"],
                "required": True,
            }
        ]

        self.assertEqual(items, GeneratedFileSet.from_list(items).to_list())

    def test_simulation_plan_reads_explicit_fields_with_parameters_fallback(self) -> None:
        plan = SimulationPlan.from_dict({
            "solver_family": "openfoam",
            "solver_name": "icoFoam",
            "physics_domain": "incompressible",
            "case_category": "cavity",
            "case_name": "cavity",
            "description": "test",
            "unsupported_changes": {"mesh_density": "fine"},
            "parameters": {
                "reference_selection": {"selected": "cavity"},
                "capabilities": {"run_pipeline": "basic"},
                "allrun_reference": {"available": True},
                "allrun_pipeline_preview": {"available": False},
            },
        })

        self.assertEqual({"mesh_density": "fine"}, plan.unsupported_changes)
        self.assertEqual({"selected": "cavity"}, plan.reference_selection)
        self.assertEqual({"run_pipeline": "basic"}, plan.capabilities)
        self.assertEqual({"available": True}, plan.allrun_metadata["allrun_reference"])
        self.assertEqual({"available": False}, plan.allrun_metadata["allrun_pipeline_preview"])


if __name__ == "__main__":
    unittest.main()
