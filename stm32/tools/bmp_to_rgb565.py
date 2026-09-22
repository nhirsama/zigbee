#!/usr/bin/env python3
"""Convert uncompressed 24/32-bit BMP to RGB565 C array for ST7735 LCD.

Usage:
  ./bmp_to_rgb565.py input.bmp image_data > image_data.h
"""
import argparse
import struct
import sys
from pathlib import Path


def read_bmp(path: Path):
    data = path.read_bytes()
    if data[:2] != b"BM":
        raise SystemExit("only BMP input is supported")
    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40:
        raise SystemExit("unsupported BMP DIB header")
    width = struct.unpack_from("<i", data, 18)[0]
    height_raw = struct.unpack_from("<i", data, 22)[0]
    planes = struct.unpack_from("<H", data, 26)[0]
    bpp = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    if planes != 1 or compression != 0 or bpp not in (24, 32):
        raise SystemExit("only uncompressed 24/32-bit BMP is supported")
    top_down = height_raw < 0
    height = abs(height_raw)
    row_stride = ((width * bpp + 31) // 32) * 4
    pixels = []
    for y in range(height):
        src_y = y if top_down else height - 1 - y
        row = pixel_offset + src_y * row_stride
        for x in range(width):
            off = row + x * (bpp // 8)
            b, g, r = data[off], data[off + 1], data[off + 2]
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            pixels.append(rgb565)
    return width, height, pixels


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bmp", type=Path)
    ap.add_argument("name")
    ap.add_argument("--columns", type=int, default=12)
    args = ap.parse_args()
    w, h, pixels = read_bmp(args.bmp)
    guard = args.name.upper() + "_H"
    print(f"#ifndef {guard}")
    print(f"#define {guard}")
    print("#include <stdint.h>")
    print(f"#define {args.name.upper()}_WIDTH {w}")
    print(f"#define {args.name.upper()}_HEIGHT {h}")
    print(f"const uint16_t {args.name}[{len(pixels)}] = {{")
    for i in range(0, len(pixels), args.columns):
        chunk = pixels[i:i + args.columns]
        print("    " + ", ".join(f"0x{v:04X}" for v in chunk) + ("," if i + args.columns < len(pixels) else ""))
    print("};")
    print(f"#endif /* {guard} */")


if __name__ == "__main__":
    main()
