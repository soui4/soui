#!/usr/bin/env python3
"""Generate the gallery's demo-local Fluent palettes, SVGs, styles, and index.

Uses only the standard library and the existing Fluent resource materializer.
Does not write layout XML, C++, CMake, or shared system resources. Existing index
entries outside this generator's resource names are retained. Run with -B to
avoid import caches; --check audits without writing. --allow-pending-layout is
only for parallel authoring before the two registered layouts are available.
"""
import argparse
import copy
import importlib.util
import json
import shutil
import struct
import xml.etree.ElementTree as ET
from pathlib import Path

import gallery_assets

DEMO = Path(__file__).resolve().parent
REPO = DEMO.parents[1]
UIRES = DEMO / "uires"
REFERENCE = REPO / "demos/dpidemo/uires"
PENDING_LAYOUTS = {"xml/gallery.xml", "xml/dialog.xml"}
PALETTES = {
    "light": {
        "gallery_sidebar": "#F7F8FA", "gallery_canvas": "#F3F5F8",
        "gallery_subtle": "#F7F9FC", "gallery_stroke": "#E5E5E5",
        "gallery_success": "#0F7B0F", "gallery_warning": "#B96B00",
        "gallery_lilac": "#7664CA",
    },
    "dark": {
        "gallery_sidebar": "#202020", "gallery_canvas": "#1C1C1C",
        "gallery_subtle": "#252525", "gallery_stroke": "#3A3A3A",
        "gallery_success": "#6CCB5F", "gallery_warning": "#FFBD66",
        "gallery_lilac": "#B8A2FF",
    },
}
# Authored 24px outline icons: one 1.6px stroke and round terminals throughout.
ICONS = {
    "home": '<rect x="4" y="4" width="6" height="6" rx="1.3"/><rect x="14" y="4" width="6" height="6" rx="1.3"/><rect x="4" y="14" width="6" height="6" rx="1.3"/><rect x="14" y="14" width="6" height="6" rx="1.3"/>',
    "buttons": '<rect x="3" y="6" width="18" height="12" rx="4"/><path d="M8 12h8"/>',
    "selection": '<rect x="4" y="4" width="16" height="16" rx="3"/><path d="m8 12 3 3 5-6"/>',
    "input": '<rect x="3" y="5" width="18" height="14" rx="3"/><path d="M8 8v8M6 8h4M6 16h4M13 12h4"/>',
    "combo": '<rect x="3" y="5" width="18" height="14" rx="3"/><path d="M7 12h5m3-1 2 2 2-2"/>',
    "progress": '<rect x="4" y="7" width="3" height="12" rx="1.5"/><rect x="10.5" y="4" width="3" height="15" rx="1.5"/><rect x="17" y="10" width="3" height="9" rx="1.5"/>',
    "slider": '<path d="M3 12h7m8 0h3"/><circle cx="14" cy="12" r="4"/>',
    "list": '<circle cx="5" cy="6" r="1"/><circle cx="5" cy="12" r="1"/><circle cx="5" cy="18" r="1"/><path d="M10 6h10M10 12h10M10 18h7"/>',
    "tree": '<rect x="4" y="3" width="5" height="5" rx="1.2"/><rect x="15" y="10" width="5" height="5" rx="1.2"/><rect x="15" y="18" width="5" height="3" rx="1"/><path d="M6.5 8v11.5H15m-8.5-7H15"/>',
    "tabs": '<path d="M3 9V6a2 2 0 0 1 2-2h5a2 2 0 0 1 2 2v3m0-3h6a2 2 0 0 1 2 2v1"/><rect x="3" y="9" width="18" height="11" rx="2"/>',
    "dialog": '<rect x="3" y="4" width="18" height="16" rx="3"/><path d="M3 9h18M7 13h8M14 16h3M7 16h3"/>',
    "icons": '<rect x="3" y="3" width="18" height="18" rx="3"/><circle cx="8" cy="8" r="1.6"/><path d="m4 17 5-5 4 4 3-3 5 5"/>',
    "type": '<path d="m3 20 6-16 6 16M5.5 14h7M16 9h5m-2.5 0v11"/>',
    "layout": '<rect x="3" y="4" width="18" height="16" rx="3"/><path d="M3 9h18M10 9v11"/>',
    "search": '<circle cx="10.5" cy="10.5" r="6.2"/><path d="m15 15 5 5"/>',
    "chevron": '<path d="m9 6 6 6-6 6"/>',
    "check": '<path d="m5 12 4.5 4.5L19 7"/>',
    "copy": '<rect x="8" y="8" width="12" height="13" rx="2"/><path d="M15 8V5a2 2 0 0 0-2-2H5a2 2 0 0 0-2 2v8a2 2 0 0 0 2 2h3"/>',
    "play": '<path d="M7 4.5v15l12-7.5Z"/>',
    "reset": '<path d="M4 10a8 8 0 1 1 1 8M4 4v6h6"/>',
    "code": '<path d="m7 7-5 5 5 5m10-10 5 5-5 5M14 4l-4 16"/>',
    "info": '<circle cx="12" cy="12" r="9"/><path d="M12 11v6M12 7v.2"/>',
    "arrow": '<path d="M4 12h16m-6-6 6 6-6 6"/>',
    "sun": '<circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M2 12h2M20 12h2M5 5l1.5 1.5M17.5 17.5 19 19M5 19l1.5-1.5M17.5 6.5 19 5"/>',
    "moon": '<path d="M20.5 14.4A9 9 0 0 1 9.6 3.5 9 9 0 1 0 20.5 14.4Z"/>',
    "sparkle": '<path d="m12 3 2.3 6.7L21 12l-6.7 2.3L12 21l-2.3-6.7L3 12l6.7-2.3Z"/>',
}


