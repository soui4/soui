#!/usr/bin/env python3
"""Add demo-local preblurred chrome without regenerating any existing palette.

Default: popup light/dark for all 13 Fluent demos; window light/dark only for
WINDOW_TARGETS. Colors come from each demo's existing theme_color_*.xml.
The current dpidemo render_popup supplies 4x supersampling, 100x100 RGBA,
64x64 panel at (18,12), radius 8 and full-gutter window smoothstep. Its current
window and popup blur/offsets are contact 3/0, ambient 5/2, both using
full-gutter smoothstep. Panel colors stay independent.
Nine-slice margins are 26/20/26/32. No runtime or generated PE resources change.

Only missing chrome skins/PNG indices are appended. Existing private entries,
colors, comments, encoding/BOM and line endings are retained. Conflicting
chrome definitions fail rather than being overwritten. Identical reruns do
not rewrite files. --demo may be repeated; --check is read-only and exits 1
when owned resources differ. Uses the already-required offline Pillow.
"""

import argparse
import codecs
from functools import lru_cache
import importlib.util
from io import BytesIO
from pathlib import Path
import re
import xml.etree.ElementTree as ET
import xml.parsers.expat as expat

from PIL import ImageColor

REPO = Path(__file__).resolve().parents[2]
TARGETS = (
    "demo", "demo2", "wxDemo", "flowlayout_demo", "framelayout_demo",
    "gridctrl_demo", "hellocharts_demo", "lightbutton_demo", "mclistview_demo",
    "propanimator_demo", "sci_demo", "wallpaper", "ipcdemo",
)
WINDOW_TARGETS = frozenset({
    "demo", "wxDemo", "flowlayout_demo", "framelayout_demo", "hellocharts_demo",
    "mclistview_demo", "propanimator_demo", "wallpaper", "ipcdemo",
})
# render_popup owns blur/offset/falloff; these match main()'s window_themes.
WINDOW_ALPHAS = {"light": (0.30, 0.08), "dark": (0.34, 0.10)}
MARGIN = "26,20,26,32"


@lru_cache(maxsize=1)
def _renderer():
    source = REPO / "demos/dpidemo/generate_popup_shadow.py"
    spec = importlib.util.spec_from_file_location("fluent_chrome_shadow_reference", source)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class _Xml:
    """Use parsed XML for identity, but insertion offsets for lossless edits."""

    def __init__(self, data, expected_root):
        self.original = data
        self.bom, self.encoding = b"", "utf-8"
        for bom, encoding in ((codecs.BOM_UTF8, "utf-8"),
                              (codecs.BOM_UTF16_LE, "utf-16-le"),
                              (codecs.BOM_UTF16_BE, "utf-16-be")):
            if data.startswith(bom):
                self.bom, self.encoding = bom, encoding
                break
        if not self.bom:
            declaration = re.match(rb"<\?xml\s[^>]*encoding\s*=\s*['\"]([^'\"]+)['\"]", data)
            if declaration:
                self.encoding = declaration[1].decode("ascii")
        self.text = data[len(self.bom):].decode(self.encoding)
        self.root = ET.fromstring(self.text)
        if self.root.tag != expected_root:
            raise ValueError(f"Expected {expected_root} XML root, got {self.root.tag}")
        self.newline = "\r\n" if "\r\n" in self.text else "\n"

    def append(self, parent, lines):
        if not lines:
            return self.original
        # Expat distinguishes real closing tags from lookalikes in comments,
        # CDATA/quoted attributes. Force UTF-8 for its byte offsets only;
        # final encoding/BOM remain those of the original document.
        parser = expat.ParserCreate("utf-8")
        stack, positions = [], {}

        def start(name, attributes):
            stack.append(name)
            path = tuple(stack)
            if path == parent:
                positions["start"] = parser.CurrentByteIndex
            elif path[:-1] == parent:
                positions.setdefault("child", parser.CurrentByteIndex)

        def end(name):
            if tuple(stack) == parent:
                positions["close"] = parser.CurrentByteIndex
            stack.pop()

        parser.StartElementHandler = start
        parser.EndElementHandler = end
        parser.Parse(self.text, True)
        utf8 = self.text.encode("utf-8")
        offsets = {key: len(utf8[:value].decode("utf-8")) for key, value in positions.items()}
        closing = offsets["close"]

        def line_indent(position):
            beginning = self.text.rfind("\n", 0, position) + 1
            indent = self.text[beginning:position]
            return indent if not indent.strip() else None

        if not re.match(r"</" + re.escape(parent[-1]) + r"\s*>", self.text[closing:]):
            # Expand an existing <PNG/> without replacing its sibling entries.
            opening = offsets["start"]
            token = self.text[opening:closing]
            if not token.endswith("/>"):
                raise ValueError("Expected a closing tag or self-closing parent")
            indent = line_indent(opening)
            if indent is None:
                content = "".join(lines)
            else:
                content = self.newline + "".join(indent + "  " + line + self.newline for line in lines) + indent
            replacement = token[:-2] + ">" + content + f"</{parent[-1]}>"
            text = self.text[:opening] + replacement + self.text[closing:]
        else:
            indent = line_indent(closing)
            if indent is None:
                content = "".join(lines)
                insertion = closing
            else:
                child_indent = line_indent(offsets["child"]) if "child" in offsets else None
                if child_indent is None:
                    child_indent = indent + "  "
                content = "".join(child_indent + line + self.newline for line in lines)
                insertion = closing - len(indent)
            text = self.text[:insertion] + content + self.text[insertion:]
        # Existing bytes outside the insertion are never serialized by ET.
        return self.bom + text.encode(self.encoding)


