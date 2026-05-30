# OpenFOAM E2E Log Fixtures

This directory stores a small regression baseline for OpenFOAM run diagnostics. Fixtures are real or real-format OpenFOAM log excerpts paired with `meta.json` expectations. The fixture tests read these files only; they never start Docker or require local OpenFOAM.

## Purpose

- Preserve high-value OpenFOAM log patterns as parser regression samples.
- Keep diagnostics behavior stable across parser changes.
- Provide a controlled bridge between opt-in Docker E2E runs and default hermetic unittest runs.

## Docker Collection

Recommended environment for collecting real Docker logs:

```powershell
$env:SIMAGENT_OPENFOAM_BACKEND="docker"
$env:SIMAGENT_OPENFOAM_DOCKER_IMAGE="leoyue123/foamagent:latest"
$env:SIMAGENT_OPENFOAM_DOCKER_CASE_DIR="/case"
```

Run the relevant case through the normal Agent/runtime path, then copy only compact log files into a fixture directory:

```text
tests/fixtures/openfoam_e2e/<scenario>/
  meta.json
  logs/
    blockMesh.log
    icoFoam.log
```

Set `meta.json.source` to:

- `docker_e2e` when the logs were collected from an actual Docker/OpenFOAM run.
- `manual_excerpt` when the files are short, hand-seeded excerpts that preserve real OpenFOAM wording but are not complete run logs.

## Files Allowed In Fixtures

- `meta.json`
- `logs/*.log`
- Optional compact manifest excerpt if needed for future tests

## Files Not Allowed In Fixtures

- Large result directories
- VTK outputs
- Full `constant/polyMesh` directories or large mesh files
- Full `task_context.json` with local/private paths
- Docker image layers or exported containers
- Temporary task directories

## meta.json Contract

Each fixture must include:

```json
{
  "case": "rect_channel",
  "backend": "docker",
  "solver": "icoFoam",
  "scenario": "courant_high",
  "source": "docker_e2e|manual_excerpt",
  "expected_status": "passed|warning|failed",
  "expected_codes": ["result.courant_high"],
  "expected_absent_codes": ["result.residual_nan"],
  "expected_metrics": ["max_courant", "courant_level"],
  "notes": "Short explanation of how the log was produced."
}
```

Rules:

- Success fixtures use an empty `expected_codes` list.
- Failure fixtures must list at least one expected diagnostic code.
- Use `expected_absent_codes` to lock false-positive behavior.
- Use `expected_metrics` only for metrics the parser should definitely produce.
- Add or update `tests/test_openfoam_real_log_fixtures.py` coverage whenever adding a fixture shape that needs new assertions.

The first fixture batch is `manual_excerpt` by design, because it is intended to bootstrap a stable parser baseline without making default tests depend on Docker/OpenFOAM.
