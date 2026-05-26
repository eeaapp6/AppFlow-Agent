from dataclasses import asdict, dataclass, field


@dataclass
class GateIssue:
    code: str
    message: str
    severity: str = "error"

    def to_dict(self) -> dict:
        return asdict(self)


@dataclass
class GateResult:
    gate: str
    status: str = "passed"
    issues: list[GateIssue] = field(default_factory=list)
    metadata: dict = field(default_factory=dict)

    def add_error(self, code: str, message: str) -> None:
        self.status = "failed"
        self.issues.append(GateIssue(code=code, message=message, severity="error"))

    def add_warning(self, code: str, message: str) -> None:
        if self.status == "passed":
            self.status = "warning"
        self.issues.append(GateIssue(code=code, message=message, severity="warning"))

    def to_dict(self) -> dict:
        data = {
            "gate": self.gate,
            "status": self.status,
            "issues": [item.to_dict() for item in self.issues],
        }
        data.update(self.metadata)
        return data
