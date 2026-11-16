"""Export the Docker config digest required by Heroku, after a successful push."""
import json
import os
from pathlib import Path
import re
import sys


def image_id(metadata: str) -> str:
    try:
        data = json.loads(metadata)
    except json.JSONDecodeError:
        raise ValueError("Backend build metadata is missing or invalid JSON.") from None
    value = data.get("containerimage.config.digest") if isinstance(data, dict) else None
    if not isinstance(value, str) or not re.fullmatch(r"sha256:[a-f0-9]{64}", value):
        raise ValueError("Backend build metadata has no valid Docker image config digest.")
    return value


if __name__ == "__main__":
    try:
        value = image_id(os.environ.get("BUILD_METADATA", ""))
    except ValueError as error:
        print(f"::error::{error}", file=sys.stderr)
        sys.exit(1)
    with Path(os.environ["GITHUB_OUTPUT"]).open("a", encoding="utf-8") as output:
        output.write(f"image_id={value}\n")
