# OpenFOAM Agent Demo and Acceptance Checklist

This checklist is for customer-facing demo and first-stage acceptance of the current AgentBridge OpenFOAM Agent. It is intentionally scoped to the implemented OpenFOAM path and does not claim support for arbitrary CFD automation.

## 1. Report Goal

Use this demo to prove the current engineering loop:

- Natural language can be turned into a controlled OpenFOAM case workflow.
- `physics_sanity` provides a first-pass physical risk check for generated `rect_channel`.
- Docker or local runtime can execute the OpenFOAM pipeline.
- Run diagnostics can identify common runtime/log problems.
- Repair action keeps user confirmation in the loop.
- `repair_followup` can mark whether a repair is resolved or unresolved after a later rerun.

## 2. Demo Environment

Recommended demo backend is Docker, especially on machines without local OpenFOAM.

PowerShell environment:

```powershell
$env:SIMAGENT_OPENFOAM_BACKEND="docker"
$env:SIMAGENT_OPENFOAM_DOCKER_IMAGE="leoyue123/foamagent:latest"
$env:SIMAGENT_OPENFOAM_DOCKER_CASE_DIR="/case"
```

Notes:

- Local backend remains the default long-term primary path when `SIMAGENT_OPENFOAM_BACKEND` is not set.
- Docker backend is opt-in and executes commands through `docker run --rm -v <host_case_dir>:/case -w /case <image> <command>`.
- WSL execution is not implemented; it should be described as detected/blocked only.
- Docker E2E test is opt-in and skipped by default.
- If Docker/OpenFOAM is not available, use mocked scenario tests to demonstrate the logic loop without running a real solver.

## 3. Demo Scenario A: rect_channel Happy Path

Prompt:

```text
Generate a laminar rectangular channel length=3 height=0.5 velocity=1 nu=0.01 endTime=1 deltaT=0.005
```

Flow:

1. Call `/foam/plan`.
2. Confirm `plan.parameters.simulation_spec.geometry.type == "rect_channel"`.
3. Call `/foam/generate`.
4. Confirm generated files include `case/system/blockMeshDict`, `case/0/U`, `case/0/p`.
5. Call `/foam/validate`.
6. Confirm static validation is `validated`.
7. Confirm `physics_sanity.status == "passed"`.
8. Call `/foam/run`.
9. Confirm `run.status == "run_completed"` and `runtime_info.backend` is `docker` or `local`.
10. Confirm `result_review.status == "passed"`.
11. Confirm manifest hints: `show_results=true`, `run_foam_to_vtk=true` when raw OpenFOAM result time directories exist and VTK has not already been generated.

Value to show:

- Controlled natural-language-to-case path.
- Deterministic generated OpenFOAM dictionaries.
- Validation and physics sanity before run.
- Runtime execution and result-review-driven AppFlow hints.

## 4. Demo Scenario B: Large deltaT Physics Warning

Prompt:

```text
Generate a rectangular channel length=5 height=1 velocity=1 nu=0.01 deltaT=0.1
```

Expected behavior:

- `physics_sanity.status == "warning"`.
- `physics_sanity.metrics.estimated_courant > 1`.
- Gate issue includes `physics.courant_risk`.
- Warning does not block `/foam/run`.
- Manifest includes:
  - `appflow_hints.physics_sanity_status="warning"`
  - `appflow_hints.physics_sanity_summary`

Message:

This is a risk hint, not a hard failure. It demonstrates that the agent can flag questionable settings without over-blocking a runnable case.

## 5. Demo Scenario C: Invalid Physics Blocks Run

Construct by editing or creating a plan/spec with one obvious invalid setting:

- `nu <= 0`
- `deltaT <= 0`
- `length <= 0`, `height <= 0`, or `depth <= 0`
- invalid `nx` or `ny`

Expected behavior:

