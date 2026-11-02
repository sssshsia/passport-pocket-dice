"""Generate or verify the fixed Pocket Dice 16px LVGL font."""

import argparse
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FONT_DIR = ROOT / "assets/fonts"
SOURCE_HASH = "faa6c9df652116dde789d351359f3d7e5d2285a2b2a1f04a2d7244df706d5ea9"


def codepoints():
    required = set(range(32, 127))
    for source in (ROOT / "main").glob("dice*.[ch]"):
        required.update(ord(character) for character in source.read_text("utf-8") if ord(character) > 127)
    return sorted(required)


def c_array(name, ctype, values):
    rows = [", ".join(str(value) for value in values[start:start + 16])
            for start in range(0, len(values), 16)]
    return f"static const {ctype} {name}[] = {{\n    " + ",\n    ".join(rows) + "\n};\n"


def generate():
    from PIL import ImageFont
    from fontTools.ttLib import TTFont

    source = FONT_DIR / "NotoSansSC-Regular.otf"
    assert hashlib.sha256(source.read_bytes()).hexdigest() == SOURCE_HASH
    inventory = codepoints()
    available = TTFont(source).getBestCmap()
    assert all(point in available for point in inventory), "Source font lacks required characters"
    font = ImageFont.truetype(str(source), 16)
    bitmap, descriptors, evidence = [], ["    {0}"], []
    for point in inventory:
        mask, offset = font.getmask2(chr(point), mode="L", anchor="ls")
        width, height = mask.size
        pixels = [value >> 4 for value in bytes(mask)]
        start = len(bitmap)
        padded = pixels + ([0] if len(pixels) % 2 else [])
        bitmap.extend((padded[index] << 4) | padded[index + 1] for index in range(0, len(padded), 2))
        advance = round(font.getlength(chr(point)) * 16)
        descriptors.append(
            f"    {{.bitmap_index={start}, .adv_w={advance}, .box_w={width}, .box_h={height}, "
            f".ofs_x={offset[0]}, .ofs_y={-offset[1] - height}}}")
        evidence.append({"point": point, "start": start, "width": width, "height": height,
                         "pixels_sha256": hashlib.sha256(bytes(pixels)).hexdigest()})
    extras = inventory[95:]
    generated = '#include "lvgl.h"\n\n'
    generated += c_array("glyph_bitmap", "uint8_t", bitmap)
    generated += "static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {\n"
    generated += ",\n".join(descriptors) + "\n};\n"
    generated += c_array("unicode_list", "uint16_t", [point - extras[0] for point in extras])
    generated += (
        "static const lv_font_fmt_txt_cmap_t cmaps[] = {\n"
        "    {.range_start=32, .range_length=95, .glyph_id_start=1, "
        ".type=LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY},\n"
        f"    {{.range_start={extras[0]}, .range_length={extras[-1] - extras[0] + 1}, "
        f".glyph_id_start=96, .unicode_list=unicode_list, .list_length={len(extras)}, "
        ".type=LV_FONT_FMT_TXT_CMAP_SPARSE_TINY}\n};\n"
        "static const lv_font_fmt_txt_dsc_t font_dsc = {\n"
        "    .glyph_bitmap=glyph_bitmap, .glyph_dsc=glyph_dsc, .cmaps=cmaps,\n"
        "    .cmap_num=2, .bpp=4, .bitmap_format=0\n};\n"
        "const lv_font_t dice_font_16 = {\n"
        "    .get_glyph_dsc=lv_font_get_glyph_dsc_fmt_txt,\n"
        "    .get_glyph_bitmap=lv_font_get_bitmap_fmt_txt,\n"
        "    .line_height=20, .base_line=4, .dsc=&font_dsc,\n"
        "    .underline_position=-2, .underline_thickness=1\n};\n"
    )
    generated += c_array("dice_font_codepoints", "uint32_t", inventory).replace("static const", "const")
    generated += f"const unsigned dice_font_codepoint_count = {len(inventory)};\n"
    (FONT_DIR / "dice_font_16.c").write_text(generated, encoding="utf-8", newline="\n")
    manifest = {"source_sha256": SOURCE_HASH, "size": 16, "bpp": 4, "line_height": 20,
                "converter": "generate_dice_font.py / Pillow 11.3.0 / fonttools 4.59.0",
                "font_sha256": hashlib.sha256(generated.encode()).hexdigest(), "glyphs": evidence}
    (FONT_DIR / "dice_font_16.json").write_text(json.dumps(manifest, indent=2) + "\n", "utf-8")


def check():
    manifest = json.loads((FONT_DIR / "dice_font_16.json").read_text("utf-8"))
    source = (FONT_DIR / "dice_font_16.c").read_text("utf-8")
    assert hashlib.sha256(source.encode()).hexdigest() == manifest["font_sha256"], "Font file changed"
    assert hashlib.sha256((FONT_DIR / "NotoSansSC-Regular.otf").read_bytes()).hexdigest() == SOURCE_HASH
    assert codepoints() == [glyph["point"] for glyph in manifest["glyphs"]], "Regenerate font for UI text"
    packed = re.search(r"glyph_bitmap\[\] = \{(.*?)\};", source, re.S).group(1)
    bitmap = [int(value) for value in re.findall(r"\d+", packed)]
    for glyph in manifest["glyphs"]:
        count = glyph["width"] * glyph["height"]
        pixels = [(bitmap[glyph["start"] + index // 2] >> (4 if index % 2 == 0 else 0)) & 15
                  for index in range(count)]
        assert hashlib.sha256(bytes(pixels)).hexdigest() == glyph["pixels_sha256"]
        if glyph["point"] > 32:
            assert any(pixels), f"Empty glyph U+{glyph['point']:04X}"
    print(f"Dice font: PASS ({len(manifest['glyphs'])} codepoints, 16px, 4bpp, pixel readback)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    options = parser.parse_args()
    if not options.check:
        generate()
    check()
