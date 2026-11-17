#!/usr/bin/env python3
"""Synchronize resource IDs and PE data using the real SOUI uiresbuilder.

Fresh staging avoids the builder's index-only header cache; no generated IDs
are invented here. Normalize only trailing spaces/tabs, preserving BOM/CRLF.
Ignore timestamp-only differences when publishing unchanged generated output.
"""
import argparse
import re
import subprocess
import tempfile
from pathlib import Path

DEMO = Path(__file__).resolve().parent
BUILDER = DEMO.parents[1] / "tools/uiresbuilder.exe"


def resource_payload(data):
    # The builder stamp depends on input mtimes, not semantic content. Keeping
    # the previous output when only that stamp changes avoids rebuild churn.
    return re.sub(rb"\A(\xef\xbb\xbf)?//stamp:[0-9a-fA-F]+\r?\n", rb"\1", data, count=1)


def sync_outputs(outputs, data, check=False):
    stale = []
    for target, content in zip(outputs, data):
        if not target.is_file() or resource_payload(target.read_bytes()) != resource_payload(content):
            stale.append(target.name)
            if not check:
                target.write_bytes(content)
    return stale


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Check generated output without publishing files")
    args = parser.parse_args()
    outputs = (DEMO / "res/soui_res.rc2", DEMO / "res/resource.h")
    with tempfile.TemporaryDirectory(prefix="soui-gallery-res-") as temporary:
        staged = tuple(Path(temporary) / target.name for target in outputs)
        subprocess.run([str(BUILDER.resolve(strict=True)), "-i", "uires/uires.idx", "-p", "uires",
                        "-r", str(staged[0]), "-h", str(staged[1]), "idtable"], cwd=DEMO, check=True)
        data = [re.sub(rb"[ \t]+(?=\r?$)", b"", source.read_bytes(), flags=re.MULTILINE) for source in staged]
        stale = sync_outputs(outputs, data, args.check)
    if args.check and stale:
        parser.exit(1, "Stale resource outputs: " + ", ".join(stale) + "\n")
    print("Gallery resource header and PE index synchronized")


if __name__ == "__main__":
    main()
