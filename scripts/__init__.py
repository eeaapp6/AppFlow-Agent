import sys
from pathlib import Path


# Keeps `scripts.simagent_core...` imports working when tests run from the repo root.
SCRIPTS_ROOT = Path(__file__).resolve().parents[1] / "AgentBridge" / "chat_ui_backend" / "scripts"
scripts_root_text = str(SCRIPTS_ROOT)

if scripts_root_text not in sys.path:
    sys.path.insert(0, scripts_root_text)

__path__ = [scripts_root_text]