def write_xml(path, root):
    ET.indent(root, space="  ")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(ET.tostring(root, encoding="utf-8", xml_declaration=True) + b"\n")


def register(index, kind, name, relative):
    group = next((n for n in index if n.tag.casefold() == kind.casefold()), None)
    if group is None:
        group = ET.SubElement(index, kind)
    item = next((n for n in group if n.get("name", "").casefold() == name.casefold()), None)
    if item is None:
        item = ET.SubElement(group, "file", name=name)
    item.set("path", str(relative).replace("/", "\\"))


def svg(width, height, body):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
            f'viewBox="0 0 {width} {height}">\n{body}\n</svg>\n')


def outline(body, color):
    return (f'<g fill="none" stroke="{color}" stroke-width="1.6" '
            f'stroke-linecap="round" stroke-linejoin="round">{body}</g>')


def rounded_strip(fills, stroke=None, radius=4, indicator=None, width=40, height=40):
    frames = []
    for state, fill in enumerate(fills):
        border = f' stroke="{stroke}" stroke-width="1"' if stroke else ""
        frames.append(f'<g id="state{state}" transform="translate({state * width},0)">'
                      f'<rect x="0.5" y="0.5" width="{width - 1}" height="{height - 1}" '
                      f'rx="{radius}" fill="{fill}"{border}/>')
        if indicator:
            frames.append(f'<rect x="1" y="12" width="3" height="16" rx="1.5" fill="{indicator}"/>')
        frames.append('</g>')
    return svg(width * len(fills), height, "\n".join(frames))


def slider_thumb(colors):
    frames = []
    for state, accent in enumerate((colors["fluent_accent"], colors["fluent_accent_hover"],
                                    colors["fluent_accent_pressed"], colors["fluent_text_disabled"])):
        radius = (5, 6, 4, 5)[state]
        frames.append(f'<g transform="translate({state * 24},0)">'
                      f'<circle cx="12" cy="12" r="10" fill="{colors["fluent_card_bg"]}" '
                      f'stroke="{colors["gallery_stroke"]}" stroke-width="1"/>'
                      f'<circle cx="12" cy="12" r="{radius}" fill="{accent}"/></g>')
    return svg(96, 24, "\n".join(frames))


def tree_toggle(colors):
    # STreeCtrl::DrawItem adds 3 to normal/hover/pressed when expanded.
    frames = []
    for state in range(6):
        color = colors["fluent_text_secondary"] if state % 3 == 0 else colors["fluent_accent"]
        path = '<path d="m7 5 5 5-5 5"/>' if state < 3 else '<path d="m5 7 5 5 5-5"/>'
        frames.append(f'<g transform="translate({state * 20},0)">{outline(path, color)}</g>')
    return svg(120, 20, "\n".join(frames))


