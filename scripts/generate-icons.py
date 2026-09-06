#!/usr/bin/env python3
"""Regenerate the desktop app and installer icons from the 512px app mark.

Usage: ./scripts/generate-icons.py [source-png]

Writes:
  src/wisdom-chess/ui/qml/images/wisdom-chess.ico   (16..256 px, Windows exe + installer)
  src/wisdom-chess/ui/qml/images/wisdom-chess.icns  (16..1024 px, macOS bundle + installer)
  installer/icons/wisdom-chess-256.png              (installer window icon, Linux .desktop)
  installer/icons/wisdom-chess-128.png              (installer logo)

Only needs Pillow (pip install pillow), so it runs the same on Linux, macOS
and Windows. Commit the generated files; the build does not run this.
"""

import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
IMAGES = ROOT / "src/wisdom-chess/ui/qml/images"
INSTALLER_ICONS = ROOT / "installer/icons"
DEFAULT_SOURCE = ROOT / "src/wisdom-chess/ui/qml/wasm/android-chrome-512x512.png"

ICO_SIZES = [16, 24, 32, 48, 64, 128, 256]


def scaled(source, size):
    return source.resize((size, size), Image.LANCZOS)


def main():
    source_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_SOURCE
    source = Image.open(source_path).convert("RGBA")
    if source.width != source.height or source.width < 512:
        sys.exit(f"{source_path}: need a square image of at least 512px, got {source.size}")

    INSTALLER_ICONS.mkdir(parents=True, exist_ok=True)

    ico = IMAGES / "wisdom-chess.ico"
    scaled(source, 256).save(ico, format="ICO", sizes=[(s, s) for s in ICO_SIZES])

    # Pillow derives the icns entries (16..1024 at 1x/2x) from the image it is
    # given; feed it a 1024px image so the retina 512@2x entry is not upscaled
    # from a smaller one.
    icns = IMAGES / "wisdom-chess.icns"
    scaled(source, 1024).save(icns, format="ICNS")

    for size in (256, 128):
        scaled(source, size).save(INSTALLER_ICONS / f"wisdom-chess-{size}.png", format="PNG")

    for path in (ico, icns, *sorted(INSTALLER_ICONS.glob("*.png"))):
        print(f"wrote {path.relative_to(ROOT)} ({path.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
