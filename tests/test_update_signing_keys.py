"""Regression checks for update signing key consistency."""
import re
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
PUBLIC_KEY = ROOT / "config/update-public-key.pem"
UPDATE_TRUST_TEMPLATE = ROOT / "config/UpdateTrust.h.in"


class UpdateSigningKeyTests(unittest.TestCase):
    def test_committed_public_key_matches_application_template(self):
        public_key = PUBLIC_KEY.read_text(encoding="ascii").strip()
        template = UPDATE_TRUST_TEMPLATE.read_text(encoding="utf-8")
        match = re.search(r"-----BEGIN PUBLIC KEY-----.*?-----END PUBLIC KEY-----", template, re.DOTALL)
        self.assertIsNotNone(match)
        self.assertEqual(public_key, match.group(0).strip())


if __name__ == "__main__":
    unittest.main()
