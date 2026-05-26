import re
from pathlib import Path


class CommandHelpDatabase:
    def __init__(self, data_root: str | Path):
        self.data_root = Path(data_root)
        self.command_help_path = self.data_root / "openfoam_command_help.txt"
        self._commands: set[str] | None = None

    @classmethod
    def from_default_data(cls) -> "CommandHelpDatabase":
        return cls(Path(__file__).resolve().parent / "data")

    def is_available(self) -> bool:
        return self.command_help_path.exists()

    def load_command_names(self) -> set[str]:
        if self._commands is not None:
            return self._commands
        if not self.command_help_path.exists():
            raise FileNotFoundError(f"OpenFOAM command help data not found: {self.command_help_path}")

        content = self.command_help_path.read_text(encoding="utf-8", errors="replace")
        self._commands = {
            item.strip()
            for item in re.findall(r"<command>(.*?)</command>", content, flags=re.DOTALL)
            if item.strip()
        }
        return self._commands