- `physics_sanity.status == "failed"`.
- Issues include one or more:
  - `physics.invalid_geometry`
  - `physics.invalid_viscosity`
  - `physics.invalid_time_step`
  - `physics.invalid_mesh_cells`
  - `physics.invalid_velocity`
- `/foam/run` returns `run_blocked`.
- OpenFOAM runtime is not entered.
- Repair proposal is manual `revise_simulation_spec`.

Message:

This demonstrates that obviously invalid parameters are stopped before launching OpenFOAM.

## 6. Demo Scenario D: solver log nan/divergence

Use the existing mocked scenario path or a prepared solver log containing:

```text
solution diverged
ExecutionTime = nan s
```

Expected behavior:

- Command can return `0`, but diagnostics still emit:
  - `result.residual_nan`
  - `result.divergence`
- `run.status == "run_failed"` when diagnostics severity is failed.
- `result_review.status == "failed"`.
- Manifest/result review exposes:
  - `should_offer_repair=true`
  - `should_offer_rerun=true`
  - `appflow_hints.offer_repair=true`
  - `appflow_hints.offer_rerun=true`
- No automatic patch.
- No automatic rerun.

Message:

This demonstrates false-success prevention: a solver process exit code is not trusted if logs show numerical failure.

## 7. Demo Scenario E: missing boundary field -> repair -> rerun resolved

Setup:

- Construct a case or diagnostics where `0/U` is missing a patch entry, for example missing `inlet`.

Expected flow:

1. Validation or run diagnostics detects missing boundary field.
2. Diagnostic code is `result.missing_boundary_field` or validation equivalent.
3. Repair proposal is `repair_case_dictionaries`.
4. User explicitly calls `/foam/repair-action` with the repair action.
5. Patch writes the missing boundary field under current `task_dir/case`.
6. `repair_history` records:
   - `repair_action_id`
   - `repair_mode`
   - `related_diagnostic_codes`
   - `patch_result`
   - `status`
7. User triggers `/foam/run` again.
8. If the related diagnostic code disappears, manifest/task state shows:
   - `repair_followup.status="resolved"`
   - `resolved_codes=["result.missing_boundary_field"]`

Message:

This demonstrates a complete diagnostic, confirmed repair, rerun tracking loop.

## 8. Scenario Matrix

| Scenario | Entry | Key API / Test | Expected gate | Expected manifest hints | Customer value | Risk / note |
| --- | --- | --- | --- | --- | --- | --- |
| A. rect_channel happy path | Natural-language prompt | `/foam/plan`, `/foam/generate`, `/foam/validate`, `/foam/run`; `tests.test_openfoam_agent_scenarios` | `static_validation=passed`, `physics_sanity=passed`, `result_review=passed` | `show_results=true`, `run_foam_to_vtk=true`, runtime backend present | Shows the full stable workflow | Prefer this as the first live demo. |
| B. Large deltaT warning | Prompt with `deltaT=0.1` | `tests.test_physics_sanity_gate` | `physics_sanity=warning`, issue `physics.courant_risk` | `physics_sanity_status=warning`, summary present | Shows physics risk review without over-blocking | Warning does not prove the run will fail. |
| C. Invalid physics blocks run | Plan/spec with invalid `nu`, `deltaT`, geometry, or cells | `tests.test_physics_sanity_gate` | `physics_sanity=failed`; run is `run_blocked` | `offer_repair=true` through manual repair proposal; physics summary present | Shows obvious bad inputs are blocked early | This is not automatic correction. |
| D. nan/divergence log | Mocked runner or prepared solver log | `tests.test_openfoam_agent_scenarios`, `tests.test_openfoam_run_diagnostics`, `tests.test_result_review_gate` | `result_review=failed` | `offer_repair=true`, `offer_rerun=true`, diagnostics severity failed | Prevents false success | Use prepared/mock log for reliable demo. |
| E. missing boundary repair resolved | Construct missing patch field | `tests.test_openfoam_agent_scenarios`, `/foam/repair-action` | validation or result review failed, then follow-up resolved | `last_repair_action`, `repair_followup.status=resolved`, `offer_rerun=true` after repair | Shows confirmed repair loop | User confirmation is required before patch. |

