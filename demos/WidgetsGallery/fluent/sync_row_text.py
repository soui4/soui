#!/usr/bin/env python3
"""Sync only the bounded Fluent row-text policy; never regenerate palettes/layouts.

Explicit targets are required: --demo demo2 [--demo wallpaper] [--check].
Only demo2 consumes fluent_row_label; wallpaper needs the listbox default only.
XML parsing identifies live nodes; byte-range edits preserve comments, BOM,
line endings, attribute order/quotes and all unmanaged resource content.
"""

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import xml.parsers.expat as expat
from xml.sax.saxutils import escape, quoteattr

REPO = Path(__file__).resolve().parents[2]
TARGETS = {"demo2": True, "wallpaper": False}
CLASS_NAME = "fluent_row_label"
_ATTRIBUTE = re.compile(rb"([^\s=<>/]+)\s*=\s*(['\"])(.*?)\2", re.DOTALL)


@dataclass
class _Tag:
    path: tuple
    attributes: dict
    start: int
    end: int
    close: int = 0


def _tag_end(data, start):
    quote = None
    for index in range(start, len(data)):
        char = data[index]
        if quote is not None:
            if char == quote:
                quote = None
        elif char in (34, 39):
            quote = char
        elif char == 62:
            return index + 1
    raise ValueError("Unterminated XML tag")


def _tags(data):
    parser = expat.ParserCreate()
    stack, tags = [], []

    def start(name, attributes):
        index = parser.CurrentByteIndex
        path = tuple(tag.path[-1] for tag in stack) + (name,)
        tag = _Tag(path, attributes, index, _tag_end(data, index))
        tags.append(tag)
        stack.append(tag)

    def end(name):
        stack.pop().close = parser.CurrentByteIndex

    parser.StartElementHandler = start
    parser.EndElementHandler = end
    try:
        parser.Parse(data, True)
    except expat.ExpatError as error:
        raise ValueError(f"Invalid UIDEF XML: {error}") from error
    if not tags or tags[0].path != ("UIDEF",):
        raise ValueError("Expected a UIDEF root")
    return tags


def _one(tags, path):
    matches = [tag for tag in tags if tag.path == path]
    if len(matches) != 1:
        raise ValueError(f"Expected one {'/'.join(path)}; found {len(matches)}")
    return matches[0]


def _attribute_edit(data, tag, name, value):
    if tag.attributes.get(name) == value:
        return None
    for match in _ATTRIBUTE.finditer(data, tag.start, tag.end):
        if match.group(1) == name.encode("ascii"):
            quote = match.group(2).decode("ascii")
            entity = "&quot;" if quote == '"' else "&apos;"
            replacement = escape(value, {quote: entity}).encode("utf-8")
            return (*match.span(3), replacement)
    # Insert before trailing whitespace, retaining the original closing token.
    index = tag.end - (2 if data[tag.end - 2:tag.end] == b"/>" else 1)
    while index > tag.start and data[index - 1:index].isspace():
        index -= 1
    return index, index, (" " + name + "=" + quoteattr(value)).encode("utf-8")


def _class_insertion(data, style, classes, padding):
    if data[style.close:style.close + 2] != b"</":
        raise ValueError("Expected an explicit closing style tag")
    fragment = (f'<class name="{CLASS_NAME}" padding={quoteattr(padding)}/>').encode("utf-8")
    line = data.rfind(b"\n", 0, style.close) + 1
    indent = data[line:style.close]
    if indent.strip():
        return style.close, style.close, fragment  # Preserve compact XML.
    child_indent = indent + b"  "
    if classes:
        child_line = data.rfind(b"\n", 0, classes[0].start) + 1
        existing_indent = data[child_line:classes[0].start]
        if not existing_indent.strip():
            child_indent = existing_indent
    newline = b"\r\n" if b"\r\n" in data else b"\n"
    return line, line, child_indent + fragment + newline


def sync_demo(demo, repo=REPO, check=False):
    """Return whether the selected local UIDEF differs; check mode never writes."""
    if demo not in TARGETS:
        raise ValueError(f"Unsupported row-policy target: {demo}")
    repo = Path(repo).resolve()
    relative = Path("demos") / demo / "uires/uidef/init.xml"
    path = repo / relative
    if path.resolve() != path:
        raise ValueError("Refusing a redirected UIDEF path")
    source = _tags((repo / "demos/fluent/controls.xml").read_bytes())
    source_classes = [tag for tag in source
                      if tag.path == ("UIDEF", "style", "class")
                      and tag.attributes.get("name") == CLASS_NAME]
    if len(source_classes) != 1 or set(source_classes[0].attributes) != {"name", "padding"}:
        raise ValueError("Expected one padding-only fluent_row_label source class")
    padding = source_classes[0].attributes["padding"]
    text_x = _one(source, ("UIDEF", "objattr", "listbox")).attributes.get("text-x")
    if not text_x:
        raise ValueError("Shared listbox text-x policy is missing")
    data = path.read_bytes()
    tags = _tags(data)
    edits = [_attribute_edit(data, _one(tags, ("UIDEF", "objattr", "listbox")), "text-x", text_x)]
    if TARGETS[demo]:
        style = _one(tags, ("UIDEF", "style"))
        classes = [tag for tag in tags if tag.path == ("UIDEF", "style", "class")]
        rows = [tag for tag in classes if tag.attributes.get("name") == CLASS_NAME]
        if len(rows) > 1:
            raise ValueError("Duplicate fluent_row_label classes")
        edits.append(_attribute_edit(data, rows[0], "padding", padding) if rows
                     else _class_insertion(data, style, classes, padding))
    updated = data
    for start, end, replacement in sorted((edit for edit in edits if edit is not None), reverse=True):
        updated = updated[:start] + replacement + updated[end:]
    _tags(updated)  # Validate all edits before touching the file.
    changed = updated != data
    if changed and not check:
        path.write_bytes(updated)
    return changed


def main(argv=None, repo=REPO):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--demo", action="append", choices=tuple(TARGETS), required=True)
    parser.add_argument("--check", action="store_true", help="Report drift without writing any files")
    args = parser.parse_args(argv)
    dirty = False
    for demo in dict.fromkeys(args.demo):
        try:
            changed = sync_demo(demo, repo, check=args.check)
        except (OSError, ValueError) as error:
            parser.error(str(error))
        dirty = dirty or changed
        result = "needs sync" if args.check and changed else "synced" if changed else "up to date"
        print(f"{demo}: row-text policy {result}")
    return int(args.check and dirty)


if __name__ == "__main__":
    raise SystemExit(main())
