"""Sign the exact release manifest with the offline Ed25519 update key.

The CI secret INVOICE_UPDATE_SIGNING_KEY contains PEM text. Local release owners
can use --key-file instead. Only the public key is committed or shipped.
"""
import argparse
import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

MAXIMUM_SIZE = 256 * 1024 * 1024
REPOSITORY = "FabianDim/InvokeInvoiceSystem"


def run_openssl(openssl, *args):
    result = subprocess.run([openssl, *map(str, args)], capture_output=True, check=False)
    if result.returncode:
        raise ValueError("OpenSSL signing/verification failed; check the signing key and public key.")
    return result.stdout


def sign_release(version, asset, output, public_key, key_file=None, openssl="openssl"):
    if not re.fullmatch(r"(0|[1-9][0-9]{0,8})\.(0|[1-9][0-9]{0,8})\.(0|[1-9][0-9]{0,8})", version):
        raise ValueError("Version must be major.minor.patch.")
    asset = Path(asset)
    expected = f"InvokeInvoiceSystem-{version}-windows-x64.exe"
    if asset.name != expected or not asset.is_file() or not 0 < asset.stat().st_size <= MAXIMUM_SIZE:
        raise ValueError("Expected a versioned standalone Windows x64 EXE within the size limit.")
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    secret = os.environ.pop("INVOICE_UPDATE_SIGNING_KEY", "")
    if not key_file and not secret:
        raise ValueError("Configure the INVOICE_UPDATE_SIGNING_KEY repository secret before publishing updates.")
    with tempfile.TemporaryDirectory(prefix="invoice-sign-") as temporary:
        temporary = Path(temporary)
        if key_file:
            private = Path(key_file)
        else:
            private = temporary / "private.pem"
            private.write_text(secret, encoding="ascii")
            private.chmod(0o600)
        secret = ""
        derived_public = run_openssl(openssl, "pkey", "-in", private, "-pubout")
        normalize = lambda data: b"".join(data.split())
        if normalize(derived_public) != normalize(Path(public_key).read_bytes()):
            raise ValueError("Signing key does not match the public key embedded in the app.")
        with asset.open("rb") as file:
            digest = hashlib.file_digest(file, "sha256").hexdigest()
        now = dt.datetime.now(dt.timezone.utc).replace(microsecond=0)
        iso = lambda value: value.isoformat().replace("+00:00", "Z")
        manifest = dict(schema=1, repository=REPOSITORY, platform="windows-x64", version=version,
                        asset=asset.name, size=asset.stat().st_size, sha256=digest,
                        published_utc=iso(now), expires_utc=iso(now + dt.timedelta(days=180)))
        source = temporary / "manifest.json"
        signature = temporary / "manifest.sig"
        source.write_bytes(json.dumps(manifest, separators=(",", ":"), sort_keys=True).encode("utf-8"))
        run_openssl(openssl, "pkeyutl", "-sign", "-rawin", "-inkey", private, "-in", source, "-out", signature)
        if signature.stat().st_size != 64:
            raise ValueError("The update signing key must be Ed25519.")
        run_openssl(openssl, "pkeyutl", "-verify", "-rawin", "-pubin", "-inkey", public_key,
                    "-in", source, "-sigfile", signature)
        (output / "update-windows-x64.json").write_bytes(source.read_bytes())
        (output / "update-windows-x64.sig").write_bytes(signature.read_bytes())
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", required=True)
    parser.add_argument("--asset", required=True)
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--public-key", default="config/update-public-key.pem")
    parser.add_argument("--key-file")
    parser.add_argument("--openssl", default="openssl")
    args = parser.parse_args()
    try:
        sign_release(args.version, args.asset, args.output_dir, args.public_key, args.key_file, args.openssl)
    except (ValueError, OSError) as error:
        parser.exit(1, f"Update signing failed: {error}\n")
    print("Signed and verified update-windows-x64.json and update-windows-x64.sig")


if __name__ == "__main__":
    main()
