import json
import os
import sys
import urllib.error
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

from simagent_core.agent import (
    active_provider,
    apply_replan,
    call_provider,
    generate_reply,
    next_action,
    parse_replan_changes,
    plan_reply,
    replan_reply,
    run_reply,
    validate_reply,
)
from simagent_core.config import Config
from simagent_core.manifest.writer import ManifestWriter
from simagent_core.models import SimulationPlan
from simagent_core.repair import RepairPatchError, apply_repair_action, current_repair_action
from simagent_core.router_func import select_solver_family
from simagent_core.state.task_store import TaskStore
from simagent_core.workflow.nodes.generate_case_node import generate_case_node
from simagent_core.workflow.nodes.local_runner_node import local_runner_node
from simagent_core.workflow.nodes.planner_node import planner_node
from simagent_core.workflow.nodes.validator_node import validator_node


HOST = os.environ.get("SIMAGENT_HOST", "127.0.0.1").strip() or "127.0.0.1"
PORT = 8765
SUPPORTED_POST_PATHS = {
    "/chat",
    "/foam/plan",
    "/foam/generate",
    "/foam/validate",
    "/foam/run",
    "/foam/replan",
    "/foam/repair-action",
}
TASK_CONTINUATION_PATHS = {
    "/foam/generate",
    "/foam/validate",
    "/foam/run",
    "/foam/replan",
    "/foam/repair-action",
}


class RequestError(Exception):
    def __init__(self, payload: dict, status: int = 400):
        super().__init__(str(payload.get("error", "")))
        self.payload = payload
        self.status = status


def read_json_body(handler: BaseHTTPRequestHandler) -> dict:
    length = int(handler.headers.get("Content-Length", "0"))
    raw_body = handler.rfile.read(length).decode("utf-8")
    return json.loads(raw_body) if raw_body else {}


def require_message(body: dict) -> str:
    message = str(body.get("message", "")).strip()
    if not message:
        raise RequestError({"error": "Message must not be empty."}, status=400)
    return message


def require_task_dir(body: dict) -> str:
    task_dir = str(body.get("task_dir", "")).strip()
    if not task_dir:
        raise RequestError({"error": "task_dir must not be empty."}, status=400)
    return task_dir


def make_runtime() -> tuple[Config, TaskStore, ManifestWriter]:
    config = Config.from_environment()
    return config, TaskStore(config), ManifestWriter()


def has_validation_errors(validation_result: dict) -> bool:
    return bool(
        validation_result.get("missing_files", [])
        or validation_result.get("dictionary_errors", [])
        or validation_result.get("missing_boundary_fields", {})
        or validation_result.get("extra_boundary_fields", {})
        or validation_result.get("empty_boundary_fields", [])
        or validation_result.get("boundary_condition_errors", [])
    )


def handle_plan(task, task_store: TaskStore, manifest_writer: ManifestWriter, config: Config) -> dict:
    plan = planner_node(task, task_store, manifest_writer, config)
    return {
        "reply": plan_reply(plan, task),
        "task": task.to_dict(),
        "plan": plan.to_dict(),
        "next_action": next_action("generate"),
    }


def handle_generate(task, task_store: TaskStore, manifest_writer: ManifestWriter) -> dict:
    reference_files, generated_files = generate_case_node(task, task_store, manifest_writer)
    return {
        "reply": generate_reply(task, generated_files, reference_files),
        "task": task.to_dict(),
        "reference_files": reference_files,
        "generated_files": generated_files,
        "next_action": next_action("validate"),
    }


def handle_validate(task, task_store: TaskStore, manifest_writer: ManifestWriter) -> dict:
    validation_result = validator_node(task, task_store, manifest_writer)
    return {
        "reply": validate_reply(task, validation_result),
        "task": task.to_dict(),
        "validation": validation_result,
        "repair_action": current_repair_action(task),
        "next_action": {} if has_validation_errors(validation_result) else next_action("run"),
    }


def handle_run(task, task_store: TaskStore, manifest_writer: ManifestWriter) -> dict:
    run_result = local_runner_node(task, task_store, manifest_writer)
    run_succeeded = str(run_result.get("status", "")).strip() == "run_completed"
    repair_action = current_repair_action(task)
    return {
        "reply": run_reply(task, run_result),
        "task": task.to_dict(),
        "run": run_result,
        "repair_action": repair_action,
        "next_action": next_action("import_manifest") if run_succeeded and not repair_action else {},
    }


def handle_replan(task, body: dict, task_store: TaskStore, manifest_writer: ManifestWriter) -> dict:
    message = require_message(body)
    plan, replan_result = apply_replan(task, message, task_store)
    manifest_writer.write_planned(task, SimulationPlan.from_dict(plan))
    return {
        "reply": replan_reply(replan_result),
        "task": task.to_dict(),
        "plan": plan,
        "replan": replan_result,
        "next_action": next_action("generate")
        if replan_result.get("status") == "updated"
        else {},
    }


