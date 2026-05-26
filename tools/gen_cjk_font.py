#!/usr/bin/env python3
"""gen_cjk_font.py — Generate a CJK 12×12 glyph database for Bafang firmware.

Reads a WenQuanYi 9pt BDF bitmap font, extracts the requested Unicode
codepoints (default: CJK Unified Ideographs U+4E00–U+9FFF plus common
punctuation), and writes a binary image suitable for flashing to SPI Flash at
offset 0x010000 (CJK_FONT_BASE).

Output binary layout (see App/l10n/cjk_font.h for the canonical spec):

  [0x00]  Header      16 bytes
  [0x10]  Index       glyph_count × 4 bytes (sorted by codepoint ASC)
  [0x10 + glyph_count*4]  Bitmaps  glyph_count × 24 bytes

Requirements:
    pip install pillow   (only needed with --scale)

Usage:
    # Recommended: WenQuanYi 9pt BDF (native 12×12, no scaling needed)
    #   Source: olikraus/u8g2 tools/font/bdf/wenquanyi_9pt.bdf
    python3 tools/gen_cjk_font.py gen \\
        --bdf /path/to/wenquanyi_9pt.bdf \\
        --subset App/l10n/strings_zh.c \\
        --out tools/cjk_font.bin

    # Verify the output:
    python3 tools/gen_cjk_font.py verify tools/cjk_font.bin

Notes on WenQuanYi source:
  Use "wenquanyi_9pt.bdf" from olikraus/u8g2 (FONTBOUNDINGBOX 12×14).
  CJK glyphs in this file have BBX ≤ 12×12, so no scaling is required.
  DO NOT use wenquanyi_12pt.bdf (FONTBOUNDINGBOX 16×19): rescaling a bitmap
  font with LANCZOS introduces antialiasing artifacts.  Use --scale only as a
  last resort and prefer the 9pt file.
  The C glyph cache assumes 12×12 = CJK_GLYPH_BYTES=24 bytes; if you change
  the size you must update that constant and rebuild the firmware.
"""

import argparse
import struct
import sys
import zlib
from pathlib import Path
from typing import Dict, List, Tuple

# ---------------------------------------------------------------------------
# Constants (must match App/l10n/cjk_font.h)
# ---------------------------------------------------------------------------
MAGIC         = 0x464D  # "FM"
VERSION       = 1
HEADER_SZ     = 16
INDEX_ENTRY_SZ = 4
GLYPH_W       = 12
GLYPH_H       = 12
GLYPH_BYTES   = 24  # GLYPH_H * 2 bytes/row

# Default codepoint ranges to include.
# Adjust to save SPI Flash space; the firmware only needs glyphs it actually
# references in strings_zh.c, but including the full CJK block is simplest.
DEFAULT_RANGES: List[Tuple[int, int]] = [
    (0x4E00, 0x9FFF),   # CJK Unified Ideographs (20,902 chars, ~486 KB raw)
    (0x3000, 0x303F),   # CJK Symbols and Punctuation
    (0xFF00, 0xFFEF),   # Halfwidth and Fullwidth Forms
]

# ---------------------------------------------------------------------------
# BDF parsing (minimal — only what we need)
# ---------------------------------------------------------------------------

def parse_bdf(path: str, wanted: set, glyph_w: int, glyph_h: int,
              scale: bool = False) -> Dict[int, bytes]:
    """Return {codepoint: bitmap_bytes} for every codepoint in `wanted`.
    When scale=True, glyphs larger than glyph_w×glyph_h are downscaled
    via Pillow instead of being skipped.
    """
    glyphs: Dict[int, bytes] = {}
    cp = None
    in_bitmap = False
    rows: List[int] = []
    bbx_w = bbx_h = 0
    skipped = 0

    with open(path, "r", encoding="latin-1") as f:
        for line in f:
            line = line.rstrip()
            if line.startswith("ENCODING "):
                cp = int(line.split()[1])
            elif line.startswith("BBX "):
                parts = line.split()
                bbx_w, bbx_h = int(parts[1]), int(parts[2])
            elif line == "BITMAP":
                in_bitmap = True
                rows = []
            elif line == "ENDCHAR":
                if in_bitmap and cp is not None and cp in wanted:
                    bitmap = _encode_bitmap(rows, bbx_w, bbx_h, glyph_w, glyph_h,
                                           scale=scale)
                    if bitmap is not None:
                        glyphs[cp] = bitmap
                    else:
                        skipped += 1
                in_bitmap = False
                cp = None
                rows = []
            elif in_bitmap:
                try:
                    rows.append(int(line, 16))
                except ValueError:
                    pass

    if skipped:
        print(f"  (skipped {skipped} glyphs too large for {glyph_w}×{glyph_h}; "
              f"use --scale to downsize them)")
    return glyphs