def _path(repo, demo, relative):
    path = repo / "demos" / demo / "uires" / relative
    if path.resolve() != path:
        raise ValueError(f"Refusing redirected resource path: {path}")
    return path


def _theme_colors(path):
    colors = _Xml(path.read_bytes(), "UIDEF").root.find("color")
    if colors is None:
        raise ValueError(f"Missing color table: {path}")
    result = {}
    for name in ("fluent_window_bg", "fluent_card_bg", "fluent_border"):
        nodes = colors.findall(name)
        if len(nodes) != 1 or not nodes[0].get("value"):
            raise ValueError(f"Expected one {name} value: {path}")
        value = nodes[0].get("value").strip()
        if len(ImageColor.getrgb(value)) != 3:
            raise ValueError(f"Expected an opaque RGB color for {name}: {path}")
        result[name] = value
    return result


def _name(kind, theme):
    return f"FL_CHROME_{kind.upper()}_{theme.upper()}"


def _palette_bytes(document, kinds, theme):
    lines = []
    for kind in kinds:
        attributes = {"name": f"fluent_{kind}_shadow", "src": "PNG:" + _name(kind, theme),
                      "states": "1", "margin": MARGIN, "enableColorize": "0"}
        existing = [n for n in document.root
                    if n.get("name", "").casefold() == attributes["name"].casefold()]
        if existing:
            if len(existing) != 1 or existing[0].tag != "imgframe" or existing[0].attrib != attributes:
                raise ValueError(f"Conflicting/duplicate skin: {attributes['name']}")
        else:
            lines.append("<imgframe " + " ".join(f'{key}="{value}"' for key, value in attributes.items()) + " />")
    return document.append(("skin",), lines)


def _index_bytes(document, entries):
    groups = [g for g in document.root if g.tag.casefold() == "png"]
    if len(groups) > 1:
        raise ValueError("Multiple PNG index groups; refusing to merge private entries")
    group = groups[0] if groups else None
    lines = []
    for name, relative in entries:
        existing = [n for n in group if n.get("name", "").casefold() == name.casefold()] if group is not None else []
        if existing:
            if (len(existing) != 1 or existing[0].tag != "file" or existing[0].get("name") != name
                    or existing[0].get("path", "").replace("/", "\\") != relative):
                raise ValueError(f"Conflicting/duplicate PNG index: {name}")
        else:
            lines.append(f'<file name="{name}" path="{relative}" />')
    if group is None:
        return document.append(("resource",), ["<PNG>", *("  " + line for line in lines), "</PNG>"])
    return document.append(("resource", group.tag), lines)


def _read_if_exists(path):
    return path.read_bytes() if path.exists() else None


def generate(demos=None, *, repo=REPO, check=False):
    """Return changed absolute paths; preflight all selected demos before writes."""
    targets = tuple(dict.fromkeys(TARGETS if demos is None else demos))
    if any(demo not in TARGETS for demo in targets):
        raise ValueError(f"Unsupported chrome targets: {targets}")
    repo = Path(repo).resolve()
    renderer = _renderer()
    plan = {}

    def remember(path, original, content):
        if original != content:
            plan[path] = (original, content)

    for demo in targets:
        kinds = ("window", "popup") if demo in WINDOW_TARGETS else ("popup",)
        entries = []
        for theme in ("light", "dark"):
            colors = _theme_colors(_path(repo, demo, f"values/theme_color_{theme}.xml"))
            for kind in kinds:
                name = _name(kind, theme)
                relative = f"image\\fluent_chrome\\{name}.png"
                output = _path(repo, demo, relative.replace("\\", "/"))
                fill = colors["fluent_window_bg" if kind == "window" else "fluent_card_bg"]
                alphas = WINDOW_ALPHAS[theme] if kind == "window" else renderer.THEMES[theme][2:]
                image = renderer.render_popup(fill, colors["fluent_border"], *alphas, window=True)
                buffer = BytesIO()
                image.save(buffer, format="PNG")
                remember(output, _read_if_exists(output), buffer.getvalue())
                entries.append((name, relative))
            palette_path = _path(repo, demo, f"values/fluent_{theme}.xml")
            palette = _Xml(palette_path.read_bytes(), "skin")
            remember(palette_path, palette.original, _palette_bytes(palette, kinds, theme))
        index_path = _path(repo, demo, "uires.idx")
        index = _Xml(index_path.read_bytes(), "resource")
        remember(index_path, index.original, _index_bytes(index, entries))

    if not check:
        # Optimistic preflight: don't replay an older palette/index over another
        # agent's edit while PNG rendering was in progress. Retry instead.
        for path, (original, _) in plan.items():
            if _read_if_exists(path) != original:
                raise ValueError(f"Resource changed during planning; retry: {path}")
        for path, (original, content) in plan.items():
            if _read_if_exists(path) != original:
                raise ValueError(f"Resource changed before writing; retry: {path}")
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(content)
    return list(plan)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--demo", action="append", choices=TARGETS,
                        help="Explicit target; repeatable. Default: all 13 demos.")
    parser.add_argument("--check", action="store_true", help="Check only; never write")
    args = parser.parse_args()
    try:
        changed = generate(args.demo, check=args.check)
    except (OSError, ValueError, ET.ParseError, expat.ExpatError) as error:
        parser.exit(2, f"Chrome resource generation failed: {error}\n")
    action = "differ" if args.check else "updated"
    print(f"{len(changed)} chrome resource files {action}; existing private palettes/colors retained.")
    return 1 if args.check and changed else 0


if __name__ == "__main__":
    raise SystemExit(main())
