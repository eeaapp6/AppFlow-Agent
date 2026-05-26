import unittest
from types import SimpleNamespace

from scripts.simagent_core.agent.replies import run_reply, validate_reply


def task_with_gate_reviews(gate_reviews: list[dict]) -> SimpleNamespace:
    return SimpleNamespace(
        gate_reviews=gate_reviews,
        manifest_path="C:/tmp/task/agent_manifest_v1.json",
    )


class AgentRepliesTests(unittest.TestCase):
    def test_validate_reply_reports_validation_errors_without_repair_hint(self) -> None:
        task = task_with_gate_reviews(
            [
                {
                    "gate": "static_validation",
                    "status": "failed",
                    "next_repair_action": {
                        "label": "Regenerate case",
                        "description": "Generate the case again.",
                    },
                }
            ]
        )

        reply = validate_reply(task, {"status": "validation_failed", "missing_files": ["case/0/p"]})

        self.assertIn("Case validation failed.", reply)
        self.assertIn("case/0/p", reply)
        self.assertNotIn("Suggested action:", reply)

    def test_run_reply_reports_failure_without_repair_hint(self) -> None:
        task = task_with_gate_reviews(
            [
                {
                    "gate": "execution",
                    "status": "failed",
                    "next_repair_action": {
                        "label": "Configure OpenFOAM runtime",
                        "description": "Source OpenFOAM before running again.",
                    },
                }
            ]
        )

        reply = run_reply(task, {"status": "run_blocked", "reason": "WM_PROJECT_DIR is not set."})

        self.assertIn("OpenFOAM run failed.", reply)
        self.assertIn("WM_PROJECT_DIR is not set.", reply)
        self.assertNotIn("Suggested action:", reply)

    def test_run_reply_reports_result_review_diagnostics(self) -> None:
        task = task_with_gate_reviews(
            [
                {
                    "gate": "result_review",
                    "status": "failed",
                    "issues": [
                        {
                            "code": "result.residual_high",
                            "message": "log.simpleFoam final residual for U is 12, above 1.",
                        }
                    ],
                    "diagnostics": {
                        "severity": "failed",
                        "categories": ["numerics"],
                        "summary": "numerics: log.simpleFoam final residual for U is 12, above 1.",
                        "primary_issue": {
                            "code": "result.residual_high",
                            "message": "log.simpleFoam final residual for U is 12, above 1.",
                            "severity": "error",
                        },
                        "evidence": {
                            "log_path": "logs/simpleFoam.log",
                            "line": 24,
                            "excerpt": "Solving for U, Initial residual = 0.1, Final residual = 12",
                        },
                    },
                }
            ]
        )

        reply = run_reply(task, {"status": "run_completed"})

        self.assertIn("OpenFOAM run completed.", reply)
        self.assertIn("Result review failed.", reply)
        self.assertIn("Main area: numerics.", reply)
        self.assertIn("Main issue: log.simpleFoam final residual for U is 12, above 1.", reply)
        self.assertIn("Evidence: logs/simpleFoam.log:24", reply)
        self.assertIn("Final residual = 12", reply)
        self.assertNotIn("C:/tmp/task/agent_manifest_v1.json", reply)

    def test_run_reply_reports_passed_result_review(self) -> None:
        task = task_with_gate_reviews(
            [
                {
                    "gate": "result_review",
                    "status": "passed",
                    "issues": [],
                    "diagnostics": {"severity": "passed", "categories": [], "summary": "Result review passed."},
                }
            ]
        )

        reply = run_reply(task, {"status": "run_completed"})

        self.assertIn("Result review passed.", reply)
        self.assertIn("AppFlow manifest is ready.", reply)


if __name__ == "__main__":
    unittest.main()
