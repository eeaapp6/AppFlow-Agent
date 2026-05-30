# OpenFOAM Diagnostics Codes

This document describes the current OpenFOAM run diagnostics contract used by `run_diagnostics.py`, `result_review_gate.py`, and `repair/proposals.py`.

It is a stability reference for UI, AppFlow, repair proposal logic, and external communication. It documents current behavior only; future parser ideas are not listed as implemented behavior.

## Diagnostics Bundle

OpenFOAM run diagnostics use this bundle shape:

```json
{
  "severity": "passed|warning|failed",
  "summary": "Human-readable primary diagnostic summary.",
  "items": [],
  "metrics": {}
}
```

Bundle severity is derived from item severity:

- `failed`: at least one item has `severity: "error"`.
- `warning`: no errors, at least one item has `severity: "warning"`.
- `passed`: no diagnostic items.

## Diagnostic Item

Each item should keep this shape:

| Field | Required | Meaning |
| --- | --- | --- |
| `code` | yes | Stable machine-readable diagnostic code, for example `result.courant_high`. |
| `severity` | yes | Item severity: `error`, `warning`, or `info` when introduced later. Current parser mainly emits `error` and `warning`. |
| `category` | yes | High-level class such as `runtime`, `boundary`, `configuration`, `numerics`, `mesh`, or `output`. |
| `message` | yes | Human-readable summary for users and review gates. |
| `source` | yes | Source command or subsystem, for example `blockMesh`, `solver`, `runtime`, `checkMesh`. |
| `log_file` | yes | Display path for the relevant log, usually under `logs/`. Empty for runtime-only diagnostics. |
| `matched_line` | yes | Short excerpt that triggered the diagnostic. Empty when no log line exists. |
| `line` | no | 1-based log line number when available. |
| `repair_hint` | no | Human-readable repair direction. |
| `repair_action_input` | no | Structured input for a safe executable repair proposal. Currently used for missing boundary fields. |

## Diagnostic Codes

