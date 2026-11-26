"""Renders Torchdelve's start screen: art/start.svg -> TDClient/textures/other/start.jpg.

The SVG is designed at 1920x1080, the client's virtual screen; the client
stretches the picture over the screen, so it's saved at that size.

The renderer is resvg (https://github.com/linebender/resvg; Windows:
winget install linebender.resvg, Linux: cargo install resvg or the release
binary), found on the PATH, in the winget folders or through the RESVG
variable. Fonts come from art/fonts only (no system fonts), so every machine
renders the same. Needs Pillow.

Usage: python tools/make_start_screen.py [--png <file>]
       --png also keeps the rendering as PNG (to look at it).
"""

import argparse
import glob
import os
import shutil
import subprocess
import sys
import tempfile

from PIL import Image

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
SVG = os.path.join(ROOT, "art", "start.svg")
FONTS = os.path.join(ROOT, "art", "fonts")
OUT = os.path.join(ROOT, "TDClient", "textures", "other", "start.jpg")
WIDTH, HEIGHT = 1920, 1080


def find_resvg():
    exe = os.environ.get("RESVG") or shutil.which("resvg")
    if exe:
        return exe
    local = os.environ.get("LOCALAPPDATA", "")
    for pattern in ("Microsoft/WinGet/Links/resvg.exe", "Microsoft/WinGet/Packages/linebender.resvg_*/resvg.exe"):
        found = glob.glob(os.path.join(local, pattern))
        if found:
            return found[0]
    sys.exit("make_start_screen: resvg not found (install it or set RESVG)")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--png", help="also save the rendering as PNG here")
    args = parser.parse_args()

    with tempfile.TemporaryDirectory() as tmp:
        png = os.path.join(tmp, "start.png")
        cmd = [find_resvg(), "--skip-system-fonts", "--use-fonts-dir", FONTS,
               "-w", str(WIDTH), "-h", str(HEIGHT), SVG, png]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0 or not os.path.isfile(png):
            sys.exit(f"make_start_screen: resvg failed:\n{result.stdout}{result.stderr}")
        # resvg reports what it skipped (unsupported features, missing fonts) on stderr
        if result.stderr.strip():
            print(result.stderr.strip(), file=sys.stderr)
        image = Image.open(png).convert("RGB")
        if args.png:
            image.save(args.png)
        image.save(OUT, quality=92, optimize=True)
    print(f"wrote {os.path.relpath(OUT, ROOT)}")


if __name__ == "__main__":
    main()