def tree_checkbox(colors):
    frames = []
    # Unchecked 0..2, checked 3..5, indeterminate 6..8; not radio's 8-state map.
    for state in range(9):
        checked = state >= 3
        fill = colors["fluent_accent"] if checked else colors["fluent_card_bg"]
        border = colors["fluent_accent"] if checked or state % 3 else colors["fluent_text_secondary"]
        body = f'<rect x="2.5" y="2.5" width="15" height="15" rx="3" fill="{fill}" stroke="{border}"/>'
        if checked:
            path = '<path d="m6 10 3 3 5-6"/>' if state < 6 else '<path d="M6 10h8"/>'
            body += outline(path, colors["fluent_on_accent"])
        frames.append(f'<g transform="translate({state * 20},0)">{body}</g>')
    return svg(180, 20, "\n".join(frames))


def hero(colors, dark):
    # Abstract component composition, deliberately no text or raster fragments.
    accent = colors["fluent_accent"]
    card = colors["fluent_card_bg"]
    stroke = colors["gallery_stroke"]
    soft = "#193D50" if dark else "#E5F1FC"
    deeper = "#234354" if dark else "#D5E9FA"
    body = f'''<defs><clipPath id="gallery_hero_clip"><rect width="520" height="240" rx="16"/></clipPath></defs>
<g clip-path="url(#gallery_hero_clip)"><rect width="520" height="240" rx="16" fill="{colors['gallery_subtle']}"/>
<circle cx="419" cy="55" r="105" fill="{soft}"/>
<circle cx="77" cy="230" r="100" fill="{soft}"/>
<rect x="171" y="24" width="238" height="153" rx="13" fill="{deeper}"/>
<rect x="66" y="47" width="277" height="151" rx="12" fill="{card}" stroke="{stroke}"/>
<path d="M67 79h275" fill="none" stroke="{stroke}"/>
<rect x="83" y="60" width="11" height="7" rx="2" fill="{accent}"/>
<rect x="104" y="60" width="72" height="7" rx="3.5" fill="{stroke}"/>
<rect x="83" y="97" width="66" height="82" rx="6" fill="{colors['gallery_subtle']}"/>
<rect x="96" y="108" width="39" height="5" rx="2.5" fill="{accent}"/>
<rect x="96" y="127" width="29" height="5" rx="2.5" fill="{stroke}"/>
<rect x="96" y="146" width="35" height="5" rx="2.5" fill="{stroke}"/>
<rect x="165" y="97" width="79" height="9" rx="4.5" fill="{stroke}"/>
<rect x="165" y="120" width="154" height="6" rx="3" fill="{stroke}"/>
<rect x="165" y="120" width="92" height="6" rx="3" fill="{accent}"/>
<rect x="165" y="147" width="61" height="28" rx="4" fill="{accent}"/>
<rect x="240" y="147" width="78" height="28" rx="4" fill="{card}" stroke="{stroke}"/>
<rect x="283" y="92" width="185" height="113" rx="12" fill="{card}" stroke="{stroke}"/>
<rect x="299" y="109" width="65" height="7" rx="3.5" fill="{stroke}"/>
<rect x="299" y="132" width="103" height="5" rx="2.5" fill="{stroke}"/>
<rect x="299" y="150" width="80" height="5" rx="2.5" fill="{stroke}"/>
<rect x="421" y="129" width="30" height="16" rx="8" fill="{accent}"/>
<circle cx="443" cy="137" r="5" fill="{colors['fluent_on_accent']}"/>
<rect x="299" y="172" width="152" height="5" rx="2.5" fill="{stroke}"/>
<rect x="299" y="172" width="104" height="5" rx="2.5" fill="{colors['gallery_success']}"/>
<circle cx="403" cy="174.5" r="8" fill="{card}" stroke="{stroke}"/>
<circle cx="403" cy="174.5" r="4" fill="{colors['gallery_success']}"/>
<rect x="36" y="121" width="55" height="55" rx="12" fill="{colors['gallery_lilac']}"/>
<g transform="translate(51,136)">{outline(ICONS['sparkle'], '#202020' if dark else '#FFFFFF')}</g>
<circle cx="439" cy="55" r="18" fill="{colors['gallery_warning']}"/>
<g transform="translate(427,43)">{outline(ICONS['check'], '#202020' if dark else '#FFFFFF')}</g></g>'''
    return svg(520, 240, body)


