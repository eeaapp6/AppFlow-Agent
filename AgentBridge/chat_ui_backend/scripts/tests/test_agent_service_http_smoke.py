import json
import sys
import tempfile
import threading
import unittest
import urllib.error
import urllib.request
from http.server import ThreadingHTTPServer
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
SCRIPTS = ROOT / "scripts"
if str(SCRIPTS) not in sys.path:
    sys.path.insert(0, str(SCRIPTS))

import agent_service
from simagent_core.models import SimulationPlan, TaskContext


class FakePlan:
    def __init__(self, requested_changes: dict | None = None):
        self._data = {
            "solver_family": "openfoam",
            "solver_name": "icoFoam",
            "physics_domain": "incompressible",
            "case_category": "cavity",
            "case_name": "cavity",
            "description": "HTTP smoke plan",
            "generation_mode": "reference_modify",
            "requested_changes": requested_changes or {"end_time": "0.5"},
            "parameters": {
                "requested_changes": requested_changes or {"end_time": "0.5"},
            },
            "planned_files": [],
        }

    def to_dict(self) -> dict:
        return dict(self._data)


class AgentServiceHttpSmokeTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.output_dir = self.tmp.name
        self.server = ThreadingHTTPServer(("127.0.0.1", 0), agent_service.AgentHandler)
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        host, port = self.server.server_address
        self.base_url = f"http://{host}:{port}"

    def tearDown(self) -> None:
        self.server.shutdown()
        self.server.server_close()
        self.thread.join(timeout=5)
        self.tmp.cleanup()

    def get_json(self, path: str) -> tuple[int, dict]:
        try:
            with urllib.request.urlopen(f"{self.base_url}{path}", timeout=5) as response:
                return response.status, json.loads(response.read().decode("utf-8"))
        except urllib.error.HTTPError as exc:
            return exc.code, json.loads(exc.read().decode("utf-8"))

    def post_json(self, path: str, payload: dict) -> tuple[int, dict]:
        data = json.dumps(payload).encode("utf-8")
        request = urllib.request.Request(
            f"{self.base_url}{path}",
            data=data,
            method="POST",
            headers={"Content-Type": "application/json"},
        )
        try:
            with urllib.request.urlopen(request, timeout=5) as response:
                return response.status, json.loads(response.read().decode("utf-8"))
        except urllib.error.HTTPError as exc:
            return exc.code, json.loads(exc.read().decode("utf-8"))

    def assert_next_action(self, payload: dict, action_id: str, endpoint: str) -> None:
        self.assertIn("next_action", payload)
        action = payload["next_action"]
        self.assertEqual(action_id, action["id"])
        self.assertEqual(endpoint, action["endpoint"])
        self.assertIs(action["requires_confirmation"], True)
        self.assertIn("label", action)

    def make_task(self) -> TaskContext:
        task_dir = Path(self.output_dir) / "task_http"
        task_dir.mkdir(parents=True, exist_ok=True)
        return TaskContext(
            version=1,
            task_id="task_http",
            status="planned",
            user_requirement="test",
            solver_family="openfoam",
            output_root=self.output_dir,
            host_output_root=self.output_dir,
            task_dir=str(task_dir),
            case_dir=str(task_dir / "case"),
            mesh_dir=str(task_dir / "mesh"),
            results_dir=str(task_dir / "results"),
            logs_dir=str(task_dir / "logs"),
            context_path=str(task_dir / "task_context.json"),
            manifest_path=str(task_dir / "agent_manifest_v1.json"),
            plan=FakePlan().to_dict(),
        )

    def test_health_and_unknown_path_contract(self) -> None:
        status, payload = self.get_json("/health")

        self.assertEqual(200, status)
        self.assertEqual("ok", payload["status"])
        self.assertIn("provider", payload)
        self.assertEqual("simagent_core", payload["core"])

        status, payload = self.post_json("/unknown", {})

        self.assertEqual(404, status)
        self.assertEqual({"error": "not found"}, payload)

    def test_missing_required_fields_contract(self) -> None:
        status, payload = self.post_json("/foam/plan", {})

        self.assertEqual(400, status)
        self.assertEqual({"error": "Message must not be empty."}, payload)

        status, payload = self.post_json("/foam/generate", {})

        self.assertEqual(400, status)
        self.assertEqual({"error": "task_dir must not be empty."}, payload)

    def test_validate_response_includes_current_repair_action_on_failure(self) -> None:
        task = self.make_task()
        validation_result = {"status": "validation_failed", "missing_files": ["case/0/p"]}

        def fake_validator_node(task, task_store, manifest_writer):
            task_store.attach_gate_review(
                task,
                {
                    "gate": "static_validation",
                    "status": "failed",
                    "issues": [{"code": "validation.missing_file", "message": "Missing required file: case/0/p."}],
                },
                save=False,
            )
            task_store.attach_validation_result(task, validation_result)
            return validation_result

        with patch("agent_service.validator_node", side_effect=fake_validator_node):
            payload = agent_service.handle_validate(task, agent_service.TaskStore(), agent_service.ManifestWriter())

        self.assertEqual("regenerate_case", payload["repair_action"]["id"])
        self.assertEqual({}, payload["next_action"])

    def test_run_response_blocks_import_when_result_review_needs_repair(self) -> None:
        task = self.make_task()
        task.gate_reviews = [
            {
                "gate": "result_review",
                "status": "failed",
                "issues": [{"code": "result.residual_high", "message": "residual high"}],
                "next_repair_action": {"id": "inspect_solver_numerics", "label": "Inspect solver numerics"},
                "diagnostics": {
                    "severity": "failed",
                    "categories": ["numerics"],
                    "primary_issue": {
                        "code": "result.residual_high",
                        "message": "residual high",
                        "severity": "error",
                    },
                },
            }
        ]

        with patch("agent_service.local_runner_node", return_value={"status": "run_completed"}):
            payload = agent_service.handle_run(task, agent_service.TaskStore(), agent_service.ManifestWriter())

        self.assertEqual("inspect_solver_numerics", payload["repair_action"]["id"])
        self.assertEqual({}, payload["next_action"])
        self.assertIn("Result review failed.", payload["reply"])

    def test_run_endpoint_returns_runtime_info_when_openfoam_unavailable(self) -> None:
        task = self.make_task()
        for rel_path in [
            "case/system/controlDict",
            "case/system/fvSchemes",
            "case/system/fvSolution",
            "case/system/blockMeshDict",
            "case/constant/physicalProperties",
            "case/0/U",
            "case/0/p",
        ]:
            path = Path(task.task_dir) / rel_path
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("FoamFile{}\n", encoding="utf-8")
        agent_service.TaskStore().save_task(task)
        runtime_info = {
            "mode": "unavailable",
            "backend": "local",
            "available": False,
            "reason": "OpenFOAM runtime is not configured. WM_PROJECT_DIR is not set.",
            "missing_commands": ["blockMesh"],
            "wm_project_dir": "",
            "commands": {"blockMesh": {"available": False}, "icoFoam": {"available": True}},
        }

        with patch("simagent_core.solvers.openfoam.run_local.detect_openfoam_runtime", return_value=runtime_info):
            status, payload = self.post_json("/foam/run", {"task_dir": task.task_dir})

        self.assertEqual(200, status)
        self.assertEqual("run_blocked", payload["run"]["status"])
        self.assertEqual(runtime_info, payload["run"]["runtime_info"])
        self.assertEqual({}, payload["next_action"])
        self.assertIn("WM_PROJECT_DIR", payload["reply"])

    def test_repair_action_endpoint_records_history(self) -> None:
        task = self.make_task()
        task_store = agent_service.TaskStore()
        task_store.save_task(task)

        status, payload = self.post_json(
            "/foam/repair-action",
            {
                "task_dir": task.task_dir,
                "repair_action": {
                    "id": "regenerate_case",
                    "label": "Regenerate case",
                },
            },
        )

        self.assertEqual(200, status)
        self.assertEqual("regenerate_case", payload["repair_history"][0]["action_id"])
        saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))
        manifest = json.loads(Path(task.manifest_path).read_text(encoding="utf-8"))
        self.assertEqual(saved["repair_history"], saved["plan"]["repair_history"])
        self.assertEqual(saved["repair_history"], manifest["workflow"]["repair_history"])

    def test_repair_action_endpoint_applies_safe_patch(self) -> None:
        task = self.make_task()
        control_path = Path(task.task_dir) / "case/system/controlDict"
        control_path.parent.mkdir(parents=True)
        control_path.write_text(
            """FoamFile
{
    object controlDict;
}

deltaT          0.005;
""",
            encoding="utf-8",
        )
        agent_service.TaskStore().save_task(task)

        status, payload = self.post_json(
            "/foam/repair-action",
            {
                "task_dir": task.task_dir,
                "repair_action": {
                    "id": "stabilize_time_step",
                    "label": "Stabilize time step",
                    "patches": [
                        {
                            "op": "adjust_time_step",
                            "path": "case/system/controlDict",
                            "value": "0.001",
                        }
                    ],
                },
            },
        )

        self.assertEqual(200, status)
        self.assertIn("deltaT          0.001;", control_path.read_text(encoding="utf-8"))
        self.assertEqual("applied", payload["repair_history"][0]["patch_result"]["status"])

    def test_repair_action_endpoint_rejects_path_escape_patch(self) -> None:
        task = self.make_task()
        control_path = Path(task.task_dir) / "case/system/controlDict"
        control_path.parent.mkdir(parents=True)
        control_path.write_text(
            """FoamFile
{
    object controlDict;
}

deltaT          0.005;
""",
            encoding="utf-8",
        )
        agent_service.TaskStore().save_task(task)

        status, payload = self.post_json(
            "/foam/repair-action",
            {
                "task_dir": task.task_dir,
                "repair_action": {
                    "id": "bad",
                    "label": "Bad",
                    "patches": [
                        {
                            "op": "adjust_time_step",
                            "path": "case/system/controlDict",
                            "value": "0.001",
                        },
                        {
                            "op": "set_dictionary_value",
                            "path": "../outside/controlDict",
                            "key": "deltaT",
                            "value": "0.001",
                        }
                    ],
                },
            },
        )

        self.assertEqual(400, status)
        self.assertIn("escapes task case", payload["error"])
        self.assertIn("deltaT          0.005;", control_path.read_text(encoding="utf-8"))
        saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))
        self.assertEqual([], saved["repair_history"])

    def test_repair_action_endpoint_rejects_schema_error_without_history(self) -> None:
        task = self.make_task()
        control_path = Path(task.task_dir) / "case/system/controlDict"
        control_path.parent.mkdir(parents=True)
        control_path.write_text(
            """FoamFile
{
    object controlDict;
}

deltaT          0.005;
""",
            encoding="utf-8",
        )
        agent_service.TaskStore().save_task(task)

        status, payload = self.post_json(
            "/foam/repair-action",
            {
                "task_dir": task.task_dir,
                "repair_action": {
                    "id": "bad",
                    "label": "Bad",
                    "patches": [
                        {
                            "op": "set_dictionary_value",
                            "path": "case/system/controlDict",
                            "key": "deltaT; hacked",
                            "value": "0.001",
                        }
                    ],
                },
            },
        )

        self.assertEqual(400, status)
        self.assertIn("OpenFOAM identifier", payload["error"])
        self.assertIn("deltaT          0.005;", control_path.read_text(encoding="utf-8"))
        saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))
        self.assertEqual([], saved["repair_history"])

    def test_generate_records_optional_repair_action_before_running(self) -> None:
        task = self.make_task()
        agent_service.TaskStore().save_task(task)
        generated_files = [{"path": "case/system/controlDict", "role": "control", "format": "openfoam-dict"}]

        def fake_generate_case_node(task, task_store, manifest_writer):
            task_store.attach_generated_files(task, generated_files)
            manifest_writer.write_generated(task, SimulationPlan.from_dict(task.plan), generated_files)
            return [], generated_files

        with patch("agent_service.generate_case_node", side_effect=fake_generate_case_node):
            status, payload = self.post_json(
                "/foam/generate",
                {
                    "task_dir": task.task_dir,
                    "repair_action": {
                        "id": "regenerate_case",
                        "label": "Regenerate case",
                    },
                },
            )

        self.assertEqual(200, status)
        self.assertEqual(generated_files, payload["generated_files"])
        saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))
        manifest = json.loads(Path(task.manifest_path).read_text(encoding="utf-8"))
        self.assertEqual("regenerate_case", saved["repair_history"][0]["action_id"])
        self.assertEqual(saved["repair_history"], manifest["workflow"]["repair_history"])

    def test_validate_records_repair_follow_up_in_manifest(self) -> None:
        task = self.make_task()
        task.repair_history = [
            {
                "action_id": "regenerate_case",
                "label": "Regenerate case",
                "status": "accepted",
                "source_gate": "static_validation",
                "created_at": "2026-05-23T00:00:00+08:00",
            }
        ]
        task.plan["repair_history"] = task.repair_history
        agent_service.TaskStore().save_task(task)
        validation_result = {"status": "validated", "missing_files": [], "dictionary_errors": []}

        def fake_validator_node(task, task_store, manifest_writer):
            task_store.attach_validation_result(task, validation_result)
            manifest_writer.write_validation_result(task, SimulationPlan.from_dict(task.plan), validation_result)
            return validation_result

        with patch("agent_service.validator_node", side_effect=fake_validator_node):
            status, payload = self.post_json("/foam/validate", {"task_dir": task.task_dir})

        self.assertEqual(200, status)
        self.assertEqual(validation_result, payload["validation"])
        saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))
        manifest = json.loads(Path(task.manifest_path).read_text(encoding="utf-8"))
        self.assertEqual("validated", saved["repair_history"][0]["follow_up"]["validation_status"])
        self.assertEqual(saved["repair_history"], manifest["workflow"]["repair_history"])

    def test_endpoint_contracts_with_mocked_runtime_dependencies(self) -> None:
        fake_plan = FakePlan()
        reference_files = [
            {"path": "case/system/controlDict", "role": "control", "format": "openfoam-dict"},
        ]
        generated_files = [
            {"path": "case/system/controlDict", "role": "control", "format": "openfoam-dict"},
        ]
        validation_result = {"status": "validated", "missing_files": [], "dictionary_errors": []}
        run_result = {"status": "run_completed", "pipeline_info": {"selected": "basic"}}
        replan_plan = FakePlan({"end_time": "1.0"}).to_dict()
        replan_result = {
            "status": "updated",
            "applied_changes": {"end_time": "1.0"},
            "unrecognized_changes": {},
        }

        def fake_planner_node(task, task_store, manifest_writer, config):
            task_store.attach_plan(task, SimulationPlan.from_dict(fake_plan.to_dict()))
            manifest_writer.write_planned(task, SimulationPlan.from_dict(fake_plan.to_dict()))
            return fake_plan

        def fake_generate_case_node(task, task_store, manifest_writer):
            task_store.attach_generated_files(task, generated_files)
            return reference_files, generated_files

        def fake_validator_node(task, task_store, manifest_writer):
            task_store.attach_validation_result(task, validation_result)
            return validation_result

        def fake_local_runner_node(task, task_store, manifest_writer):
            task_store.attach_run_result(task, run_result)
            return run_result

        with (
            patch("agent_service.call_provider", return_value="mock chat reply") as call_provider,
            patch("agent_service.planner_node", side_effect=fake_planner_node) as planner_node,
            patch("agent_service.generate_case_node", side_effect=fake_generate_case_node) as generate_case_node,
            patch("agent_service.validator_node", side_effect=fake_validator_node) as validator_node,
            patch("agent_service.local_runner_node", side_effect=fake_local_runner_node) as local_runner_node,
            patch("agent_service.apply_replan", return_value=(replan_plan, replan_result)) as apply_replan,
        ):
            status, chat = self.post_json(
                "/chat",
                {
                    "message": "hello",
                    "output_dir": self.output_dir,
                    "host_output_dir": self.output_dir,
                },
            )

            self.assertEqual(200, status)
            self.assertEqual("mock chat reply", chat["reply"])
            self.assertIn("task", chat)
            self.assertIn("task_dir", chat["task"])

            status, plan = self.post_json(
                "/foam/plan",
                {
                    "message": "create cavity case",
                    "output_dir": self.output_dir,
                    "host_output_dir": self.output_dir,
                },
            )

            self.assertEqual(200, status)
            self.assertIn("reply", plan)
            self.assertIn("task", plan)
            self.assertIn("plan", plan)
            self.assert_next_action(plan, "generate", "/foam/generate")
            task_dir = plan["task"]["task_dir"]

            status, generated = self.post_json("/foam/generate", {"task_dir": task_dir})

            self.assertEqual(200, status)
            self.assertIn("reply", generated)
            self.assertIn("task", generated)
            self.assertEqual(reference_files, generated["reference_files"])
            self.assertEqual(generated_files, generated["generated_files"])
            self.assert_next_action(generated, "validate", "/foam/validate")

            status, validation = self.post_json("/foam/validate", {"task_dir": task_dir})

            self.assertEqual(200, status)
            self.assertIn("reply", validation)
            self.assertIn("task", validation)
            self.assertEqual(validation_result, validation["validation"])
            self.assertEqual({}, validation["repair_action"])
            self.assert_next_action(validation, "run", "/foam/run")

            status, run = self.post_json("/foam/run", {"task_dir": task_dir})

            self.assertEqual(200, status)
            self.assertIn("reply", run)
            self.assertIn("task", run)
            self.assertEqual(run_result, run["run"])
            self.assertEqual({}, run["repair_action"])
            self.assert_next_action(run, "import_manifest", "appflow://import_manifest")

            status, replan = self.post_json(
                "/foam/replan",
                {"task_dir": task_dir, "message": "set end_time to 1.0"},
            )

            self.assertEqual(200, status)
            self.assertIn("reply", replan)
            self.assertIn("task", replan)
            self.assertEqual(replan_plan, replan["plan"])
            self.assertEqual(replan_result, replan["replan"])
            self.assert_next_action(replan, "generate", "/foam/generate")

        call_provider.assert_called_once()
        planner_node.assert_called_once()
        generate_case_node.assert_called_once()
        validator_node.assert_called_once()
        local_runner_node.assert_called_once()
        apply_replan.assert_called_once()


if __name__ == "__main__":
    unittest.main()