def _bdf_rows_to_pixels(rows: List[int], bbx_w: int, bbx_h: int) -> List[List[int]]:
    """Decode BDF hex rows to a 2-D list of 0/1 pixel values [row][col]."""
    bdf_bytes_per_row = (bbx_w + 7) // 8
    total_bits = bdf_bytes_per_row * 8
    pixels = []
    for row_val in rows[:bbx_h]:
        row_pixels = []
        for col in range(bbx_w):
            bit = (row_val >> (total_bits - 1 - col)) & 1
            row_pixels.append(bit)
        pixels.append(row_pixels)
    return pixels


def _encode_bitmap(rows: List[int], bbx_w: int, bbx_h: int,
                   glyph_w: int, glyph_h: int,
                   scale: bool = False) -> bytes | None:
    """Convert a list of raw BDF hex rows into a normalised glyph_w×glyph_h
    bitmap, packed as glyph_h × 2 bytes (MSB = leftmost pixel).
    Returns None if the source is too large to fit and scale=False.
    When scale=True, uses Pillow to resize oversized glyphs.
    """
    too_large = bbx_w > glyph_w or bbx_h > glyph_h

    if too_large and not scale:
        return None  # glyph too large; skip rather than clip

    if too_large and scale:
        # Use Pillow to scale the source glyph down to target size.
        from PIL import Image
        pixels = _bdf_rows_to_pixels(rows, bbx_w, bbx_h)
        img = Image.new('L', (bbx_w, bbx_h), 0)
        for y, row_pixels in enumerate(pixels):
            for x, bit in enumerate(row_pixels):
                img.putpixel((x, y), 255 if bit else 0)
        img = img.resize((glyph_w, glyph_h), Image.LANCZOS)
        result = bytearray(glyph_h * 2)
        for row in range(glyph_h):
            word = 0
            for col in range(glyph_w):
                if img.getpixel((col, row)) >= 64:  # threshold ~25%
                    word |= 1 << (15 - col)
            result[row * 2]     = (word >> 8) & 0xFF
            result[row * 2 + 1] =  word       & 0xFF
        return bytes(result)

    # How many BDF bytes per row?
    bdf_bytes_per_row = (bbx_w + 7) // 8

    result = bytearray(glyph_h * 2)
    row_offset = glyph_h - bbx_h  # pad at top if shorter

    for i, row_val in enumerate(rows[:bbx_h]):
        dst_row = row_offset + i
        if dst_row < 0 or dst_row >= glyph_h:
            continue

        # BDF stores bits left-aligned within bytes.
        # Extract bbx_w bits and place left-aligned in a 16-bit word.
        # BDF row value is already left-aligned within bdf_bytes_per_row bytes.
        # We need the top `bbx_w` bits, shift to fit into 16-bit word.
        total_bits = bdf_bytes_per_row * 8
        pixel_word = row_val >> (total_bits - bbx_w)  # right-justified, bbx_w bits
        pixel_word = pixel_word << (glyph_w - bbx_w)   # left-justify in glyph_w bits
        pixel_word = pixel_word << (16 - glyph_w)       # left-justify in 16-bit word

        result[dst_row * 2]     = (pixel_word >> 8) & 0xFF
        result[dst_row * 2 + 1] =  pixel_word       & 0xFF

    return bytes(result)


# ---------------------------------------------------------------------------
# Binary packing
# ---------------------------------------------------------------------------

