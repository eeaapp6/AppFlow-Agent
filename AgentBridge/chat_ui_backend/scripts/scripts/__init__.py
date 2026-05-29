from pathlib import Path


# Keeps `scripts.simagent_core...` imports working when cwd is this scripts directory.
__path__ = [str(Path(__file__).resolve().parents[1])]
