"""Demo-local Fluent skin and objattr extensions for generate_assets.py.

Call add_skins once per theme with the generator's palette and callbacks, and
add_defaults with its objattr element before writing init.xml. Importing this
module does not generate files or change shared resources.
"""

import xml.etree.ElementTree as ET


def add_skins(colors, frame, skin, asset, svg):
    """Register horizontal SVG strips using SOUI's native state ordering."""
    card = colors["fluent_card_bg"]
    hover = colors["fluent_fill_hover"]
    pressed = colors["fluent_fill_pressed"]
    subtle = colors["gallery_subtle"]
    stroke = colors["gallery_stroke"]
    text = colors["fluent_text"]
    secondary = colors["fluent_text_secondary"]
    disabled = colors["fluent_text_disabled"]
    accent = colors["fluent_accent"]
    accent_hover = colors["fluent_accent_hover"]
    accent_pressed = colors["fluent_accent_pressed"]

    def outline(body, color):
        return (
            f'<g fill="none" stroke="{color}" stroke-width="1.6" '
            f'stroke-linecap="round" stroke-linejoin="round">{body}</g>'
        )

    def strip(name, width, height, bodies):
        frames = [
            f'<g id="state{index}" transform="translate({index * width},0)">{body}</g>'
            for index, body in enumerate(bodies)
        ]
        source = asset(name, svg(width * len(frames), height, "\n".join(frames)))
        skin("imglist", name, source, states=str(len(frames)),
             **({"checkAsPushdown": "0"} if len(frames) == 8 else {}))

    # SToggle is a skin-only button. Child text is supplied by the XML.
    # Do not map checked to pressed: these strips have eight distinct states.
    fills = (card, hover, pressed, subtle, subtle, hover, pressed, subtle)
    borders = (stroke,) * 4 + (accent, accent_hover, accent_pressed, stroke)
    cells = [
        f'<g transform="translate({index * 40},0)"><rect x="0.5" y="0.5" '
        f'width="39" height="39" rx="4" fill="{fill}" stroke="{border}"/></g>'
        for index, (fill, border) in enumerate(zip(fills, borders))
    ]
    skin("imgframe", "gallery_toggle", asset("gallery_toggle", svg(320, 40, "".join(cells))),
         states="8", margin="6,6,6,6", checkAsPushdown="0")

    # SSwitch always draws track state 0. Only skinforce follows GetState().
    strip(
        "gallery_switch_track",
        44,
        24,
        [
            (
                f'<rect x="1" y="1" width="42" height="22" rx="11" '
                f'fill="{subtle}" stroke="{secondary}" stroke-width="1"/>'
            ),
        ],
    )
    thumb_colors = (
        secondary,
        secondary,
        secondary,
        disabled,
        accent,
        accent_hover,
        accent_pressed,
        disabled,
    )
    strip(
        "gallery_switch_thumb",
        24,
        24,
        [
            f'<circle cx="12" cy="12" r="{(6, 7, 5, 6)[state % 4]}" fill="{color}"/>'
            for state, color in enumerate(thumb_colors)
        ],
    )

    arrow_colors = (secondary, text, accent_pressed, disabled)
    arrows = {
        "up": "m6 12 4-4 4 4",
        "down": "m6 8 4 4 4-4",
        "prev": "m12 6-4 4 4 4",
        "next": "m8 6 4 4-4 4",
    }
    for direction, path in arrows.items():
        strip(
            f"gallery_arrow_{direction}",
            20,
            20,
            [outline(f'<path d="{path}"/>', color) for color in arrow_colors],
        )

    # TreeCtrl indexes six frames directly, whereas TreeView's SToggle uses
    # normal/hover/push/disabled + four checked states. Never share the maps.
    strip(
        "gallery_tree_expander", 20, 20,
        [outline('<path d="' + ("m7 5 5 5-5 5" if state < 4 else "m5 7 5 5 5-5") + '"/>',
                 arrow_colors[state % 4]) for state in range(8)],
    )

    # SHeaderCtrl uses DrawByIndex: HDF_SORTUP is 0, HDF_SORTDOWN is 1.
    strip(
        "gallery_sort",
        16,
        16,
        [
            outline(f'<path d="{path}"/>', secondary)
            for path in ("m4 10 4-4 4 4", "m4 6 4 4 4-4")
        ],
    )

    # These are three icon indices (new/save/pin), not interaction states.
    toolbar_paths = (
        "M5 12h14M12 5v14",
        "M5 4h12l3 3v13H4V4ZM8 4v6h8V4M8 20v-6h8v6",
        "M8 4h8M9 4v6l-3 4v2h12v-2l-3-4V4M12 16v5",
    )
    strip(
        "gallery_toolbar_icons",
        24,
        24,
        [outline(f'<path d="{path}"/>', text) for path in toolbar_paths],
    )

    strip(
        "gallery_image_button",
        40,
        36,
        [
            f'<rect x="0.5" y="0.5" width="39" height="35" rx="4" '
            f'fill="{fill}" stroke="{stroke}" stroke-width="1"/>'
            + outline('<path d="M14 18h12M20 12v12"/>', color)
            for fill, color in zip(
                (card, hover, pressed, subtle), (text, text, secondary, disabled)
            )
        ],
    )

    # Twelve actual frames; no SMIL, raster images or external SVG references.
    strip(
        "gallery_spinner",
        24,
        24,
        [
            f'<circle cx="12" cy="12" r="8" fill="none" '
            f'stroke="{stroke}" stroke-width="2"/>'
            f'<path d="M12 4a8 8 0 1 1-8 8" fill="none" stroke="{accent}" '
            f'stroke-width="2" stroke-linecap="round" '
            f'transform="rotate({state * 30} 12 12)"/>'
            for state in range(12)
        ],
    )
    # Solid geometry also stretches correctly into a horizontal split bar.
    strip(
        "gallery_separator",
        1,
        16,
        [
            f'<rect width="1" height="16" fill="{colors["fluent_divider"]}"/>',
        ],
    )


