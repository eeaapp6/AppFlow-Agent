__all__ = ["OpenFOAMAdapter"]


def __getattr__(name: str):
    if name == "OpenFOAMAdapter":
        from .adapter import OpenFOAMAdapter

        return OpenFOAMAdapter
    raise AttributeError(name)