| Code | Category | Default severity | Trigger / matched pattern | Key metrics | Repair behavior | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| `result.runtime_unavailable` | runtime | error | Runtime readiness event says backend is unavailable. | none | `configure_openfoam_runtime` manual action. | Used when no selected backend can execute OpenFOAM. |
| `result.command_nonzero` | runtime | error | A pipeline step has non-zero `return_code`. | none; item includes `return_code` | `review_solver_logs` manual action. | Log parser still runs first so domain diagnostics can accompany this code. |
| `result.command_timeout` | runtime | error | Command timeout event. | none; item includes `timeout_seconds` | `review_solver_logs` manual action. | Created by `timeout_diagnostic()`. |
| `result.log_warning` | runtime | warning | `--> FOAM Warning` or `Warning:` in a run log. | none | `review_solver_logs` manual action. | Warning-only diagnostics do not fail result review if usable results exist. |
| `result.log_fatal` | runtime | error | `FOAM FATAL ERROR`, `Segmentation fault`, or `core dumped`. | none | `inspect_fatal_log` manual action. | `Floating point exception` is mapped to `result.residual_nan` instead. |
| `result.run_incomplete` | runtime | error | Result review sees `run_result.status != "run_completed"` and no more specific diagnostics are available. | none | `configure_openfoam_runtime` manual action. | Result-review fallback code, not emitted by `run_diagnostics.py`. |
| `result.diagnostics_failed` | runtime | error | Diagnostics bundle has `severity: "failed"` but no itemized evidence. | none | Generic `inspect_gate_failure` unless another mapping is added. | Result-review fallback for malformed or incomplete diagnostics. |
| `result.diagnostics_warning` | runtime | warning | Diagnostics bundle has `severity: "warning"` but no itemized evidence. | none | Generic `inspect_gate_failure` for non-passed review. | Result-review fallback for warning bundles without items. |
| `result.missing_boundary_field` | boundary | error | `Cannot find patchField entry for patchName`. | `missing_boundary_fields` | `repair_case_dictionaries`; can generate executable `add_missing_boundary_field` patches when field/patch are known. | Item may include `field`, `patch`, and `repair_action_input`. |
| `result.unknown_patch` | boundary | error | `Unknown patch name`, `Unknown patch`, or `patch X not found/does not exist`. | none | `repair_case_dictionaries` manual action unless missing-field metrics provide patches. | Indicates dictionary and mesh patch names disagree. |
| `result.patch_type_inconsistent` | boundary | error | `inconsistent patch and patchField types`; optionally `patch type X and patchField type Y`. | `patch_type`, `patch_field_type` | `repair_case_dictionaries` manual action. | No automatic patch is generated because the correct type depends on mesh and physics intent. |
| `result.pressure_reference_missing` | configuration | error | `Unable to set reference cell for field p` or `Please supply either pRefCell or pRefPoint`. | `pressure_reference_missing_field` | `review_case_configuration` manual action. | No automatic patch yet; pressure reference choice must be case-aware. |
| `result.turbulence_properties_missing` | configuration | error | `cannot find file turbulenceProperties`. | none | `review_case_configuration` manual action. | Distinct from turbulence-model mismatch; only missing file is recognized. |
| `result.fvsolution_solver_block_missing` | configuration | error | `keyword H is undefined in dictionary ... fvSolution.solvers` or another missing field entry under `fvSolution`. | `missing_fvsolution_keyword` | `review_case_configuration` manual action. | Means a solver sub-dictionary is missing for a field. |
| `result.fvsolution_solver_keyword_missing` | configuration | error | `keyword solver is undefined in dictionary`. | `missing_fvsolution_keyword` | `review_case_configuration` manual action. | Means a solver sub-dictionary exists but lacks the `solver` keyword. |
| `result.courant_high` | numerics | error | `Courant Number ... max: value` with `value > 1`. | `max_courant`, `courant_level` | `stabilize_time_step`; executable patches enable `adjustTimeStep` and lower `deltaT`. | `courant_level` is `warning` for `>=10`, `error` for `>=100`, `runaway` for `>=1e4`. The code remains `result.courant_high`. |
| `result.continuity_abnormal` | numerics | warning or error | `time step continuity errors : sum local = ..., global = ..., cumulative = ...`. | `max_continuity_local`, `max_continuity_global`, `max_continuity_cumulative` | Solver numerics action. May become executable only if Courant/residual metrics also justify existing safe patches. | Warning when `abs(local)` or `abs(global) >= 1e-2`; error when `>= 1`. Smaller values only update metrics. |
| `result.residual_nan` | numerics | error | `Floating point exception`, or standalone `nan`/`inf` in solver log. | `has_nan_or_inf` | `inspect_solver_numerics`; executable patches enable `adjustTimeStep` and lower `deltaT`. | Used for NaN/Inf and floating point exception. |
| `result.divergence` | numerics | error | `divergence detected`, `solution diverged`, or `diverging`. | none | Solver numerics action. | May remain manual unless other metrics select a safe executable patch. |
| `result.residual_high` | numerics | error | Solver residual line where final residual is greater than `1.0`. | `residual_count`, `residual_fields`, `max_initial_residual`, `max_final_residual`, `worst_residual_field` | `inspect_solver_numerics`; can generate executable `update_solver_tolerance` patch for known worst field. | The current threshold is intentionally simple and conservative. |
| `result.no_execution_time` | runtime | warning | Result review scans residual output but finds no `ExecutionTime` marker. | residual metrics may be present | `review_solver_logs` manual action. | Emitted by `result_review_gate.py`, not by `run_diagnostics.py`. |
| `result.mesh_quality_failed` | mesh | error | `blockMesh` log contains `***Error`, `FOAM FATAL ERROR`, or `failed`, or result review sees mesh failure markers. | `failed_mesh_checks`, `max_non_orthogonality`, `max_skewness`, `max_aspect_ratio`, `min_volume` when parsed by result review | `review_mesh_quality` manual action. | `run_diagnostics.py` suppresses this generic code when a more specific blockMesh `result.mesh_*` error already exists. |
| `result.mesh_quality_warning` | mesh | warning | Result review sees mesh warning markers such as severely non-orthogonal, highly skew, max skewness, or max aspect ratio. | mesh quality metrics when parsed | `review_mesh_quality` manual action. | Emitted by `result_review_gate.py`, not by `run_diagnostics.py`. |
| `result.mesh_severe_non_orthogonality` | mesh | warning or error | `Number of severely non-orthogonal (> 70 degrees) faces: N`. | `severe_non_orthogonal_faces`, `failed_mesh_checks` | `review_mesh_quality` manual action. | Warning when `N > 0`; raised to error if the same log has `Failed N mesh checks` with `N > 0`. |
| `result.mesh_negative_volume` | mesh | error | `Zero or negative cell volume detected`, `Minimum negative volume`, or `Number of negative volume cells`. | `negative_volume_cells`, `min_volume` | `review_mesh_quality` manual action. | Invalid volume is never auto-patched. |
| `result.missing_latest_path` | output | error | Run completed but `outputs.results.latest_path` is missing. | none | `review_solver_logs` manual action. | Result-review output check. |
| `result.missing_latest_dir` | output | error | `outputs.results.latest_path` exists in metadata but does not resolve to an existing directory. | none | `review_solver_logs` manual action. | Result-review output check. |

## Metrics Fields