def add_defaults(objects):
    """Merge demo defaults in place without duplicating existing class nodes."""

    def update(tag, **attributes):
        node = next(
            (child for child in objects if child.tag.casefold() == tag.casefold()),
            None,
        )
        if node is None:
            node = ET.SubElement(objects, tag)
        for name, value in attributes.items():
            # SOUI attribute names are case-insensitive; avoid two spellings.
            for old_name in tuple(node.attrib):
                if old_name != name and old_name.casefold() == name.casefold():
                    del node.attrib[old_name]
            node.set(name, value)

    text = {
        "colorText": "@color/fluent_text",
        "colorTextHover": "@color/fluent_text",
        "colorTextPush": "@color/fluent_text_secondary",
        "colorTextDisable": "@color/fluent_text_disabled",
    }
    # Do not set imgbtn's skin: caption buttons provide their own skins.
    update("imgbtn", **text)
    update("toggle", skin="gallery_toggle", focusSkin="gallery_focus")
    update(
        "switch",
        skin="gallery_switch_track",
        skinforce="gallery_switch_thumb",
        focusable="1",
    )
    update(
        "hotkey",
        skin="fluent_edit",
        colorText="@color/fluent_text",
        padding="8dp,6dp,8dp,6dp",
    )
    update(
        "richedit",
        skin="fluent_edit",
        ncSkin="",
        colorText="@color/fluent_text",
        padding="8dp,6dp,8dp,6dp",
        multiLines="1",
        sbSkin="_skin.sys.scrollbar",
    )
    # SEdit derives from SRichEdit. Keep ordinary edits single-line; local
    # multiLines="1" still opts individual edit controls into multiline mode.
    update("edit", multiLines="0")
    update("spinButton", upSkin="gallery_arrow_up", downSkin="gallery_arrow_down")
    # SCalendar paints its selected accent fill itself before daySkin. An
    # opaque day skin would hide that fill and break on-accent text contrast.
    update(
        "calendar",
        skin="gallery_card",
        colorBkgnd="@color/fluent_card_bg",
        colorText="@color/fluent_text",
        colorSelText="@color/fluent_on_accent",
        colorOtherText="@color/fluent_text_secondary",
        colorSelBg="@color/fluent_accent",
        colorHoverText="@color/fluent_accent",
        prevSkin="gallery_arrow_prev",
        nextSkin="gallery_arrow_next",
        daySkin="",
        weekSkin="",
    )
    update(
        "dateTimePicker",
        skin="fluent_edit",
        ncSkin="",
        btnSkin="gallery_arrow_down",
        colorText="@color/fluent_text",
        cueColor="@color/fluent_text_secondary",
        padding="8dp,0,8dp,0",
    )
    update("header", itemSkin="_skin.sys.header", sortSkin="gallery_sort", **text)
    # The themed item skin overlays row colors; its neutral selected fill
    # requires regular text, not fluent_on_accent.
    update(
        "listctrl",
        skin="gallery_card",
        colorBkgnd="@color/fluent_card_bg",
        colorText="@color/fluent_text",
        colorSelText="@color/fluent_text",
        colorItemBkgnd="@color/fluent_card_bg",
        colorItemBkgnd2="@color/gallery_subtle",
        colorItemHotBkgnd="@color/fluent_fill_hover",
        colorItemSelBkgnd="@color/fluent_fill_pressed",
        itemSkin="_skin.sys.list.item",
        checkSkin="_skin.sys.checkbox",
        sbSkin="_skin.sys.scrollbar",
    )
    # Virtual views take item visuals from their local <template>, not from
    # listctrl/listbox attributes such as itemSkin or colorItemSelBkgnd.
    for tag in ("listview", "mclistview", "tileview"):
        update(
            tag,
            colorText="@color/fluent_text",
            colorBkgnd="@color/fluent_card_bg",
            sbSkin="_skin.sys.scrollbar",
        )
    update("scrollbar", skin="_skin.sys.scrollbar")
    update(
        "group",
        colorText="@color/fluent_text",
        colorLine1="@color/gallery_stroke",
        colorLine2="@color/gallery_stroke",
        round="4dp",
    )
    # SSplitWnd::OnPaint uses sepSkin, not skin, for splitter rectangles.
    # The solid 1x16 SVG stretches correctly in both orientations.
    for tag in ("splitwnd", "splitcol", "splitrow"):
        update(tag, sepSkin="gallery_separator", colorBkgnd="@color/fluent_card_bg")
    update("frame", skin="gallery_card", colorText="@color/fluent_text")
    update(
        "stackPage",
        colorBkgnd="@color/fluent_card_bg",
        colorText="@color/fluent_text",
    )
