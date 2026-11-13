"""Give each main workflow run a unique patch version; reruns keep that version."""
import json
import os
from pathlib import Path
import re


def release_version(base: str, run_number: str) -> str:
    if not re.fullmatch(r"(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)", base):
        raise ValueError("vcpkg version-string must be major.minor.patch")
    if not re.fullmatch(r"[1-9][0-9]*", run_number):
        raise ValueError("GITHUB_RUN_NUMBER must be a positive integer")
    major, minor, patch = map(int, base.split("."))
    return f"{major}.{minor}.{patch + int(run_number)}"


if __name__ == "__main__":
    manifest = json.loads(Path("vcpkg.json").read_text(encoding="utf-8"))
    print(f"version={release_version(manifest['version-string'], os.environ['GITHUB_RUN_NUMBER'])}")
