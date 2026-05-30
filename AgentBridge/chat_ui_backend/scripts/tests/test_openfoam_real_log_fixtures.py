import json
import unittest
from pathlib import Path
from typing import Any

from scripts.simagent_core.solvers.openfoam.run_diagnostics import parse_openfoam_log


FIXTURE_ROOT = Path(__file__).resolve().parent / "fixtures" / "openfoam_e2e"
ALLOWED_SOURCES = {"docker_e2e", "manual_excerpt"}
ALLOWED_STATUSES = {"passed", "warning", "failed"}


class OpenFOAMRealLogFixtureTests(unittest.TestCase):
    def test_openfoam_e2e_log_fixtures_match_expected_diagnostics(self) -> None:
        fixture_dirs = _fixture_dirs()
        self.assertGreater(len(fixture_dirs), 0, "No OpenFOAM E2E log fixtures were found.")

        for fixture_dir in fixture_dirs:
            with self.subTest(fixture=fixture_dir.name):
                meta = _read_meta(fixture_dir)
                diagnostics = _parse_fixture_logs(fixture_dir)
                codes = _diagnostic_codes(diagnostics)
                metrics = diagnostics["metrics"]

                self.assertIn(meta["source"], ALLOWED_SOURCES, f"{fixture_dir.name}: invalid source")
                self.assertIn(meta["expected_status"], ALLOWED_STATUSES, f"{fixture_dir.name}: invalid expected_status")
                self.assertEqual(
                    meta["expected_status"],
                    diagnostics["severity"],
                    f"{fixture_dir.name}: expected status {meta['expected_status']} but got {diagnostics['severity']}",
                )

                if meta["expected_status"] == "passed":
                    self.assertEqual([], _error_items(diagnostics), f"{fixture_dir.name}: success fixture emitted errors")
                else:
                    self.assertTrue(
                        meta["expected_codes"],
                        f"{fixture_dir.name}: non-passed fixtures must list at least one expected code",
                    )

                for code in meta["expected_codes"]:
                    self.assertIn(code, codes, f"{fixture_dir.name}: missing expected code {code}")
                for code in meta["expected_absent_codes"]:
                    self.assertNotIn(code, codes, f"{fixture_dir.name}: unexpected false-positive code {code}")
                for metric in meta["expected_metrics"]:
                    self.assertIn(metric, metrics, f"{fixture_dir.name}: missing expected metric {metric}")


def _fixture_dirs() -> list[Path]:
    return sorted(
        path
        for path in FIXTURE_ROOT.iterdir()
        if path.is_dir() and (path / "meta.json").is_file()
    )


def _read_meta(fixture_dir: Path) -> dict[str, Any]:
    meta = json.loads((fixture_dir / "meta.json").read_text(encoding="utf-8"))
    required = {
        "case",
        "backend",
        "solver",
        "scenario",
        "source",
        "expected_status",
        "expected_codes",
        "expected_absent_codes",
        "expected_metrics",
        "notes",
    }
    missing = sorted(required - set(meta))
    if missing:
        raise AssertionError(f"{fixture_dir.name}: meta.json missing fields: {', '.join(missing)}")
    for key in ["expected_codes", "expected_absent_codes", "expected_metrics"]:
        if not isinstance(meta[key], list):
            raise AssertionError(f"{fixture_dir.name}: {key} must be a list")
    return meta


def _parse_fixture_logs(fixture_dir: Path) -> dict[str, Any]:
    log_paths = sorted((fixture_dir / "logs").glob("*.log"))
    if not log_paths:
        raise AssertionError(f"{fixture_dir.name}: no logs/*.log files found")

    items: list[dict[str, Any]] = []
    metrics: dict[str, Any] = {}
    for log_path in log_paths:
        parsed = parse_openfoam_log(
            log_path.read_text(encoding="utf-8", errors="replace"),
            command=_command_from_log_name(log_path),
            log_file=_fixture_log_file(fixture_dir, log_path),
        )
        items.extend(parsed["items"])
        _merge_metrics(metrics, parsed["metrics"])

    return {
        "severity": _combined_severity(items),
        "items": items,
        "metrics": metrics,
    }


def _command_from_log_name(log_path: Path) -> str:
    return log_path.stem


def _fixture_log_file(fixture_dir: Path, log_path: Path) -> str:
    return f"fixtures/openfoam_e2e/{fixture_dir.name}/logs/{log_path.name}"


def _combined_severity(items: list[dict[str, Any]]) -> str:
    if any(item.get("severity") == "error" for item in items):
        return "failed"
    if any(item.get("severity") == "warning" for item in items):
        return "warning"
    return "passed"


def _merge_metrics(target: dict[str, Any], incoming: dict[str, Any]) -> None:
    for key, value in incoming.items():
        if key not in target:
            target[key] = value
            continue
        if isinstance(value, bool):
            target[key] = bool(target[key]) or value
        elif isinstance(value, (int, float)) and isinstance(target[key], (int, float)):
            target[key] = min(target[key], value) if key == "min_volume" else max(target[key], value)
        elif isinstance(value, list) and isinstance(target[key], list):
            target[key] = sorted({*target[key], *value})
        elif isinstance(value, dict) and isinstance(target[key], dict):
            target[key] = {**target[key], **value}
        else:
            target[key] = value


def _diagnostic_codes(diagnostics: dict[str, Any]) -> list[str]:
    return sorted({
        str(item.get("code", "")).strip()
        for item in diagnostics["items"]
        if isinstance(item, dict) and str(item.get("code", "")).strip()
    })


def _error_items(diagnostics: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        item
        for item in diagnostics["items"]
        if isinstance(item, dict) and item.get("severity") == "error"
    ]


if __name__ == "__main__":
    unittest.main()
