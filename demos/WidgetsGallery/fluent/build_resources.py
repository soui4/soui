#!/usr/bin/env python3
"""Run the actual uiresbuilder for selected Fluent demos and normalize its whitespace.

Generated IDs and resources come exclusively from the builder, not from this script.
The normalization only removes trailing spaces/tabs; BOMs and line endings survive.
"""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[2]


def main():
    demos = json.loads(Path(__file__).with_name("contracts.json").read_text(encoding="utf-8"))["demos"]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--demo", action="append", choices=demos)
    parser.add_argument("--builder", type=Path, default=REPO / "tools" / ("uiresbuilder.exe" if sys.platform == "win32" else "uiresbuilder"))
    args = parser.parse_args()
    builder = args.builder.resolve(strict=True)
    for demo in args.demo or demos:
        directory = REPO / "demos" / demo
        outputs = (directory / "res/soui_res.rc2", directory / "res/resource.h")
        # The builder's header stamp omits index-only changes (e.g. a new PNG
        # section). Generate into fresh paths instead of trusting that cache.
        # Publish neither file when the builder fails or an output is missing.
        with tempfile.TemporaryDirectory(prefix="soui-fluent-res-") as temporary:
            staged = tuple(Path(temporary) / output.name for output in outputs)
            subprocess.run([str(builder), "-i", "uires/uires.idx", "-p", "uires",
                            "-r", str(staged[0]), "-h", str(staged[1]), "idtable"],
                           cwd=directory, check=True)
            generated = [re.sub(rb"[ \t]+(?=\r?$)", b"", source.read_bytes(), flags=re.MULTILINE)
                         for source in staged]
            for output, data in zip(outputs, generated):
                if not output.is_file() or output.read_bytes() != data:
                    output.write_bytes(data)
        print(f"{demo}: fresh builder outputs synchronized; generated whitespace normalized")


if __name__ == "__main__":
    main()