def build_image(glyphs: Dict[int, bytes]) -> bytes:
    """Pack glyphs into the binary image format."""
    if not glyphs:
        raise ValueError("No glyphs to pack")

    sorted_cps: List[int] = sorted(glyphs.keys())
    glyph_count = len(sorted_cps)
    bitmap_off = HEADER_SZ + glyph_count * INDEX_ENTRY_SZ

    # Index table
    index_data = bytearray()
    for cp in sorted_cps:
        index_data += struct.pack("<HH", cp, 0)

    # Bitmap data
    bitmap_data = bytearray()
    for cp in sorted_cps:
        bitmap_data += glyphs[cp]

    # CRC over (header[0..11] ++ index ++ bitmaps)
    # Build the first 12 bytes of the header for CRC input
    header_body = struct.pack("<HBBHHl",
                              MAGIC, VERSION, 0,
                              glyph_count, 0,
                              bitmap_off)
    crc_input = header_body + bytes(index_data) + bytes(bitmap_data)
    crc = zlib.crc32(crc_input) & 0xFFFFFFFF

    # Full 16-byte header
    header = struct.pack("<HBBHHlL",
                         MAGIC, VERSION, 0,
                         glyph_count, 0,
                         bitmap_off,
                         crc)
    assert len(header) == HEADER_SZ

    return bytes(header) + bytes(index_data) + bytes(bitmap_data)


# ---------------------------------------------------------------------------
# Verification
# ---------------------------------------------------------------------------

