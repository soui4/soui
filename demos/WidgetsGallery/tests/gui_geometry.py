"""Windows-only owned-window resize/maximize/restore verification.

Run from any directory after building the Debug gallery. Uses existing GUI QA
libraries, captures only this process, and writes evidence under output/.
"""
import importlib.util
import json
import subprocess
import sys
from pathlib import Path

repo = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location("gallery_gui_wrapper", repo / "demos/SOUIWidgetsGalleryDemo/gui_smoke.py")
wrapper = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = wrapper
spec.loader.exec_module(wrapper)
smoke = wrapper.smoke
args, _, _ = smoke.options(["--demo", "SOUIWidgetsGalleryDemo", "--single-theme", "--dwell", "0.2"])
smoke.enable_dpi_awareness()
exe = repo / "build/bin/Debug/SOUIWidgetsGalleryDemo.exe"
out = repo / "output/gallery-geometry"
out.mkdir(parents=True, exist_ok=True)
evidence = {"status": "failed", "captures": []}
for page in ("progress", "home", "listview", "commands"):
    process = subprocess.Popen([str(exe), "--snapshot", "--page=" + page], cwd=exe.parent, creationflags=subprocess.CREATE_NO_WINDOW)
    session = smoke.WindowsSession(process, "SOUIWidgetsGalleryDemo", args)
    try:
        session.find_main()
        session.owner(session.hwnd)
        with session.dpi.scope(session.hwnd):
            session.gui.SetWindowPos(session.hwnd, 0, 0, 0, 1040, 700, session.con.SWP_NOMOVE | session.con.SWP_NOZORDER | session.con.SWP_NOACTIVATE)
        session.baseline = session.snapshot(session.hwnd)
        session.wait(0.3)
        labels = ["narrow_" + page]
        if page == "progress":
            labels += ["maximized", "restored"]
        for label in labels:
            if label == "maximized":
                session.send(session.hwnd, session.con.WM_SYSCOMMAND, session.con.SC_MAXIMIZE)
            elif label == "restored":
                session.send(session.hwnd, session.con.WM_SYSCOMMAND, session.con.SC_RESTORE)
            session.baseline = session.snapshot(session.hwnd)
            session.wait(0.3)
            metadata, image = session.capture()
            target = out / (label + ".png")
            image.save(target)
            metadata["image"] = str(target)
            evidence["captures"].append({"label": label, **metadata})
    finally:
        evidence.setdefault("cleanup", []).append(session.close())
        (out / "evidence.json").write_text(json.dumps(evidence, indent=2), encoding="utf-8")
assert evidence["captures"][1]["maximized"]
assert not evidence["captures"][2]["maximized"]
assert evidence["captures"][0]["client_rect"] == evidence["captures"][2]["client_rect"]
evidence["status"] = "passed"
evidence["scope"] = "PID-owned resize/system commands/PrintWindow only; no foreground activation or desktop input"
(out / "evidence.json").write_text(json.dumps(evidence, indent=2), encoding="utf-8")
print(json.dumps({"status": evidence.get("status"), "evidence": str(out / "evidence.json")}))
