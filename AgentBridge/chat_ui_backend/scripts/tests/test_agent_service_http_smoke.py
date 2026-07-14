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

    def assert_allowed_actions(self, payload: dict, expected: list[str]) -> None:
        self.assertIn("allowed_actions", payload)
        self.assertEqual(expected, payload["allowed_actions"])

    def assert_invalid_workflow_state(
        self,
        path: str,
        task: TaskContext,
        requested_action: str,
        allowed_actions: list[str],
        extra_payload: dict | None = None,
    ) -> None:
        store = agent_service.TaskStore()
        store.save_task(task)
        context_path = Path(task.context_path)
        manifest_path = Path(task.manifest_path)
        context_before = context_path.read_bytes()
        manifest_before = manifest_path.read_bytes() if manifest_path.exists() else None
        payload = {"task_dir": task.task_dir}
        payload.update(extra_payload or {})

        status, response = self.post_json(path, payload)

        self.assertEqual(409, status)
        self.assertEqual("invalid_workflow_state", response["error_code"])
        self.assertEqual(task.status, response["current_status"])
        self.assertEqual(requested_action, response["requested_action"])
        self.assertEqual(allowed_actions, response["allowed_actions"])
        self.assertTrue(response["error"])
        self.assertEqual(context_before, context_path.read_bytes())
        if manifest_before is None:
            self.assertFalse(manifest_path.exists())
        else:
            self.assertEqual(manifest_before, manifest_path.read_bytes())

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

    def test_plan_rejects_non_positive_rect_channel_height_with_failed_task(self) -> None:
        status, payload = self.post_json(
            "/foam/plan",
            {
                "message": (
                    "\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u901a\u9053\uff0c\u957f\u5ea6 5m\uff0c\u9ad8\u5ea6 -1m\uff0c"
                    "\u5165\u53e3\u901f\u5ea6 1m/s\uff0c\u51fa\u53e3\u538b\u529b 0\uff0c\u8fd0\u52a8\u9ecf\u5ea6 0.01"
                ),
                "output_dir": self.output_dir,
                "host_output_dir": self.output_dir,
            },
        )

        self.assertEqual(422, status)
        self.assertEqual("geometry.non_positive_height", payload["code"])
        self.assertEqual("failed", payload["current_status"])
        self.assertEqual([], payload["allowed_actions"])
        self.assertIn("height", payload["error"])
        self.assertIn("-1", payload["error"])
        self.assertNotIn("Traceback", payload["error"])
        self.assertNotIn(self.output_dir, payload["error"])

        task_dirs = list(Path(self.output_dir).glob("task_*"))
        self.assertEqual(1, len(task_dirs))
        task_context = json.loads((task_dirs[0] / "task_context.json").read_text(encoding="utf-8"))
        manifest = json.loads((task_dirs[0] / "agent_manifest_v1.json").read_text(encoding="utf-8"))

        self.assertEqual("failed", task_context["status"])
        self.assertEqual({}, task_context["plan"])
        self.assertEqual([], task_context["generated_files"])
        self.assertEqual(payload["error"], task_context["error_message"])
        self.assertEqual("failed", manifest["workflow"]["status"])
        self.assertEqual(payload["error"], manifest["workflow"]["error"])

    def test_rect_channel_http_plan_generate_validate_uses_generated_route(self) -> None:
        status, planned = self.post_json(
            "/foam/plan",
            {
                "message": (
                    "\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u901a\u9053\uff0c\u957f\u5ea65m\uff0c\u9ad8\u5ea61m\uff0c"
                    "\u5165\u53e3\u901f\u5ea61m/s\uff0c\u51fa\u53e3\u538b\u529b0\uff0c\u8fd0\u52a8\u7c98\u5ea60.01"
                ),
                "output_dir": self.output_dir,
                "host_output_dir": self.output_dir,
            },
        )

        self.assertEqual(200, status)
        self.assertEqual("planned", planned["task"]["status"])
        self.assertEqual("generated_case", planned["plan"]["parameters"]["route_decision"]["selected_mode"])
        self.assertEqual("generated_case", planned["plan"]["generation_mode"])
        self.assertEqual("rect_channel", planned["plan"]["case_name"])
        self.assertEqual("icoFoam", planned["plan"]["solver_name"])
        self.assert_allowed_actions(planned, ["generate", "replan", "repair_action"])

        task_dir = planned["task"]["task_dir"]
        status, generated = self.post_json("/foam/generate", {"task_dir": task_dir})

        self.assertEqual(200, status)
        self.assertEqual("generated", generated["task"]["status"])
        generation_gate = next(
            item for item in generated["task"]["gate_reviews"] if item["gate"] == "generation"
        )
        self.assertEqual("passed", generation_gate["status"])
        self.assert_allowed_actions(
            generated,
            ["generate", "validate", "replan", "repair_action"],
        )

        status, validated = self.post_json("/foam/validate", {"task_dir": task_dir})

        self.assertEqual(200, status)
        self.assertEqual("validated", validated["task"]["status"])
        self.assertEqual("validated", validated["validation"]["status"])
        self.assertEqual([], validated["validation"]["missing_files"])
        self.assert_allowed_actions(
            validated,
            ["generate", "validate", "run", "replan", "repair_action"],
        )

    def test_rect_channel_http_replan_preserves_generated_route(self) -> None:
        status, planned = self.post_json(
            "/foam/plan",
            {
                "message": (
                    "\u751f\u6210\u4e00\u4e2a\u4e8c\u7ef4\u77e9\u5f62\u901a\u9053\uff0c\u957f\u5ea65m\uff0c\u9ad8\u5ea61m\uff0c"
                    "\u5165\u53e3\u901f\u5ea61m/s\uff0c\u51fa\u53e3\u538b\u529b0\uff0c\u8fd0\u52a8\u7c98\u5ea60.01"
                ),
                "output_dir": self.output_dir,
                "host_output_dir": self.output_dir,
            },
        )
        self.assertEqual(200, status)

        task_dir = planned["task"]["task_dir"]
        status, replanned = self.post_json(
            "/foam/replan",
            {"task_dir": task_dir, "message": "end_time=2.0"},
        )

        self.assertEqual(200, status)
        self.assertEqual("updated", replanned["replan"]["status"])
        self.assertEqual("planned", replanned["task"]["status"])
        self.assertEqual("generate", replanned["next_action"]["id"])
        self.assert_allowed_actions(replanned, ["generate", "replan", "repair_action"])
        self.assertEqual("generated_case", replanned["plan"]["generation_mode"])
        self.assertEqual("rect_channel", replanned["plan"]["case_name"])
        self.assertEqual("icoFoam", replanned["plan"]["solver_name"])
        self.assertEqual(
            "generated_case",
            replanned["plan"]["parameters"]["route_decision"]["selected_mode"],
        )
        spec = replanned["plan"]["parameters"]["simulation_spec"]
        self.assertEqual("rect_channel", spec["geometry"]["type"])
        self.assertEqual(2.0, spec["numerics"]["time"]["end_time"])
        manifest = json.loads(Path(replanned["task"]["manifest_path"]).read_text(encoding="utf-8"))
        self.assertEqual("partial", manifest["workflow"]["status"])
        self.assertEqual("2.0", manifest["solver_settings"]["time"]["end_time"])

    def test_actions_requiring_plan_reject_task_without_plan(self) -> None:
        task = self.make_task()
        task.status = "context_created"
        task.plan = {}

        cases = [
            ("/foam/generate", "generate", {}),
            ("/foam/validate", "validate", {}),
            ("/foam/run", "run", {}),
            ("/foam/replan", "replan", {"message": "set end_time to 1.0"}),
        ]
        for path, action, extra_payload in cases:
            with self.subTest(action=action):
                self.assert_invalid_workflow_state(
                    path,
                    task,
                    action,
                    ["repair_action"],
                    extra_payload,
                )

    def test_validate_rejects_task_before_case_generation(self) -> None:
        task = self.make_task()

        self.assert_invalid_workflow_state(
            "/foam/validate",
            task,
            "validate",
            ["generate", "replan", "repair_action"],
        )

    def test_run_rejects_generated_but_unvalidated_case(self) -> None:
        task = self.make_task()
        task.status = "generated"
        task.generated_files = [{"path": "case/system/controlDict"}]
        task.plan["generated_files"] = task.generated_files

        self.assert_invalid_workflow_state(
            "/foam/run",
            task,
            "run",
            ["generate", "validate", "replan", "repair_action"],
        )

    def test_failed_task_rejects_stale_generated_and_validated_state(self) -> None:
        task = self.make_task()
        task.status = "failed"
        task.generated_files = [{"path": "case/system/controlDict"}]
        task.validation = {"status": "validated", "missing_files": []}
        task.plan["generated_files"] = task.generated_files
        task.plan["validation"] = task.validation

        for path, action in (("/foam/validate", "validate"), ("/foam/run", "run")):
            with self.subTest(action=action):
                self.assert_invalid_workflow_state(
                    path,
                    task,
                    action,
                    ["generate", "replan", "repair_action"],
                )

    def test_run_outcome_states_allow_rerun(self) -> None:
        task = self.make_task()
        task.generated_files = [{"path": "case/system/controlDict"}]
        task.validation = {"status": "validated", "missing_files": []}
        task.plan["generated_files"] = task.generated_files
        task.plan["validation"] = task.validation

        with patch("agent_service.local_runner_node", return_value={"status": "run_completed"}) as runner:
            for current_status in ("run_failed", "run_blocked", "run_completed"):
                with self.subTest(current_status=current_status):
                    task.status = current_status
                    agent_service.TaskStore().save_task(task)

                    status, payload = self.post_json("/foam/run", {"task_dir": task.task_dir})

                    self.assertEqual(200, status)
                    self.assertEqual("run_completed", payload["run"]["status"])
                    self.assert_allowed_actions(
                        payload,
                        ["generate", "validate", "run", "replan", "repair_action"],
                    )

        self.assertEqual(3, runner.call_count)

    def test_in_progress_states_reject_conflicting_actions(self) -> None:
        task = self.make_task()
        cases = [
            ("planning", "/foam/generate", "generate", {}),
            ("loading_reference", "/foam/validate", "validate", {}),
            ("generating_files", "/foam/run", "run", {}),
            ("validating", "/foam/replan", "replan", {"message": "set end_time to 1.0"}),
            (
                "running",
                "/foam/repair-action",
                "repair_action",
                {"repair_action": {"id": "inspect_solver_numerics"}},
            ),
        ]
        for current_status, path, action, extra_payload in cases:
            with self.subTest(current_status=current_status, action=action):
                task.status = current_status
                self.assert_invalid_workflow_state(path, task, action, [], extra_payload)

    def test_validate_response_includes_current_repair_action_on_failure(self) -> None:
        task = self.make_task()
        task.status = "generated"
        task.generated_files = [{"path": "case/system/controlDict"}]
        task.plan["generated_files"] = task.generated_files
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
        self.assert_allowed_actions(
            payload,
            ["generate", "validate", "replan", "repair_action"],
        )

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

    def test_run_response_requires_manifest_import_readiness(self) -> None:
        cases = [
            {"status": "run_completed"},
            {
                "status": "run_completed",
                "manifest_validation": {
                    "status": "valid",
                    "import_ready": False,
                    "import_blockers": [{"code": "manifest.repair_required", "message": "repair"}],
                },
            },
        ]
        for run_result in cases:
            with self.subTest(has_validation="manifest_validation" in run_result):
                task = self.make_task()
                with patch("agent_service.local_runner_node", return_value=run_result):
                    payload = agent_service.handle_run(
                        task,
                        agent_service.TaskStore(),
                        agent_service.ManifestWriter(),
                    )

                self.assertEqual({}, payload["next_action"])

    def test_run_endpoint_returns_runtime_info_when_openfoam_unavailable(self) -> None:
        task = self.make_task()
        task.status = "validated"
        task.generated_files = [{"path": "case/system/controlDict"}]
        task.validation = {"status": "validated", "missing_files": []}
        task.plan["generated_files"] = task.generated_files
        task.plan["validation"] = task.validation
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
        manifest = json.loads(Path(task.manifest_path).read_text(encoding="utf-8"))
        self.assertFalse(manifest["appflow_hints"]["import_ready"])
        manifest_gate = next(
            review
            for review in manifest["workflow"]["gates"]["reviews"]
            if review["gate"] == "manifest"
        )
        self.assertEqual("failed", manifest_gate["status"])
        self.assertIn("manifest.run_not_completed", {item["code"] for item in manifest_gate["issues"]})

    def test_repair_action_endpoint_records_history(self) -> None:
        task = self.make_task()
        task.status = "run_failed"
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
        self.assertEqual("regenerate_case", payload["repair_history"][0]["repair_action_id"])
        self.assertEqual("recorded", payload["repair_history"][0]["status"])
        self.assertEqual("manual", payload["repair_history"][0]["repair_mode"])
        self.assertNotIn("patch_result", payload["repair_history"][0])
        saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))
        manifest = json.loads(Path(task.manifest_path).read_text(encoding="utf-8"))
        self.assertEqual(saved["repair_history"], saved["plan"]["repair_history"])
        self.assertEqual(saved["repair_history"], manifest["workflow"]["repair_history"])
        self.assertEqual("regenerate_case", manifest["workflow"]["last_repair_action"]["repair_action_id"])
        self.assertTrue(manifest["workflow"]["last_repair_action"]["should_rerun"])
        self.assertTrue(manifest["appflow_hints"]["offer_rerun"])
        self.assert_allowed_actions(
            payload,
            ["generate", "replan", "repair_action"],
        )

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
        self.assertEqual("applied", payload["repair_history"][0]["status"])
        self.assertEqual("executable", payload["repair_history"][0]["repair_mode"])
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
        task.status = "validation_failed"
        task.generated_files = [{"path": "case/0/U"}]
        task.validation = {"status": "validation_failed", "missing_files": ["case/0/p"]}
        task.plan["generated_files"] = task.generated_files
        task.plan["validation"] = task.validation
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
        self.assert_allowed_actions(
            payload,
            ["generate", "validate", "replan", "repair_action"],
        )
        saved = json.loads(Path(task.context_path).read_text(encoding="utf-8"))
        manifest = json.loads(Path(task.manifest_path).read_text(encoding="utf-8"))
        self.assertEqual("regenerate_case", saved["repair_history"][0]["action_id"])
        self.assertEqual(saved["repair_history"], manifest["workflow"]["repair_history"])

    def test_validate_records_repair_follow_up_in_manifest(self) -> None:
        task = self.make_task()
        task.status = "generated"
        task.generated_files = [{"path": "case/system/controlDict"}]
        task.plan["generated_files"] = task.generated_files
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
        run_result = {
            "status": "run_completed",
            "pipeline_info": {"selected": "basic"},
            "manifest_validation": {
                "status": "valid",
                "import_ready": True,
                "import_blockers": [],
            },
        }
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
            self.assert_allowed_actions(plan, ["generate", "replan", "repair_action"])
            task_dir = plan["task"]["task_dir"]

            status, generated = self.post_json("/foam/generate", {"task_dir": task_dir})

            self.assertEqual(200, status)
            self.assertIn("reply", generated)
            self.assertIn("task", generated)
            self.assertEqual(reference_files, generated["reference_files"])
            self.assertEqual(generated_files, generated["generated_files"])
            self.assert_next_action(generated, "validate", "/foam/validate")
            self.assert_allowed_actions(
                generated,
                ["generate", "validate", "replan", "repair_action"],
            )

            status, validation = self.post_json("/foam/validate", {"task_dir": task_dir})

            self.assertEqual(200, status)
            self.assertIn("reply", validation)
            self.assertIn("task", validation)
            self.assertEqual(validation_result, validation["validation"])
            self.assertEqual({}, validation["repair_action"])
            self.assert_next_action(validation, "run", "/foam/run")
            self.assert_allowed_actions(
                validation,
                ["generate", "validate", "run", "replan", "repair_action"],
            )

            status, run = self.post_json("/foam/run", {"task_dir": task_dir})

            self.assertEqual(200, status)
            self.assertIn("reply", run)
            self.assertIn("task", run)
            self.assertEqual(run_result, run["run"])
            self.assertEqual({}, run["repair_action"])
            self.assert_next_action(run, "import_manifest", "appflow://import_manifest")
            self.assert_allowed_actions(
                run,
                ["generate", "validate", "run", "replan", "repair_action"],
            )

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
            self.assert_allowed_actions(
                replan,
                ["generate", "validate", "run", "replan", "repair_action"],
            )

        call_provider.assert_called_once()
        planner_node.assert_called_once()
        generate_case_node.assert_called_once()
        validator_node.assert_called_once()
        local_runner_node.assert_called_once()
        apply_replan.assert_called_once()


if __name__ == "__main__":
    unittest.main()