def verify_image(data: bytes) -> None:
    """Parse and validate a previously generated binary image."""
    if len(data) < HEADER_SZ:
        raise ValueError("File too small to be a valid font image")

    magic, version, _r, glyph_count, _r2, bitmap_off, stored_crc = \
        struct.unpack_from("<HBBHHlL", data, 0)

    print(f"Magic:       0x{magic:04X}  ({'OK' if magic == MAGIC else 'BAD, expected 0x' + hex(MAGIC)})")
    print(f"Version:     {version}  ({'OK' if version == VERSION else 'BAD'})")
    print(f"Glyphs:      {glyph_count}")
    print(f"Bitmap off:  0x{bitmap_off:04X}")
    print(f"Stored CRC:  0x{stored_crc:08X}")

    expected_bitmap_off = HEADER_SZ + glyph_count * INDEX_ENTRY_SZ
    if bitmap_off != expected_bitmap_off:
        raise ValueError(f"bitmap_off mismatch: got {bitmap_off}, expected {expected_bitmap_off}")

    expected_total = bitmap_off + glyph_count * GLYPH_BYTES
    if len(data) < expected_total:
        raise ValueError(f"File too small: need {expected_total}, got {len(data)}")

    crc_input = data[:12] + data[HEADER_SZ:]
    computed = zlib.crc32(crc_input) & 0xFFFFFFFF
    if computed != stored_crc:
        raise ValueError(f"CRC mismatch: stored 0x{stored_crc:08X}, computed 0x{computed:08X}")
    print(f"CRC check:   PASS (0x{computed:08X})")

    # Verify index is sorted
    for i in range(glyph_count - 1):
        cp_a = struct.unpack_from("<H", data, HEADER_SZ + i * INDEX_ENTRY_SZ)[0]
        cp_b = struct.unpack_from("<H", data, HEADER_SZ + (i + 1) * INDEX_ENTRY_SZ)[0]
        if cp_a >= cp_b:
            raise ValueError(f"Index not sorted at position {i}: U+{cp_a:04X} >= U+{cp_b:04X}")
    print(f"Index order: PASS (sorted)")
    print(f"Total size:  {len(data)} bytes ({len(data)/1024:.1f} KB)")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="cmd", required=True)

    gen = sub.add_parser("generate", aliases=["gen"],
                         help="Generate binary from a BDF font file")
    gen.add_argument("--bdf", required=True, metavar="FILE",
                     help="Input BDF font file (12×12)")
    gen.add_argument("--out", required=True, metavar="FILE",
                     help="Output binary file")
    gen.add_argument("--width",  type=int, default=GLYPH_W,
                     help=f"Glyph width in pixels (default: {GLYPH_W})")
    gen.add_argument("--height", type=int, default=GLYPH_H,
                     help=f"Glyph height in pixels (default: {GLYPH_H})")
    gen.add_argument("--range", metavar="START:END", action="append",
                     dest="ranges",
                     help="Additional codepoint range (hex, e.g. 4E00:9FFF). "
                          "Repeatable. Default: standard CJK + punctuation ranges.")
    gen.add_argument("--codepoints", metavar="FILE",
                     help="Text file listing hex codepoints (one per line) to include. "
                          "Merges with --range.")
    gen.add_argument("--subset", metavar="FILE", action="append",
                     help="UTF-8 source file: include only codepoints present in this "
                          "file (useful to minimise flash usage). Repeatable — "
                          "codepoints from ALL listed files are unioned together.")
    gen.add_argument("--scale", action="store_true",
                     help="Downscale glyphs larger than --width×--height using Pillow "
                          "(needed for WenQuanYi 12pt BDF which is 16×19 in u8g2).")

    ver = sub.add_parser("verify", help="Verify a generated binary file")
    ver.add_argument("file", metavar="FILE")

    dc = sub.add_parser("dump-c", help="Convert binary font to a C array (cjk_font_data.c)")
    dc.add_argument("file", metavar="FILE", help="Input binary font file")
    dc.add_argument("--sym", default="g_cjk_font_data",
                    help="C symbol name (default: g_cjk_font_data)")

    args = parser.parse_args()

    if args.cmd in ("generate", "gen"):
        # Build the wanted set
        ranges = DEFAULT_RANGES[:]
        if args.ranges:
            for r in args.ranges:
                a, b = r.split(":")
                ranges.append((int(a, 16), int(b, 16)))

        wanted: set = set()
        for start, end in ranges:
            wanted.update(range(start, end + 1))

        if args.codepoints:
            with open(args.codepoints, "r") as f:
                for line in f:
                    line = line.strip()
                    if line:
                        wanted.add(int(line, 16))

        if args.subset:
            subset_cps = set()
            for subset_file in args.subset:
                with open(subset_file, "r", encoding="utf-8") as f:
                    for ch in f.read():
                        cp = ord(ch)
                        if cp in wanted:
                            subset_cps.add(cp)
            wanted = subset_cps
            print(f"Subset mode: {len(wanted)} unique codepoints from {len(args.subset)} file(s)")

        print(f"Parsing BDF: {args.bdf} (target {len(wanted)} codepoints)…")
        if getattr(args, 'scale', False):
            print(f"  --scale enabled: oversized glyphs will be downscaled to "
                  f"{args.width}×{args.height} via Pillow")
        glyphs = parse_bdf(args.bdf, wanted, args.width, args.height,
                           scale=getattr(args, 'scale', False))
        print(f"Extracted {len(glyphs)} glyphs")

        if not glyphs:
            print("ERROR: No glyphs found. Check --bdf path and ranges.", file=sys.stderr)
            return 1

        image = build_image(glyphs)
        Path(args.out).write_bytes(image)
        print(f"Written {len(image)} bytes ({len(image)/1024:.1f} KB) → {args.out}")
        verify_image(image)

    elif args.cmd == "verify":
        data = Path(args.file).read_bytes()
        verify_image(data)

    elif args.cmd == "dump-c":
        data = Path(args.file).read_bytes()
        sym  = args.sym
        size = len(data)
        # Grab glyph_count from header so we can embed it in the comment
        import struct as _s
        _, _, _, glyph_count, _, bitmap_off, _ = _s.unpack_from("<HBBHHlL", data, 0)
        print(f"/* Auto-generated from {Path(args.file).name} — do not edit.")
        print(f" * Regenerate: python3 tools/gen_cjk_font.py gen --bdf <wenquanyi_9pt.bdf>")
        print(f" *             --subset App/l10n/strings_zh.c \\")
        print(f" *             --subset App/ui/menu_sub_values_zh.c \\")
        print(f" *             --out tools/cjk_font.bin")
        print(f" * then: python3 tools/gen_cjk_font.py dump-c tools/cjk_font.bin > App/l10n/cjk_font_data.c")
        print(f" * Format: FM font v1, {size} bytes, {glyph_count} glyphs, see App/l10n/cjk_font.h.")
        print(f" */")
        print(f"#include <stdint.h>")
        print(f"")
        print(f"const uint8_t {sym}[] = {{")
        for i in range(0, size, 12):
            chunk = data[i:i+12]
            line = ", ".join(f"0x{b:02x}" for b in chunk)
            if i + 12 < size:
                print(f"    {line},")
            else:
                print(f"    {line}")
        print(f"}};")
        print(f"const unsigned int {sym}_len = {size}u;")

    return 0


if __name__ == "__main__":
    sys.exit(main())
