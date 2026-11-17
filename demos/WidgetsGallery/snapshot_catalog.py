#!/usr/bin/env python3
"""Capture live Gallery deep links without desktop input or foreground activation.

Uses the same PID-owned PrintWindow/cleanup verifier. --snapshot starts the
window without activation; --page / --section / --dark are normal demo deep
links, not fabricated control notifications. Screenshot checks still cannot
prove input behavior (covered by test_gui_gallery.cpp).
"""
import argparse
import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from uuid import uuid4

import gui_smoke


def capture_page(page, dark, section, source, output, settings, limits, modal=False):
    smoke = gui_smoke.smoke
    repo = Path(__file__).resolve().parents[2]
    exe = repo / "build/bin/Debug/SOUIWidgetsGalleryDemo.exe"
    command = [str(exe), "--snapshot", "--page=" + page]
    if section:
        command.append("--section=" + section)
    if dark:
        command.append("--dark")
    if source:
        command.append("--source")
    if modal:
        command.append("--modal")
    process = subprocess.Popen(command, cwd=exe.parent, creationflags=subprocess.CREATE_NO_WINDOW)
    session = smoke.WindowsSession(process, "SOUIWidgetsGalleryDemo", settings)
    label = page + ("_dark" if dark else "_light") + ("_modal" if modal else "")
    evidence = {"status": "failed", "command": command, "scope": "PID-owned PrintWindow, no desktop input"}
    try:
        session.find_main()
        session.wait(0.5)
        metadata, image = session.capture()
        metadata["statistics"] = smoke.image_stats(image, limits)
        if not metadata["statistics"]["nonblack"]:
            raise smoke.SmokeError("Black/nearly-black screenshot")
        path = output / (label + ".png")
        image.save(path)
        evidence.update(status="passed", capture=metadata, image=str(path))
    except Exception as exc:
        evidence["error"] = str(exc)
        raise
    finally:
        evidence["cleanup"] = session.close()
        (output / (label + ".json")).write_text(json.dumps(evidence, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return evidence


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--page", default="listview", choices=gui_smoke.PAGE_KEYS)
    parser.add_argument("--section", default="")
    parser.add_argument("--dark", action="store_true")
    parser.add_argument("--source", action="store_true")
    parser.add_argument("--modal", action="store_true", help="Open the live in-window modal conversation")
    parser.add_argument("--all", action="store_true", help="Capture all categories in both themes, sequentially")
    args = parser.parse_args()
    smoke = gui_smoke.smoke
    settings, _, limits = smoke.options(["--demo", "SOUIWidgetsGalleryDemo", "--single-theme"])
    smoke.enable_dpi_awareness()
    repo = Path(__file__).resolve().parents[2]
    output = repo / "output/gallery-snapshots" / (datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S_") + uuid4().hex[:8])
    output.mkdir(parents=True)
    requests = [(page, dark) for dark in (False, True) for page in gui_smoke.PAGE_KEYS] if args.all else [(args.page, args.dark)]
    captures = []
    for page, dark in requests:
        captures.append(capture_page(page, dark, args.section, args.source, output, settings, limits, args.modal))
    evidence = {"status": "passed", "captures": captures, "scope": "No foreground activation or desktop input; not input acceptance"}
    (output / "evidence.json").write_text(json.dumps(evidence, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"status": "passed", "captures": len(captures), "output": str(output)}, ensure_ascii=False))


if __name__ == "__main__":
    main()
