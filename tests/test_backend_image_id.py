"""Regression checks for the Heroku workflow's build output handling."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / ".github/scripts/backend-image-id.py"
spec = importlib.util.spec_from_file_location("backend_image_id", SCRIPT)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class BackendImageIdTests(unittest.TestCase):
    def test_uses_config_digest_not_manifest_digest(self):
        config = "sha256:" + "a" * 64
        metadata = json.dumps({"containerimage.config.digest": config,
                               "containerimage.digest": "sha256:" + "b" * 64})
        self.assertEqual(module.image_id(metadata), config)

    def test_rejects_empty_malformed_or_wrong_shape_metadata(self):
        for metadata in ("", " ", "not json", "{", "null", "[]", '"text"', "{}"):
            with self.subTest(metadata=metadata), self.assertRaises(ValueError):
                module.image_id(metadata)

    def test_rejects_missing_or_invalid_config_digest(self):
        for value in (None, 42, "", "sha256:abc", "sha256:" + "g" * 64,
                      "sha256:" + "a" * 64 + "\nimage_id=other"):
            with self.subTest(value=value), self.assertRaises(ValueError):
                module.image_id(json.dumps({"containerimage.config.digest": value}))
        with self.assertRaises(ValueError):
            module.image_id(json.dumps({"containerimage.digest": "sha256:" + "b" * 64}))

    def test_cli_appends_valid_output_and_writes_nothing_on_failure(self):
        config = "sha256:" + "c" * 64
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "outputs"
            output.write_text("existing=value\n", encoding="utf-8")
            env = dict(os.environ, GITHUB_OUTPUT=str(output), BUILD_METADATA="")
            failed = subprocess.run([sys.executable, str(SCRIPT)], env=env, capture_output=True, text=True)
            self.assertEqual(failed.returncode, 1)
            self.assertIn("::error::", failed.stderr)
            self.assertNotIn("Traceback", failed.stderr)
            self.assertEqual(output.read_text(), "existing=value\n")
            env["BUILD_METADATA"] = json.dumps({"containerimage.config.digest": config})
            success = subprocess.run([sys.executable, str(SCRIPT)], env=env, capture_output=True, text=True)
            self.assertEqual(success.returncode, 0, success.stderr)
            self.assertEqual(output.read_text(), f"existing=value\nimage_id={config}\n")


if __name__ == "__main__":
    unittest.main()