## 9. First-Stage Acceptance Criteria

The current milestone can be accepted if these are demonstrated:

- Natural-language prompt can generate a `rect_channel` OpenFOAM plan.
- Generated `rect_channel` files are created deterministically.
- Static validation passes for the generated happy path.
- `physics_sanity` passes, warns, or fails according to first-version rules.
- Failed `physics_sanity` blocks run before OpenFOAM runtime is entered.
- Docker or local backend can be selected and runtime information is reported.
- Run diagnostics can classify key run/log failures.
- Result review prevents failed diagnostics from being treated as success.
- Repair proposal is generated for supported diagnostic/gate issues.
- Executable repair patch is applied only after user confirmation.
- `repair_history` records what was applied or recorded.
- Later rerun updates `repair_followup` as `resolved`, `unresolved`, or `unknown`.
- Manifest exposes stable `workflow.gates`, `workflow.run`, diagnostics, result review, repair, and AppFlow hints.
- Full unittest discovery passes; Docker E2E remains opt-in.

## 10. Recommended Commands

Focused tests:

```powershell
$env:PYTHONDONTWRITEBYTECODE="1"
python -m unittest tests.test_physics_sanity_gate
python -m unittest tests.test_openfoam_agent_scenarios
python -m unittest tests.test_openfoam_run_diagnostics
```

Full tests from repo root:

```powershell
$env:PYTHONDONTWRITEBYTECODE="1"
python -m unittest discover -s AgentBridge/chat_ui_backend/scripts/tests
```

Full tests from `AgentBridge/chat_ui_backend/scripts`:

```powershell
$env:PYTHONDONTWRITEBYTECODE="1"
python -m unittest discover -s tests
```

Docker E2E opt-in:

```powershell
$env:PYTHONDONTWRITEBYTECODE="1"
$env:SIMAGENT_OPENFOAM_E2E="1"
$env:SIMAGENT_OPENFOAM_BACKEND="docker"
$env:SIMAGENT_OPENFOAM_DOCKER_IMAGE="leoyue123/foamagent:latest"
$env:SIMAGENT_OPENFOAM_DOCKER_CASE_DIR="/case"
python -m unittest tests.test_openfoam_docker_e2e
```

Cache check:

```powershell
rg --files -g "*.pyc" -g "__pycache__" AgentBridge/chat_ui_backend
```

Expected output: none.

## 11. Demo Risk Control

Do not over-claim during demo:

- Do not attempt arbitrary complex OpenFOAM cases live.
- Do not promise complete CFD physical correctness.
- Do not claim WSL, MPI, GPU, or parallel execution support.
- Do not claim automatic repair; repair requires explicit user confirmation.
- Do not claim automatic rerun; rerun is user-triggered.
- Do not claim all OpenFOAM logs are parsed; diagnostics are incrementally expanded from real logs.
- Do not claim `physics_sanity` covers all cases; first version only supports generated `rect_channel`.
- Do not present Docker as the only architecture; local remains the default long-term backend.

Use stable prepared inputs for live demo:

- Keep Scenario A as the live happy path.
- Use Scenario B/C to show gate behavior without running expensive solver work.
- Use Scenario D/E through mocked scenario tests or prepared logs when runtime availability is uncertain.

## 12. Recommended Live Demo Order

1. Scenario A: normal `rect_channel` happy path.
2. Scenario B: large `deltaT` produces `physics_sanity` warning but does not block.
3. Scenario C: invalid physical parameter blocks run before runtime.
4. Scenario D: solver log `nan/divergence` prevents false success.
5. Scenario E: missing boundary field repair, confirmed patch, rerun follow-up resolved.

This order starts with the most stable visible success, then shows increasingly advanced safety and repair behavior.
