# AgentBridge

AgentBridge defines the artifact contract between an external Agent workflow and APPFlow.

The first integration stage keeps the Agent workflow end-to-end. The Agent runs the whole simulation, then writes `agent_manifest_v1.json`. APPFlow reads this manifest to locate the generated case, mesh, logs, and results.

## Stage 1 Goal

```text
Agent end-to-end workflow
  -> writes agent_manifest_v1.json
  -> APPFlow reads the manifest
  -> APPFlow imports mesh, shows logs, and opens results
```

At this stage, APPFlow does not control each Agent step. It only consumes the final artifacts after the Agent workflow finishes.

## Expected Agent Output

```text
agent_output/
  agent_manifest_v1.json
  case/
    system/
    constant/
    0/
  logs/
  results/
```

The exact folder names can change, but `agent_manifest_v1.json` must point to the real artifact paths.

## Examples

```text
examples/minimal_manifest_v1.json
  Minimal required fields only. Use this when validating the smallest Agent output contract.

examples/agent_manifest_v1.json
  OpenFOAM example with existing VTK results. APPFlow can import mesh and open ParaView directly.

examples/test.json
  OpenFOAM example without pre-generated VTK. APPFlow can run Docker foamToVTK before opening ParaView.

examples/phenglei_minimal_manifest_v1.json
  Minimal PHengLEI example. APPFlow currently supports OpenFOAM only, so this example verifies that APPFlow reports PHengLEI as unsupported instead of running OpenFOAM logic.
```

## Manifest Structure

The v1 manifest uses five top-level sections:

```json
{
  "version": 1,
  "workflow": {},
  "solver": {},
  "artifacts": {},
  "case_summary": {},
  "appflow_hints": {}
}
```

### Minimal Required Fields

These fields are the smallest contract APPFlow needs to load the manifest and locate the generated case. Other fields should be treated as optional enrichment.

| Field | Type | Meaning |
| --- | --- | --- |
| `version` | integer | Manifest schema version. Current version is `1`. |
| `workflow.id` | string | Unique Agent workflow id or case id. |
| `workflow.status` | string | Workflow result: `succeeded`, `failed`, or `partial`. |
| `solver.family` | string | Solver backend family, for example `openfoam` or future `phenglei`. |
| `artifacts.case_dir` | string | Generated solver case directory. |

### Optional Artifact Fields

| Field | Type | Meaning |
| --- | --- | --- |
| `artifacts.mesh_dir` | string | Mesh directory. For OpenFOAM this is usually `case/constant/polyMesh`. |
| `artifacts.result_dir` | string | Result directory, for example OpenFOAM `case/VTK`. |
| `artifacts.logs` | object array | Logs APPFlow can display. |

### Optional Display Fields

| Field | Type | Meaning |
| --- | --- | --- |
| `workflow.created_at` | string | ISO-like timestamp for display and sorting. |
| `solver.command` | string | Actual solver command, for example `simpleFoam`. |
| `solver.case_type` | string | Coarse case category, for example `steady_incompressible`. |
| `case_summary.case_name` | string | Human-readable case name. |
| `case_summary.mesh_format` | string | Mesh format, for example `openfoam_polyMesh` or future PHengLEI mesh type. |
| `case_summary.fields` | string array | Main fields, for example `U`, `p`, `T`. |
| `case_summary.boundaries` | object array | Boundary names and coarse boundary types. |

### Optional Behavior Hints

| Field | Type | Meaning |
| --- | --- | --- |
| `appflow_hints.import_mesh` | boolean | Whether APPFlow should try to import the mesh automatically. |
| `appflow_hints.show_logs` | boolean | Whether APPFlow should show logs after loading the manifest. |
| `appflow_hints.show_results` | boolean | Whether APPFlow should show results after loading the manifest. |
| `appflow_hints.preferred_result_format` | string | Preferred result format, for example `vtk`. |
| `appflow_hints.open_paraview` | boolean | Whether APPFlow should open the detected VTK result with ParaView. |
| `appflow_hints.export_vtk` | boolean | Whether APPFlow should run `foamToVTK -ascii -case <case_dir>` when OpenFOAM time directories exist but VTK output is missing. |
| `appflow_hints.foam_to_vtk_backend` | string | Optional VTK export backend. Use `native` for local `foamToVTK` or `docker` for Docker-based OpenFOAM. |
| `appflow_hints.docker_image` | string | Docker image used when `foam_to_vtk_backend` is `docker`, for example `foamagent`. |
| `appflow_hints.docker_case_dir` | string | Container mount path for the OpenFOAM case. Defaults to `/case`. |

## APPFlow Stage 1 Behavior

APPFlow should initially support these actions:

```text
load_agent_manifest
import_mesh_if_exists
show_logs
show_result_if_exists
export_vtk_if_requested
refresh_tree_and_view
```

For OpenFOAM, `artifacts.mesh_dir` can be connected later to the existing OpenFOAM mesh import path. For PHengLEI, the same manifest structure should be reused, while the solver-specific adapter interprets PHengLEI paths and files.

## Current Code Structure

The current APPFlow implementation is split into small layers so the core bridge logic can be reused later:

```text
OperatorsAgentManifest
  APPFlow binding layer: menu action, file dialog, FITK messages, and APPFlow mesh import operator.

AgentManifestData
  Reusable manifest layer: reads JSON, validates v1 required fields, and exposes workflow / solver / artifacts / hints.

AgentLogPreview
  Reusable log layer: checks log file existence and appends a short preview to message output.

OpenFoamAgentAdapter
  OpenFOAM adapter layer: checks result files, detects time directories, runs native or Docker foamToVTK, and opens ParaView.
```

For a future `appPHengLEI` migration, prefer reusing `AgentManifestData` and `AgentLogPreview` first. Keep each application-specific UI entry in its own binding layer, and add a dedicated PHengLEI adapter instead of mixing PHengLEI logic into `OperatorsAgentManifest`.

## Later Extensions

After the end-to-end integration works, this contract can be extended with step-level events:

```text
plan_ready
mesh_ready
case_config_ready
run_started
run_log
run_finished
result_ready
```

The same top-level structure should continue to support both OpenFOAM and PHengLEI through solver-specific adapters.
