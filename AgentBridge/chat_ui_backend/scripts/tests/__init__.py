import sys
from pathlib import Path


SCRIPTS_ROOT = Path(__file__).resolve().parents[1]
CHAT_UI_BACKEND_ROOT = SCRIPTS_ROOT.parent

for path in (SCRIPTS_ROOT, CHAT_UI_BACKEND_ROOT):
    text = str(path)
    if text not in sys.path:
        sys.path.insert(0, text)