def generate():
    UIRES.mkdir(parents=True, exist_ok=True)
    index_path = UIRES / "uires.idx"
    index = ET.parse(index_path).getroot() if index_path.exists() else ET.Element("resource")
    register(index, "layout", "gallery", "xml/gallery.xml")
    register(index, "layout", "gallery_dialog", "xml/dialog.xml")
    register(index, "layout", "gallery_modal", "xml/gallery_modal.xml")
    register(index, "layout", "gallery_native", "xml/gallery_native.xml")
    register(index, "SMENU", "gallery_actions", "xml/gallery_actions.xml")
    register(index, "SMENU", "gallery_view", "xml/gallery_view.xml")
    register(index, "uidef", "xml_init", "uidef/init.xml")
    write_xml(index_path, index)
    spec = importlib.util.spec_from_file_location("gallery_fluent_resources", REPO / "demos/fluent/generate_resources.py")
    reference = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(reference)
    reference.materialize(DEMO.name)
    index = ET.parse(index_path).getroot()

    for variant, palette in PALETTES.items():
        color_path = UIRES / "values" / f"theme_color_{variant}.xml"
        color_root = ET.parse(color_path).getroot()
        color_nodes = color_root.find("color")
        for name, value in palette.items():
            ET.SubElement(color_nodes, name, value=value)
        colors = {node.tag: node.get("value") for node in color_nodes}
        write_xml(color_path, color_root)
        pool_path = UIRES / "values" / f"fluent_{variant}.xml"
        pool = ET.parse(pool_path).getroot()

        def asset(name, document, variant=variant):
            relative = Path("svg/gallery") / variant / f"{name}.svg"
            destination = UIRES / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_text(document, encoding="utf-8", newline="\n")
            resource = f"GALLERY_{variant.upper()}_{name.upper()}"
            register(index, "SVG", resource, relative)
            return f"SVG:{resource}"

        def skin(tag, name, source, pool=pool, **attributes):
            node = ET.SubElement(pool, tag, name=name, src=source, **attributes)
            return node

        def frame(name, fills, border=None, radius=4, indicator=None, margin=6):
            source = asset(name, rounded_strip(fills, border, radius, indicator))
            skin("imgframe", name, source, states=str(len(fills)), margin=f"{margin},{margin},{margin},{margin}")

        gallery_assets.add_skins(colors, frame, skin, asset, svg)

        selected = "#E9F3FC" if variant == "light" else "#263C48"
        selected_hover = "#DDECF9" if variant == "light" else "#304B5A"
        selected_push = "#D2E5F6" if variant == "light" else "#385463"
        hover, pushed = colors["fluent_fill_hover"], colors["fluent_fill_pressed"]
        frame("gallery_nav", ("none", hover, pushed, "none"))
        frame("gallery_nav_selected", (selected, selected_hover, selected_push, colors["gallery_subtle"]), indicator=colors["fluent_accent"])
        frame("gallery_card", (colors["fluent_card_bg"],), colors["gallery_stroke"], radius=8, margin=9)
        frame("gallery_subtle", (colors["gallery_subtle"],), radius=8, margin=9)
        frame("gallery_badge", (selected,), radius=4)
        frame("gallery_quiet", ("none", hover, pushed, "none"))
        # GalleryButton paints this ring only when keyboard focus is visible.
        # A transparent center leaves primary/secondary button fills untouched.
        focus = svg(24, 24, f'<rect x="1" y="1" width="22" height="22" rx="4" fill="none" stroke="{colors["fluent_text"]}" stroke-width="2"/>')
        skin("imgframe", "gallery_focus", asset("gallery_focus", focus), states="1", margin="7,7,7,7")

        track = "#DFE4EB" if variant == "light" else "#454545"
        for name, fill in (("gallery_progress_track", track),
                           ("gallery_progress_blue", colors["fluent_accent"]),
                           ("gallery_progress_green", colors["gallery_success"]),
                           ("gallery_progress_orange", colors["gallery_warning"]),
                           ("gallery_progress_lilac", colors["gallery_lilac"])):
            source = asset(name, svg(12, 4, f'<rect width="12" height="4" rx="2" fill="{fill}"/>'))
            skin("imgframe", name, source, states="1", margin="2,1,2,1")
        # 32x20 leaves a 16x4 center between 8px cuts. At the intended 16dp
        # height, top/bottom corners meet without a zero-sized source slice.
        # SVG antialiasing is retained for both the track and amber indicator.
        for name, fill in (("gallery_progress_track_pill", track),
                           ("gallery_progress_pill", colors["gallery_warning"])):
            source = asset(name, svg(32, 20, f'<rect width="32" height="20" rx="8" fill="{fill}"/>'))
            skin("imgframe", name, source, states="1", margin="8,8,8,8")
        skin("imglist", "gallery_slider_thumb", asset("gallery_slider_thumb", slider_thumb(colors)), states="4")
        skin("imglist", "gallery_tree_toggle", asset("gallery_tree_toggle", tree_toggle(colors)), states="6")
        skin("imglist", "_skin.sys.tree.checkbox", asset("gallery_tree_checkbox", tree_checkbox(colors)), states="9")
        ET.SubElement(pool, "treelines", name="_skin.sys.tree.lines", lineWidth="1", colorLine=colors["gallery_stroke"], colorCross=colors["fluent_text_secondary"])
        for original, alias in (("gallery_slider_thumb", "_skin.sys.slider.thumb"), ("gallery_tree_toggle", "_skin.sys.tree.toggle"), ("gallery_quiet", "gallery_btn_quiet"), ("gallery_progress_pill", "gallery_progress_amber"), ("gallery_progress_lilac", "gallery_progress_purple")):
            node = copy.deepcopy(next(node for node in pool if node.get("name") == original))
            node.set("name", alias)
            pool.append(node)
        for name, body in ICONS.items():
            skin("imglist", f"gallery_icon_{name}", asset(f"icon_{name}", svg(24, 24, outline(body, colors["fluent_text_secondary"]))), states="1")
        skin("imglist", "gallery_hero", asset("hero", hero(colors, variant == "dark")), states="1")
        for kind in ("window", "popup"):
            filename = f"{kind}_{variant}.png"
            destination = UIRES / "image" / filename
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(REFERENCE / "image" / filename, destination)
            register(index, "img", f"{kind}_{variant}", Path("image") / filename)
            skin("imgframe", f"fluent_{kind}_shadow", f"img:{kind}_{variant}", states="1", margin="26,20,26,32")
        write_xml(pool_path, pool)

    definitions = ET.parse(REPO / "demos/fluent/controls.xml").getroot()
    definitions.insert(0, ET.Element("unit", defUnit="dp"))
    # No fixed color domain here: Theme owns the replaceable light/dark domains.
    style = definitions.find("style")
    style.find("class[@name='fluent_text_secondary']").set("font", "size:14")
    for name, skin_name in (("gallery_nav", "gallery_nav"), ("gallery_nav_selected", "gallery_nav_selected"),
                            ("gallery_card", "gallery_card"), ("gallery_subtle", "gallery_subtle"),
                            ("gallery_badge", "gallery_badge"), ("gallery_quiet", "gallery_quiet")):
        node = ET.SubElement(style, "class", name=name, skin=skin_name, colorText="@color/fluent_text")
        if name in {"gallery_nav", "gallery_nav_selected", "gallery_quiet"}:
            node.set("cursor", "hand")
            for attribute in ("colorTextHover", "colorTextPush"):
                node.set(attribute, "@color/fluent_text")
            node.set("colorTextDisable", "@color/fluent_text_disabled")
    objects = definitions.find("objattr")
    gallery_button = copy.deepcopy(objects.find("button"))
    gallery_button.tag = "gallerybutton"
    objects.append(gallery_button)
    gallery_tabs = copy.deepcopy(objects.find("tabctrl"))
    gallery_tabs.tag = "gallerytabs"
    gallery_tabs.set("drawFocusRect", "0")
    objects.append(gallery_tabs)
    ET.SubElement(objects, "text", noPrefix="1")
    ET.SubElement(objects, "progress", bkgndSkin="gallery_progress_track", posSkin="gallery_progress_blue", colorText="@color/fluent_text", showText="0")
    ET.SubElement(objects, "sliderbar", bkgndSkin="gallery_progress_track", posSkin="gallery_progress_blue", thumbSkin="gallery_slider_thumb", colorText="@color/fluent_text", sepColor="@color/gallery_stroke", showText="0")
    ET.SubElement(objects, "treectrl", toggleSkin="gallery_tree_toggle", checkSkin="_skin.sys.tree.checkbox", lineSkin="_skin.sys.tree.lines", itemSkin="gallery_nav_selected", colorText="@color/fluent_text", colorTextPush="@color/fluent_text", colorItemBkgnd="@color/fluent_card_bg", colorItemSelBkgnd="@color/gallery_subtle", hasLines="0", itemHeight="32dp", itemMargin="8dp", indent="20dp")
    gallery_assets.add_defaults(objects)
    combo_view = copy.deepcopy(objects.find("combobox"))
    combo_view.tag = "comboview"
    objects.append(combo_view)
    write_xml(UIRES / "uidef/init.xml", definitions)
    write_xml(index_path, index)


