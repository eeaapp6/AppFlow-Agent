from pathlib import Path
from pathlib import PureWindowsPath

from ..models import SimulationPlan, TaskContext
from ..utils import atomic_write_json, now_iso


class ManifestWriter:
    def write_pending(self, task: TaskContext) -> dict:
        manifest = self._base_manifest(task, workflow_status="partial")
        return self._write(task, manifest)

    def write_planned(self, task: TaskContext, plan: SimulationPlan) -> dict:
        manifest = self._base_manifest(task, workflow_status="partial", plan=plan)
        return self._write(task, manifest)

    def write_failed(self, task: TaskContext, message: str) -> dict:
        manifest = self._base_manifest(task, workflow_status="failed")
        manifest["workflow"]["error"] = message
        return self._write(task, manifest)

    def write_generated(self, task: TaskContext, plan: SimulationPlan, generated_files: list[dict]) -> dict:
        manifest = self._base_manifest(task, workflow_status="partial", plan=plan)
        return self._write(task, manifest)

    def write_validation_result(self, task: TaskContext, plan: SimulationPlan, validation_result: dict) -> dict:
        status = validation_result.get("status", "validation_failed")
        workflow_status = "partial" if status == "validated" else "failed"
        manifest = self._base_manifest(task, workflow_status=workflow_status, plan=plan)
        manifest["workflow"]["validation"] = validation_result
        return self._write(task, manifest)

    def write_run_result(self, task: TaskContext, plan: SimulationPlan, run_result: dict) -> dict:
        status = run_result.get("status", "run_blocked")
        result_review = self._latest_result_review(task)
        workflow_status = self._run_workflow_status(status, result_review)
        manifest = self._base_manifest(task, workflow_status=workflow_status, plan=plan)
        outputs = run_result.get("outputs", {}) if isinstance(run_result.get("outputs", {}), dict) else {}
        results = outputs.get("results", {}) if isinstance(outputs.get("results", {}), dict) else {}
        latest_path = str(results.get("latest_path", "")).strip()
        vtk_path = "case/VTK"
        has_vtk = self._has_vtk_result(task)
        has_raw_results = bool(latest_path)
        can_show_results = self._review_bool(result_review, "can_show_results", has_vtk or has_raw_results)
        should_offer_vtk = self._review_bool(result_review, "should_offer_vtk", has_raw_results and not has_vtk)
        should_offer_paraview = self._review_bool(result_review, "should_offer_paraview", has_vtk or has_raw_results)

        manifest["workflow"]["run"] = {
            "status": status,
            "reason": run_result.get("reason", ""),
        }
        if result_review:
            manifest["workflow"]["run"]["result_review"] = result_review
        diagnostics = run_result.get("diagnostics", {})
        diagnostics = diagnostics if isinstance(diagnostics, dict) else {}
        if diagnostics:
            manifest["workflow"]["run"]["diagnostics"] = self._run_diagnostics_for_manifest(diagnostics)
        runtime_info = run_result.get("runtime_info", {})
        runtime_info = runtime_info if isinstance(runtime_info, dict) else {}
        if isinstance(runtime_info, dict) and runtime_info:
            manifest["workflow"]["run"]["runtime_info"] = self._runtime_info_for_manifest(runtime_info)
        manifest["artifacts"]["result_dir"] = self._host_path(task, vtk_path) if can_show_results else ""
        manifest["artifacts"]["logs"] = [
            {
                "type": item.get("role", ""),
                "name": item.get("role", ""),
                "path": self._host_path(task, item.get("path", "")),
            }
            for item in run_result.get("logs", [])
            if isinstance(item, dict)
        ]
        manifest["appflow_hints"]["show_results"] = can_show_results
        manifest["appflow_hints"]["preferred_result_format"] = "vtk"
        manifest["appflow_hints"]["open_paraview"] = should_offer_paraview
        manifest["appflow_hints"]["paraview_open_mode"] = "first_vtk"
        manifest["appflow_hints"]["run_foam_to_vtk"] = should_offer_vtk
        manifest["appflow_hints"]["export_vtk"] = False
        manifest["appflow_hints"]["foam_to_vtk_backend"] = "docker"
        manifest["appflow_hints"]["docker_image"] = str(runtime_info.get("docker_image", "")).strip() or "leoyue123/foamagent:latest"
        manifest["appflow_hints"]["docker_case_dir"] = str(runtime_info.get("docker_case_dir", "")).strip() or "/case"
        if runtime_info:
            manifest["appflow_hints"]["runtime_backend"] = str(runtime_info.get("backend", "")).strip()
            manifest["appflow_hints"]["runtime_available"] = bool(runtime_info.get("available", False))
        if diagnostics:
            manifest["appflow_hints"]["run_diagnostics_summary"] = str(diagnostics.get("summary", "")).strip()
            manifest["appflow_hints"]["run_diagnostics_severity"] = str(diagnostics.get("severity", "")).strip()
            manifest["appflow_hints"]["has_run_diagnostics"] = bool(diagnostics.get("items"))
        manifest["appflow_hints"]["offer_repair"] = self._review_bool(result_review, "should_offer_repair", False)
        manifest["appflow_hints"]["offer_rerun"] = self._review_bool(result_review, "should_offer_rerun", status != "run_completed")
        manifest["appflow_hints"]["has_suggested_repair_actions"] = self._has_suggested_repair_action(task)
        return self._write(task, manifest)

    def _base_manifest(
        self,
        task: TaskContext,
        workflow_status: str,
        plan: SimulationPlan | None = None,
    ) -> dict:
        solver_name = plan.solver_name if plan else ""
        case_name = plan.case_name if plan else task.task_id
        case_type = plan.case_category if plan else ""
        fields = self._fields_from_plan(plan)
        manifest = {
            "version": 1,
            "workflow": {
                "id": task.task_id,
                "status": workflow_status,
                "created_at": task.created_at,
                "updated_at": now_iso(),
                "gates": self._gate_summary(task),
                "repair_history": [item for item in task.repair_history if isinstance(item, dict)],
                "last_repair_action": self._last_repair_action_summary(task),
                "repair_followup": self._repair_followup_summary(task),
            },
            "solver": {
                "family": task.solver_family,
                "command": solver_name,
                "case_type": case_type,
            },
            "artifacts": {
                "case_dir": self._host_path(task, "case"),
                "mesh_dir": self._host_path(task, "case/constant/polyMesh"),
                "result_dir": "",
                "logs": [],
            },
            "case_summary": {
                "case_name": case_name,
                "mesh_format": self._mesh_format(task, plan),
                "fields": fields,
                "boundaries": self._default_boundaries(task),
                "description": plan.description if plan else task.user_requirement,
            },
            "solver_settings": self._solver_settings(plan),
            "appflow_hints": {
                "import_mesh": True,
                "show_logs": True,
                "show_results": False,
                "preferred_result_format": "vtk",
                "open_paraview": False,
                "paraview_open_mode": "first_vtk",
                "run_foam_to_vtk": False,
                "export_vtk": False,
                "foam_to_vtk_backend": "docker",
                "docker_image": "leoyue123/foamagent:latest",
                "docker_case_dir": "/case",
                "offer_repair": False,
                "offer_rerun": self._last_repair_should_rerun(task),
            },
        }
        physics_sanity = self._latest_physics_sanity(task)
        if physics_sanity:
            manifest["appflow_hints"]["physics_sanity_status"] = str(physics_sanity.get("status", "")).strip()
            manifest["appflow_hints"]["physics_sanity_summary"] = str(physics_sanity.get("summary", "")).strip()
        return manifest

    def _mesh_format(self, task: TaskContext, plan: SimulationPlan | None) -> str:
        if task.solver_family != "openfoam":
            return ""
        mesh = plan.parameters.get("mesh", {}) if plan and isinstance(plan.parameters, dict) else {}
        if isinstance(mesh, dict) and mesh.get("mode") == "gmsh_external":
            return "gmsh_msh"
        return "openfoam_polyMesh"

    def _host_task_root(self, task: TaskContext) -> str:
        if task.host_output_root:
            return self._join_host_path(task.host_output_root, task.task_id)
        return task.task_dir

    def _host_path(self, task: TaskContext, relative_path: str) -> str:
        root = self._host_task_root(task)
        cleaned = str(relative_path).strip().replace("\\", "/").strip("/")
        if not cleaned:
            return root
        return self._join_host_path(root, *cleaned.split("/"))

    def _join_host_path(self, root: str, *parts: str) -> str:
        if ":" in root[:3] or "\\" in root:
            return str(PureWindowsPath(root, *parts)).replace("\\", "/")
        return str(Path(root, *parts)).replace("\\", "/")

    def _fields_from_plan(self, plan: SimulationPlan | None) -> list[str]:
        if not plan:
            return []
        fields = []
        for item in plan.planned_files:
            name = Path(item.path).name
            if item.path.startswith("case/0/") and name:
                fields.append(name)
        return sorted(set(fields))

    def _default_boundaries(self, task: TaskContext) -> list[dict]:
        if task.solver_family != "openfoam":
            return []
        return [
            {"name": "movingWall", "type": "wall"},
            {"name": "fixedWalls", "type": "wall"},
            {"name": "frontAndBack", "type": "empty"},
        ]

    def _solver_settings(self, plan: SimulationPlan | None) -> dict:
        if not plan:
            return {}

        changes = plan.requested_changes if isinstance(plan.requested_changes, dict) else {}
        time_settings = self._pick_existing(
            changes,
            {
                "start_time": "start_time",
                "end_time": "end_time",
                "delta_t": "delta_t",
                "write_interval": "write_interval",
            },
        )
        physics_settings = self._pick_existing(
            changes,
            {
                "kinematic_viscosity": "nu",
                "nu": "nu",
            },
        )
        numerics_settings = {"solver": plan.solver_name} if plan.solver_name else {}

        settings = {}
        if time_settings:
            settings["time"] = time_settings
        if physics_settings:
            settings["physics"] = physics_settings
        if numerics_settings:
            settings["numerics"] = numerics_settings
        return settings

    def _pick_existing(self, data: dict, field_map: dict[str, str]) -> dict:
        picked = {}
        for source_key, target_key in field_map.items():
            if source_key in data and data[source_key] not in ["", None]:
                picked[target_key] = data[source_key]
        return picked

    def _runtime_info_for_manifest(self, runtime_info: dict) -> dict:
        commands = runtime_info.get("commands", {})
        command_summary = {}
        if isinstance(commands, dict):
            for name, info in commands.items():
                if isinstance(info, dict):
                    command_summary[str(name)] = {"available": bool(info.get("available", False))}

        return {
            "mode": str(runtime_info.get("mode", "")).strip(),
            "backend": str(runtime_info.get("backend", "")).strip(),
            "available": bool(runtime_info.get("available", False)),
            "reason": str(runtime_info.get("reason", "")).strip(),
            "missing_commands": [
                str(item) for item in runtime_info.get("missing_commands", [])
                if str(item).strip()
            ] if isinstance(runtime_info.get("missing_commands", []), list) else [],
            "wm_project_dir": str(runtime_info.get("wm_project_dir", "")).strip(),
            "commands": command_summary,
            "docker_image": str(runtime_info.get("docker_image", "")).strip(),
            "docker_case_dir": str(runtime_info.get("docker_case_dir", "")).strip(),
        }

    def _run_diagnostics_for_manifest(self, diagnostics: dict) -> dict:
        items = diagnostics.get("items", [])
        normalized_items = []
        if isinstance(items, list):
            for item in items[:20]:
                if not isinstance(item, dict):
                    continue
                normalized_items.append({
                    "code": str(item.get("code", "")).strip(),
                    "severity": str(item.get("severity", "")).strip(),
                    "category": str(item.get("category", "")).strip(),
                    "message": str(item.get("message", "")).strip(),
                    "source": str(item.get("source", "")).strip(),
                    "log_file": str(item.get("log_file", "")).strip(),
                    "matched_line": str(item.get("matched_line", "")).strip(),
                    "repair_hint": str(item.get("repair_hint", "")).strip(),
                    "repair_action_input": item.get("repair_action_input", {})
                    if isinstance(item.get("repair_action_input", {}), dict)
                    else {},
                })
        return {
            "severity": str(diagnostics.get("severity", "")).strip(),
            "summary": str(diagnostics.get("summary", "")).strip(),
            "items": normalized_items,
            "metrics": diagnostics.get("metrics", {}) if isinstance(diagnostics.get("metrics", {}), dict) else {},
        }

    def _latest_result_review(self, task: TaskContext) -> dict:
        reviews = task.gate_reviews if isinstance(task.gate_reviews, list) else []
        for review in reversed(reviews):
            if not isinstance(review, dict):
                continue
            if str(review.get("gate", "")).strip() != "result_review":
                continue
            result_review = review.get("result_review", {})
            return result_review if isinstance(result_review, dict) else {}
        return {}

    def _run_workflow_status(self, run_status: str, result_review: dict) -> str:
        review_status = str(result_review.get("status", "")).strip() if isinstance(result_review, dict) else ""
        if review_status == "failed":
            return "failed"
        return "succeeded" if run_status == "run_completed" else "failed"

    def _review_bool(self, result_review: dict, key: str, fallback: bool) -> bool:
        if isinstance(result_review, dict) and key in result_review:
            return bool(result_review.get(key, False))
        return bool(fallback)

    def _gate_summary(self, task: TaskContext) -> dict:
        reviews = [item for item in task.gate_reviews if isinstance(item, dict)]
        summaries = [self._gate_review_summary(item) for item in reviews]
        counts = {
            "passed": 0,
            "warning": 0,
            "failed": 0,
            "unsupported": 0,
        }
        for item in summaries:
            status = item["status"]
            if status in counts:
                counts[status] += 1

        if not summaries:
            overall_status = "pending"
        elif counts["failed"] or counts["unsupported"]:
            overall_status = "failed"
        elif counts["warning"]:
            overall_status = "warning"
        else:
            overall_status = "passed"

        return {
            "status": overall_status,
            "total": len(summaries),
            "passed": counts["passed"],
            "warning": counts["warning"],
            "failed": counts["failed"],
            "unsupported": counts["unsupported"],
            "reviews": summaries,
        }

    def _gate_review_summary(self, review: dict) -> dict:
        issues = review.get("issues", [])
        normalized_issues = [item for item in issues if isinstance(item, dict)] if isinstance(issues, list) else []
        warning_count = sum(1 for item in normalized_issues if item.get("severity") == "warning")
        error_count = len(normalized_issues) - warning_count

        summary = {
            "gate": str(review.get("gate", "")).strip(),
            "status": str(review.get("status", "")).strip() or "unknown",
            "issue_count": len(normalized_issues),
            "error_count": error_count,
            "warning_count": warning_count,
            "issues": normalized_issues,
        }
        if isinstance(review.get("decision"), dict):
            summary["decision"] = review["decision"]
        if isinstance(review.get("diagnostics"), dict):
            summary["diagnostics"] = review["diagnostics"]
        if isinstance(review.get("result_review"), dict):
            summary["result_review"] = review["result_review"]
        if isinstance(review.get("physics_sanity"), dict):
            summary["physics_sanity"] = review["physics_sanity"]
        if isinstance(review.get("next_repair_action"), dict):
            summary["next_repair_action"] = review["next_repair_action"]
        return summary

    def _latest_physics_sanity(self, task: TaskContext) -> dict:
        reviews = task.gate_reviews if isinstance(task.gate_reviews, list) else []
        for review in reversed(reviews):
            if not isinstance(review, dict):
                continue
            if str(review.get("gate", "")).strip() != "physics_sanity":
                continue
            physics = review.get("physics_sanity", {})
            return physics if isinstance(physics, dict) else {}
        return {}

    def _has_suggested_repair_action(self, task: TaskContext) -> bool:
        reviews = task.gate_reviews if isinstance(task.gate_reviews, list) else []
        return any(isinstance(item, dict) and isinstance(item.get("next_repair_action"), dict) for item in reviews)

    def _last_repair_action_summary(self, task: TaskContext) -> dict:
        record = self._last_repair_record(task)
        if not record:
            return {}
        patch_result = record.get("patch_result", {})
        patch_result = patch_result if isinstance(patch_result, dict) else {}
        return {
            "repair_action_id": str(record.get("repair_action_id", record.get("action_id", ""))).strip(),
            "action_id": str(record.get("action_id", "")).strip(),
            "label": str(record.get("label", "")).strip(),
            "status": str(record.get("status", "")).strip(),
            "repair_mode": str(record.get("repair_mode", "")).strip(),
            "source_gate": str(record.get("source_gate", "")).strip(),
            "related_diagnostic_codes": self._string_list(record.get("related_diagnostic_codes", [])),
            "related_diagnostic_categories": self._string_list(record.get("related_diagnostic_categories", [])),
            "patch_result": self._patch_result_summary(patch_result),
            "should_rerun": str(record.get("status", "")).strip() in {"applied", "recorded"},
            "created_at": str(record.get("created_at", "")).strip(),
        }

    def _repair_followup_summary(self, task: TaskContext) -> dict:
        record = self._last_repair_record(task)
        if not record:
            return {"status": "unknown", "resolved_codes": [], "unresolved_codes": []}
        follow_up = record.get("repair_followup", {})
        if not isinstance(follow_up, dict):
            return {"status": "unknown", "resolved_codes": [], "unresolved_codes": []}
        return {
            "status": str(follow_up.get("status", "unknown")).strip() or "unknown",
            "resolved_codes": self._string_list(follow_up.get("resolved_codes", [])),
            "unresolved_codes": self._string_list(follow_up.get("unresolved_codes", [])),
            "checked_at": str(follow_up.get("checked_at", "")).strip(),
        }

    def _last_repair_should_rerun(self, task: TaskContext) -> bool:
        record = self._last_repair_record(task)
        return bool(record and str(record.get("status", "")).strip() in {"applied", "recorded"})

    def _last_repair_record(self, task: TaskContext) -> dict:
        history = task.repair_history if isinstance(task.repair_history, list) else []
        for record in reversed(history):
            if isinstance(record, dict):
                return record
        return {}

    def _patch_result_summary(self, patch_result: dict) -> dict:
        if not patch_result:
            return {}
        applied = patch_result.get("applied", [])
        skipped = patch_result.get("skipped", [])
        return {
            "status": str(patch_result.get("status", "")).strip(),
            "applied_count": len(applied) if isinstance(applied, list) else 0,
            "skipped_count": len(skipped) if isinstance(skipped, list) else 0,
        }

    def _string_list(self, value) -> list[str]:
        return [str(item).strip() for item in value if str(item).strip()] if isinstance(value, list) else []

    def _has_vtk_result(self, task: TaskContext) -> bool:
        vtk_dir = Path(task.case_dir) / "VTK"
        return vtk_dir.exists() and any(path.suffix.lower() == ".vtk" for path in vtk_dir.iterdir() if path.is_file())

    def _write(self, task: TaskContext, manifest: dict) -> dict:
        manifest["workflow"]["updated_at"] = now_iso()
        atomic_write_json(Path(task.manifest_path), manifest)
        return manifest
