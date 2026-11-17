#!/usr/bin/env python3
"""Materialize self-contained demo palettes from the checked-in dpidemo reference.

No system resource is modified. Run from any directory, then run the target's
existing build_rc2.bat/uiresbuilder command to refresh its PE resource outputs.
"""
import argparse
import copy
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

REPO = Path(__file__).resolve().parents[2]
REFERENCE = REPO / "demos/dpidemo/uires"
OMITTED = {
    "chat_bg", "shape_test", "fluent_overlay", "fluent_tile_accent",
    "fluent_tile_neutral", "fluent_window_shadow", "fluent_popup_shadow",
    "fluent_btn_icon_menu",
}


def write_xml(path, root):
    ET.indent(root, space="  ")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(ET.tostring(root, encoding="utf-8", xml_declaration=True) + b"\n")


def materialize(demo):
    folder = REPO / "demos" / demo / "uires"
    if not folder.is_dir() or folder.resolve().parent.parent != (REPO / "demos").resolve():
        raise ValueError(f"Not a demo resource directory: {demo}")
    idx_path = folder / "uires.idx"
    idx = ET.parse(idx_path).getroot()
    sources = {
        (group.tag.casefold(), item.get("name", "").casefold()): item
        for group in ET.parse(REFERENCE / "uires.idx").getroot()
        for item in group
    }
    groups = {group.tag.casefold(): group for group in idx}

    def register(kind, name, path):
        key = kind.casefold()
        group = groups.get(key)
        if group is None:
            group = ET.SubElement(idx, kind)
            groups[key] = group
        existing = next((n for n in group if n.get("name", "").casefold() == name.casefold()), None)
        if existing is None:
            existing = ET.SubElement(group, "file", name=name)
        existing.set("path", str(path).replace("/", "\\"))

    for variant in ("light", "dark"):
        pool = ET.parse(REFERENCE / "values" / f"fluent_{variant}.xml").getroot()
        for skin in list(pool):
            if skin.get("name") in OMITTED:
                pool.remove(skin)
        # Constructors and message-box templates can request this system name directly.
        button = copy.deepcopy(next(n for n in pool if n.get("name") == "fluent_btn_secondary"))
        button.set("name", "_skin.sys.btn.normal")
        pool.append(button)
        for skin in pool:
            source = skin.get("src")
            if not source:
                continue
            kind, name = source.split(":", 1)
            resource = sources[(kind.casefold(), name.casefold())]
            source_path = REFERENCE / resource.get("path").replace("\\", "/")
            relative = Path("svg/fluent") / variant / source_path.name
            destination = folder / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source_path, destination)
            register(kind, name, relative)
        write_xml(folder / "values" / f"fluent_{variant}.xml", pool)
        colors = ET.parse(REFERENCE / "values" / f"theme_color_{variant}.xml").getroot()
        write_xml(folder / "values" / f"theme_color_{variant}.xml", colors)
        register("values", f"fluent_{variant}", Path("values") / f"fluent_{variant}.xml")
        register("values", f"theme_color_{variant}", Path("values") / f"theme_color_{variant}.xml")
    write_xml(idx_path, idx)
    print(f"{demo}: copied paired palettes and registered self-contained SVG resources")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--demo", action="append", required=True, help="Explicit demo directory; can be repeated")
    args = parser.parse_args()
    for name in args.demo:
        materialize(name)