def audit(allow_pending_layout=False):
    index = ET.parse(UIRES / "uires.idx").getroot()
    assert index.tag == "resource", "Index must have a resource root"
    entries, documents, pending = {}, {}, []
    for group in index:
        for item in group:
            key = (group.tag.casefold(), item.attrib["name"].casefold())
            assert key not in entries, f"Duplicate index ID: {key}"
            relative = item.attrib["path"].replace("\\", "/")
            path = (UIRES / relative).resolve()
            assert path.is_relative_to(UIRES.resolve()), f"Escaping resource path: {relative}"
            entries[key] = path
            if not path.is_file():
                assert allow_pending_layout and relative in PENDING_LAYOUTS, f"Missing resource: {relative}"
                pending.append(relative)
            elif path.suffix in {".xml", ".svg"}:
                documents[path] = ET.parse(path).getroot()
    skins, tokens = {}, {}
    for variant, palette in PALETTES.items():
        pool = documents[(UIRES / f"values/fluent_{variant}.xml").resolve()]
        assert pool.tag == "skin"
        names = [node.attrib["name"] for node in pool]
        assert len(names) == len(set(names)), f"Duplicate {variant} skin"
        skins[variant] = set(names)
        color_root = documents[(UIRES / f"values/theme_color_{variant}.xml").resolve()]
        colors = list(color_root.find("color"))
        tokens[variant] = {node.tag for node in colors}
        assert len(colors) == len(tokens[variant]), f"Duplicate {variant} color token"
        assert palette.keys() <= tokens[variant]
        assert {f"gallery_icon_{name}" for name in ICONS} <= skins[variant]
        for node in pool:
            if "src" not in node.attrib:
                continue
            kind, name = node.attrib["src"].split(":", 1)
            source = entries[(kind.casefold(), name.casefold())]
            assert source.is_file(), f"Unresolved skin source: {node.attrib['src']}"
            if source.suffix != ".svg":
                continue
            asset = documents[source]
            width, height = float(asset.attrib["width"]), float(asset.attrib["height"])
            states = int(node.get("states", "1"))
            assert width > 0 and height > 0 and width % states == 0, f"Invalid SVG state strip: {source}"
            if node.tag == "imgframe" and "margin" in node.attrib:
                left, top, right, bottom = map(int, node.attrib["margin"].split(","))
                assert left + right < width / states and top + bottom < height, f"Invalid nine-slice: {node.attrib['name']}"
        for kind in ("window", "popup"):
            filename = f"{kind}_{variant}.png"
            data = (UIRES / "image" / filename).read_bytes()
            assert data == (REFERENCE / "image" / filename).read_bytes(), f"Chrome must match existing PNG: {filename}"
            assert data[:8] == b"\x89PNG\r\n\x1a\n" and struct.unpack(">II", data[16:24]) == (100, 100)
        hero_path = entries[("svg", f"gallery_{variant}_hero")]
        assert documents[hero_path].attrib["viewBox"] == "0 0 520 240"
        for name in ICONS:
            icon_path = entries[("svg", f"gallery_{variant}_icon_{name}")]
            assert documents[icon_path].attrib["viewBox"] == "0 0 24 24"
    assert skins["light"] == skins["dark"], "Light/dark skin sets differ"
    assert tokens["light"] == tokens["dark"], "Light/dark color sets differ"
    skin_attributes = {"skin", "itemskin", "toggleSkin".casefold(), "checkskin", "lineskin", "thumbskin", "bkgndskin", "trackskin", "posskin", "filledskin", "ncskin", "sbskin", "bandskin", "focusskin", "btnskin", "tabskin", "captionskin", "closebtnskin"}
    for document in documents.values():
        for node in document.iter():
            for attribute, value in node.attrib.items():
                if value.startswith("@color/"):
                    assert value[7:] in tokens["light"], f"Unknown color: {value}"
                if attribute.casefold() in skin_attributes and value.startswith(("gallery_", "fluent_", "_skin.sys.")):
                    assert value in skins["light"], f"Unknown themed skin: {value}"
    definitions = documents[(UIRES / "uidef/init.xml").resolve()]
    assert definitions.find("deffont").get("value") == "face:Segoe UI,size:14"
    assert definitions.find("objattr/text").get("noPrefix") == "1"
    assert definitions.find("objattr/gallerybutton").attrib == definitions.find("objattr/button").attrib
    for variant, palette in PALETTES.items():
        pool = documents[(UIRES / f"values/fluent_{variant}.xml").resolve()]
        by_name = {node.get("name"): node for node in pool}
        for name in ("gallery_progress_track_pill", "gallery_progress_pill", "gallery_progress_amber"):
            node = by_name[name]
            assert node.tag == "imgframe" and node.get("margin") == "8,8,8,8"
            kind, resource = node.get("src").split(":", 1)
            asset = documents[entries[(kind.casefold(), resource.casefold())]]
            assert asset.get("viewBox") == "0 0 32 20"
            assert asset.find("{http://www.w3.org/2000/svg}rect").get("rx") == "8"
        assert by_name["gallery_progress_amber"].get("src") == by_name["gallery_progress_pill"].get("src")
        focus = by_name["gallery_focus"]
        assert focus.tag == "imgframe" and focus.get("states") == "1" and focus.get("margin") == "7,7,7,7"
        kind, resource = focus.get("src").split(":", 1)
        focus_svg = documents[entries[(kind.casefold(), resource.casefold())]]
        assert focus_svg.get("viewBox") == "0 0 24 24"
        ring = focus_svg.find("{http://www.w3.org/2000/svg}rect")
        assert ring.get("fill") == "none" and ring.get("stroke-width") == "2" and ring.get("rx") == "4"
        theme_colors = documents[(UIRES / f"values/theme_color_{variant}.xml").resolve()].find("color")
        assert ring.get("stroke") == theme_colors.find("fluent_text").get("value")
    for control in ("progress", "sliderbar"):
        defaults = definitions.find(f"objattr/{control}")
        assert defaults.get("bkgndSkin") == "gallery_progress_track"
        assert defaults.get("posSkin") == "gallery_progress_blue"
        assert "bgSkin" not in defaults.attrib
    return {"indexed_files": len(entries), "parsed_xml_svg": len(documents),
            "skins_per_theme": len(skins["light"]), "tokens_per_theme": len(tokens["light"]),
            "icons_per_theme": len(ICONS), "pending_layouts": pending,
            "scope": "static index, sources, paired themes, SVG geometry, defaults, and copied chrome; not GUI rendering"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Read-only resource audit")
    parser.add_argument("--allow-pending-layout", action="store_true", help="Allow only gallery.xml/dialog.xml to be absent during parallel authoring")
    args = parser.parse_args()
    if not args.check:
        generate()
    try:
        print(json.dumps(audit(args.allow_pending_layout), indent=2))
    except (AssertionError, OSError, ET.ParseError, KeyError, ValueError) as error:
        parser.exit(1, f"Resource audit failed: {error}\n")


if __name__ == "__main__":
    main()