def handle_repair_action(task, body: dict, task_store: TaskStore, manifest_writer: ManifestWriter) -> dict:
    action = body.get("repair_action", {})
    if not isinstance(action, dict) or not str(action.get("id", "")).strip():
        raise RequestError({"error": "repair_action.id must not be empty."}, status=400)

    action = apply_and_record_repair_action(task, action, task_store)
    manifest_writer.write_pending(task)
    return {
        "reply": "",
        "task": task.to_dict(),
        "repair_action": action,
        "repair_history": task.repair_history,
    }


def record_optional_repair_action(task, body: dict, task_store: TaskStore, manifest_writer: ManifestWriter) -> None:
    action = body.get("repair_action", {})
    if not isinstance(action, dict) or not str(action.get("id", "")).strip():
        return
    apply_and_record_repair_action(task, action, task_store)
    manifest_writer.write_pending(task)


def apply_and_record_repair_action(task, action: dict, task_store: TaskStore) -> dict:
    action_with_result = dict(action)
    patches = action.get("patches", [])
    if isinstance(patches, list) and patches:
        try:
            patch_result = apply_repair_action(task, action)
        except RepairPatchError as exc:
            raise RequestError({"error": str(exc)}, status=400) from exc
        action_with_result["patch_result"] = patch_result
    task_store.record_repair_action(task, action_with_result)
    return action_with_result


def handle_chat(message: str, task, task_store: TaskStore) -> dict:
    reply = call_provider(message, task.to_dict())
    task_store.update_status(task, "chat_completed")
    return {"reply": reply, "task": task.to_dict()}


class AgentHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path != "/health":
            self.send_json({"error": "not found"}, status=404)
            return

        self.send_json({"status": "ok", "provider": active_provider(), "core": "simagent_core"})

    def dispatch_existing_task(self, body: dict) -> dict:
        task_dir = require_task_dir(body)
        config, task_store, manifest_writer = make_runtime()
        task = task_store.load_task(task_dir)
        try:
            if self.path == "/foam/replan":
                return handle_replan(task, body, task_store, manifest_writer)

            if self.path == "/foam/repair-action":
                return handle_repair_action(task, body, task_store, manifest_writer)

            if self.path == "/foam/validate":
                record_optional_repair_action(task, body, task_store, manifest_writer)
                return handle_validate(task, task_store, manifest_writer)

            if self.path == "/foam/run":
                record_optional_repair_action(task, body, task_store, manifest_writer)
                return handle_run(task, task_store, manifest_writer)

            record_optional_repair_action(task, body, task_store, manifest_writer)
            return handle_generate(task, task_store, manifest_writer)
        except RequestError:
            raise
        except Exception as exc:
            task_store.update_status(task, "failed", str(exc))
            manifest_writer.write_failed(task, str(exc))
            raise

    def dispatch_new_task(self, body: dict) -> dict:
        message = require_message(body)
        output_dir = str(body.get("output_dir", "")).strip()
        host_output_dir = str(body.get("host_output_dir", "")).strip()
        config, task_store, manifest_writer = make_runtime()
        solver_family = select_solver_family(message, config)
        task = task_store.create_task(message, output_dir, solver_family, host_output_dir)
        manifest_writer.write_pending(task)

        try:
            if self.path == "/foam/plan":
                return handle_plan(task, task_store, manifest_writer, config)

            return handle_chat(message, task, task_store)
        except Exception as exc:
            task_store.update_status(task, "failed", str(exc))
            manifest_writer.write_failed(task, str(exc))
            raise

    def do_POST(self):
        if self.path not in SUPPORTED_POST_PATHS:
            self.send_json({"error": "not found"}, status=404)
            return

        try:
            body = read_json_body(self)
            if self.path in TASK_CONTINUATION_PATHS:
                payload = self.dispatch_existing_task(body)
            else:
                payload = self.dispatch_new_task(body)
            self.send_json(payload)
        except RequestError as exc:
            self.send_json(exc.payload, status=exc.status)
        except urllib.error.HTTPError as exc:
            detail = exc.read().decode("utf-8", errors="replace")
            self.send_json({"error": f"Provider HTTP error {exc.code}: {detail}"}, status=502)
        except Exception as exc:
            self.send_json({"error": str(exc)}, status=500)

    def send_json(self, payload, status=200):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, format, *args):
        sys.stderr.write("%s - %s\n" % (self.address_string(), format % args))


def main():
    server = ThreadingHTTPServer((HOST, PORT), AgentHandler)
    print(f"Local agent service listening on http://{HOST}:{PORT} provider={active_provider()}", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