| Metric | Type | Producer | Meaning / merge behavior |
| --- | --- | --- | --- |
| `max_courant` | number | run diagnostics and item merge | Maximum parsed Courant number. Merged by maximum. |
| `courant_level` | string | run diagnostics | Qualitative level for high Courant: `warning`, `error`, `runaway`. Merged by worst level. |
| `has_nan_or_inf` | boolean | run diagnostics, result review | True when NaN/Inf or floating point exception is detected. Merged by OR. |
| `residual_count` | integer | run diagnostics, result review | Number of parsed residual lines. Merged by addition. |
| `residual_fields` | list[string] | run diagnostics, result review | Fields with parsed residuals. Merged as sorted unique list. |
| `max_initial_residual` | number | run diagnostics, result review | Maximum initial residual. Merged by maximum. |
| `max_final_residual` | number | run diagnostics, result review | Maximum final residual. Merged by maximum. |
| `worst_residual_field` | string | run diagnostics, result review | Field associated with the highest final residual. |
| `missing_boundary_fields` | object | run diagnostics, validation diagnostics | Map of field name to missing patch names, for example `{ "U": ["inlet"] }`. Merged by field and unique patch. |
| `pressure_reference_missing_field` | string | run diagnostics | Field whose pressure reference could not be set, usually `p`. |
| `patch_type` | string | run diagnostics | Mesh patch type from an inconsistent patch-type diagnostic. |
| `patch_field_type` | string | run diagnostics | Field patch type from an inconsistent patch-type diagnostic. |
| `max_continuity_local` | number | run diagnostics | Maximum absolute local continuity error. Merged by maximum. |
| `max_continuity_global` | number | run diagnostics | Maximum absolute global continuity error. Merged by maximum. |
| `max_continuity_cumulative` | number | run diagnostics | Maximum absolute cumulative continuity error. Merged by maximum. |
| `severe_non_orthogonal_faces` | integer | run diagnostics | Maximum parsed count of severely non-orthogonal faces. |
| `failed_mesh_checks` | number | run diagnostics, result review | Number of failed mesh checks. Merged by maximum. |
| `negative_volume_cells` | integer | run diagnostics | Maximum parsed count of negative-volume cells. |
| `min_volume` | number | run diagnostics, result review | Minimum parsed cell volume. Merged by minimum once present. |
| `missing_fvsolution_keyword` | string | run diagnostics | Missing fvSolution keyword or field solver block name. |
| `max_non_orthogonality` | number | result review mesh log scan | Maximum mesh non-orthogonality from review log scan. |
| `max_skewness` | number | result review mesh log scan | Maximum mesh skewness from review log scan. |
| `max_aspect_ratio` | number | result review mesh log scan | Maximum mesh aspect ratio from review log scan. |

## Repair Behavior

Repair proposals are generated by `repair/proposals.py` from gate review issue codes and diagnostics metrics. Repair actions are never applied automatically; they are applied only when the user submits `/foam/repair-action`.

### Can Generate Executable Patches

These codes can produce executable patches when the needed metrics are present:

- `result.missing_boundary_field`: can produce `add_missing_boundary_field` patches from `missing_boundary_fields`.
- `result.courant_high`: produces `enable_adjust_time_step` and `adjust_time_step`.
- `result.residual_nan`: produces timestep stabilization patches.
- `result.residual_high`: can produce `update_solver_tolerance` for known worst residual field.
- `result.divergence` and `result.continuity_abnormal`: use the solver numerics path; executable patches are selected only if supporting metrics such as high Courant, NaN/Inf, or worst residual field are present.

### Manual Review Only

These codes intentionally avoid automatic patching:

- Boundary/dictionary review: `result.unknown_patch`, `result.patch_type_inconsistent` unless missing-boundary metrics independently provide safe patch inputs.
- Configuration review: `result.pressure_reference_missing`, `result.turbulence_properties_missing`, `result.fvsolution_solver_block_missing`, `result.fvsolution_solver_keyword_missing`.
- Mesh review: `result.mesh_quality_failed`, `result.mesh_quality_warning`, `result.mesh_severe_non_orthogonality`, `result.mesh_negative_volume`.
- Log/output review: `result.log_fatal`, `result.log_warning`, `result.command_nonzero`, `result.command_timeout`, `result.no_execution_time`, `result.missing_latest_path`, `result.missing_latest_dir`.

### Runtime Configuration

These codes point users to runtime setup rather than case repair:

- `result.runtime_unavailable`
- `result.run_incomplete` when no more specific diagnostics are available

### Generic / No Specific Repair Mapping

These review fallback codes currently fall through to generic inspection unless a future mapping is added:

- `result.diagnostics_failed`
- `result.diagnostics_warning`

## Result Review Contract

`result_review_gate.py` consumes diagnostics and converts them to a stable run conclusion:

- A `run_failed` or `run_blocked` result cannot be reviewed as passed.
- Any diagnostic item with `severity: "error"` makes the review failed.
- Warning-only diagnostics can produce a warning review and still allow results to be shown when output directories exist.
- `result.missing_latest_path` and `result.missing_latest_dir` prevent result display even if commands completed.
- `result_review.can_show_results`, `should_offer_repair`, `should_offer_rerun`, `should_offer_vtk`, and `should_offer_paraview` should be consumed instead of duplicating review logic in UI.

## Stability Rules

- Diagnostic code names are stable identifiers. Once UI, AppFlow, tests, or repair proposal logic consume a code, do not rename it casually.
- New diagnostic codes must include focused tests for positive detection and false-positive resistance.
- Parser uncertainty should prefer `warning` or no diagnostic over false-positive `error`.
- Automatic patches must remain conservative. Default to manual review unless a patch is deterministic, path-safe, and already covered by tests.
- Diagnostics with `severity: "failed"` must not be converted to a passed result review.
- `result.courant_high` is the stable code for high and runaway Courant conditions; use `metrics.courant_level` for severity detail instead of inventing a second code.
