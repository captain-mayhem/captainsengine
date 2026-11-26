"""Paints Torchdelve's 2D map: tiles and tokens.

The 2D map draws every field as one opaque square, furniture and found traps as one opaque
quad over their fields (turned to their wall), creatures as one opaque square on their field.
So everything here is a full tile with its own floor:
  textures/map/          floor.png (corridors), room1..22.png (the rooms' colours, as on the
                         3D floors), unknown.png (fields not seen yet), listed in ground.dat
  textures/heros/        <hero>.jpg: a portrait medallion with a gold rim
  textures/monsters/     <monster>.jpg: a portrait medallion with a dark red rim
  textures/furniture/    <code>.jpg: the piece seen from straight above, on flagstones,
                         W x D fields at 128 px per field (the back at the top)
  textures/overlays/     stairs, traps, seen from above
  textures/other/        items.jpg (the marker of items on a field), script.jpg
  textures/world/        startpos1.jpg (the scene names it for the start field)
The portraits and the views from above are rendered from the 3D models (modellib.render),
so the map shows the same figures and pieces as the 3D view.

Usage: python tools/make_map_art.py [--preview <png>]
"""

import argparse
import math
import os

from PIL import Image, ImageChops, ImageDraw, ImageFilter

import figures as F
import furniture as FU
import make_models as MM
import make_textures as MT
import modellib as M
import texlib as T

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
TEX = os.path.join(ROOT, "TDClient", "textures")
TILE = 128
TOKEN = 256
QUALITY = 92

FURNITURE_CODES = {"alembic_bench": ("ab", 3, 2), "bookcase": ("bc", 3, 1), "chest": ("ch", 1, 1),
                   "cupboard": ("cb", 3, 1), "fireplace": ("fp", 3, 1), "rack": ("ra", 3, 2),
                   "scrying_table": ("st", 3, 2), "table": ("ta", 3, 2), "throne": ("th", 1, 1),
                   "tomb": ("to", 3, 2), "weapons_rack": ("wr", 3, 1)}
OVERLAY_SIZES = {"stairs": (2, 2), "pittrap": (1, 1), "rocktrap": (1, 1), "speartrap": (1, 1)}


# ---------------- floor tiles ----------------

def room_tile(index, rgb):
    """A room's field: one flagstone in the room's colour, calm enough for the map."""
    stones = MT.flagstones(TILE, seed=500 + index, base=rgb, gap=3.0, grid=1)
    mortar = tuple(int(c * 0.35) for c in rgb)
    img = MT.stone_surface(TILE, stones, mortar, seed=500 + index, bevel=3.0, grain=0.2, cracks=0.25)
    return frame_line(img, rgb)


def corridor_tile():
    stones = MT.cobbles(TILE, seed=520, base=(96, 90, 82), count=3)
    img = MT.stone_surface(TILE, stones, (52, 48, 44), seed=520, bevel=3.0, grain=0.15, cracks=0.0)
    return frame_line(img, (100, 94, 86))


def unknown_tile():
    img = Image.new("RGB", (TILE, TILE), (20, 18, 17))
    return T.modulate(img, T.fbm(TILE, 530, ((4, 0.5), (12, 0.5))), 0.35)


def frame_line(img, rgb):
    """A faint darker line around the field, so the grid stays readable on the map."""
    d = ImageDraw.Draw(img)
    d.rectangle((0, 0, TILE - 1, TILE - 1), outline=tuple(int(c * 0.45) for c in rgb), width=2)
    return img


def floor_under(w_fields, d_fields):
    """Neutral flagstones for the footprint of furniture and traps (they cover the floor)."""
    tile = room_tile(99, (104, 100, 94))
    img = Image.new("RGB", (w_fields * TILE, d_fields * TILE))
    for i in range(w_fields):
        for j in range(d_fields):
            img.paste(tile, (i * TILE, j * TILE))
    return img


# ---------------- portraits ----------------

def medallion(model, skin, rim, rim_dark, glow):
    """A portrait (head and shoulders, from a little above and to the side) in a round frame."""
    lo, hi = model.bounds()
    H = hi[1]
    yaw, pitch = 0.45, 0.18
    # the frame: around the upper body in camera space
    top = M.rotate(M.rotate((0, H * 1.02, 0), (0, 1, 0), -yaw), (1, 0, 0), pitch)
    mid = M.rotate(M.rotate((0, H * 0.48, 0), (0, 1, 0), -yaw), (1, 0, 0), pitch)
    half = (top[1] - mid[1]) / 2 * 1.05
    cy = (top[1] + mid[1]) / 2
    portrait = M.render(model, skin, yaw, pitch, (TOKEN, TOKEN), frame=(-half, cy - half, half, cy + half),
                        light=(-0.6, 0.7, 0.8), ambient=0.38)
    s = TOKEN
    img = Image.new("RGB", (s, s), (18, 16, 15))
    # the medallion's background: a warm glow fading into the dark
    bg = Image.new("RGB", (s, s), (0, 0, 0))
    bd = ImageDraw.Draw(bg)
    for r in range(s // 2, 0, -2):
        k = r / (s / 2)
        col = tuple(int(g * (1 - k) ** 0.8 + 22 * k) for g in glow)
        bd.ellipse((s / 2 - r, s / 2 - r * 0.95 - 10, s / 2 + r, s / 2 + r * 0.95 - 10), fill=col)
    disc = Image.new("L", (s, s), 0)
    ImageDraw.Draw(disc).ellipse((14, 14, s - 14, s - 14), fill=255)
    img.paste(bg, (0, 0), disc)
    img.paste(portrait.convert("RGB"), (0, 0), ImageChops.multiply(portrait.getchannel("A"), disc))
    # the rim: a ring with a light upper left and a darker lower right
    ring = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    rd = ImageDraw.Draw(ring)
    rd.ellipse((6, 6, s - 6, s - 6), outline=rim_dark + (255,), width=14)
    rd.ellipse((8, 8, s - 8, s - 8), outline=rim + (255,), width=8)
    rd.arc((10, 10, s - 10, s - 10), 160, 290, fill=tuple(min(255, c + 60) for c in rim) + (255,), width=3)
    img.paste(ring.convert("RGB"), (0, 0), ring.getchannel("A"))
    return img


# ---------------- views from above ----------------

def from_above(model, skin, w_fields, d_fields):
    """The model seen from straight above over its footprint, on flagstones (back at the top)."""
    w, h = w_fields * TILE, d_fields * TILE
    hw, hd = w_fields * 4.0, d_fields * 4.0
    if d_fields == 1 and model.bounds()[1][1] > 7.0:
        # a tall piece at a wall: seen obliquely from the front, so its front shows, squeezed
        # into its footprint (the frame spans the footprint's front edge to the piece's top back)
        pitch = 1.05
        top_back = M.rotate((0, model.bounds()[1][1], -hd), (1, 0, 0), pitch)
        front = M.rotate((0, 0, hd), (1, 0, 0), pitch)
        frame = (-hw, front[1], hw, top_back[1])
    else:
        pitch = math.pi / 2
        frame = (-hw, -hd, hw, hd)
    view = M.render(model, skin, 0.0, pitch, (w, h), frame=frame, light=(-0.4, 0.5, 0.75), ambient=0.4)
    img = floor_under(w_fields, d_fields)
    # a soft shadow under the piece
    shadow = view.getchannel("A").filter(ImageFilter.GaussianBlur(6)).point(lambda v: v * 0.6)
    img.paste((10, 8, 8), (5, 7), shadow)
    img.paste(view.convert("RGB"), (0, 0), view.getchannel("A"))
    return img


def items_marker():
    """The marker of items lying on a field: a pouch with coins."""
    m = M.Model("pouch", FU.colors(FU.WOOD, FU.METAL, FU.MISC, {"pouch": (128, 86, 50)}))
    m.spheroid("pouch", (0, 1.3, 0), (1.6, 1.4, 1.5), segs=12, rings=9)
    m.tube("pouch", (0, 2.4, 0), (0, 3.0, 0), 0.6, 0.9, segs=10)
    m.torus("rope", (0, 2.45, 0), (0, 1, 0), 0.62, 0.12, segs=12, sides=4)
    for k, (x, z) in enumerate(((2.2, 0.8), (1.6, 2.0), (-1.8, 1.8), (2.6, -0.6))):
        m.tube("gold", (x, 0.0, z), (x, 0.18 + 0.06 * k, z), 0.6, 0.6, segs=12)
    skin = m.paint_skin(1)
    view = M.render(m, skin, 0.5, 0.75, (TILE, TILE), frame=(-4.2, -3.6, 4.2, 4.8), light=(-0.4, 0.8, 0.6))
    img = corridor_tile()
    img = T.apply_light(img, Image.new("L", img.size, 90))
    img.paste(view.convert("RGB"), (0, 0), view.getchannel("A"))
    return img


def save(img, *parts):
    path = os.path.join(TEX, *parts)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if path.endswith(".png"):
        img.save(path, optimize=True)
    else:
        img.convert("RGB").save(path, quality=QUALITY, optimize=True)
    print(f"wrote textures/{'/'.join(parts)} {img.size[0]}x{img.size[1]}")
    return img


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--preview", help="write a contact sheet here")
    args = parser.parse_args()
    made = []
    # the floor tiles and their list
    made.append(save(corridor_tile(), "map", "floor.png"))
    for i, rgb in enumerate(MT.ROOM_COLORS, 1):
        made.append(save(room_tile(i, rgb), "map", f"room{i}.png"))
    made.append(save(unknown_tile(), "map", "unknown.png"))
    names = ["floor.png"] + [f"room{i}.png" for i in range(1, 23)] + ["unknown.png"]
    with open(os.path.join(TEX, "map", "ground.dat"), "w", newline="\r\n") as f:
        f.write(f"number: {len(names)}\n" + "".join(f"{n} {i}\n" for i, n in enumerate(names)))
    # portraits
    for name, build in [("warrior", MM.warrior)] + list(F.HEROES.items()):
        model = build()
        made.append(save(medallion(model, model.paint_skin(1), (204, 162, 72), (110, 80, 30), (150, 110, 60)), "heros", name + ".jpg"))
    for name, build in F.MONSTERS.items():
        model = build()
        made.append(save(medallion(model, model.paint_skin(1), (150, 40, 34), (70, 16, 14), (110, 60, 50)), "monsters", name + ".jpg"))
    # furniture and overlays from above
    for name, (code, w, d) in FURNITURE_CODES.items():
        model = FU.FURNITURE[name]()
        made.append(save(from_above(model, model.paint_skin(1), w, d), "furniture", code + ".jpg"))
    for name, (w, d) in OVERLAY_SIZES.items():
        model = FU.OVERLAYS[name]()
        made.append(save(from_above(model, model.paint_skin(1), w, d), "overlays", name + ".jpg"))
    # the rest
    made.append(save(items_marker(), "other", "items.jpg"))
    script = FU.OTHER["script"]()
    made.append(save(from_above(script, script.paint_skin(1), 1, 1), "other", "script.jpg"))
    start = F.WORLD["startpos"]()
    made.append(save(from_above(start, start.paint_skin(1), 1, 1), "world", "startpos1.jpg"))
    if args.preview:
        cell = 160
        cols = 10
        rows = (len(made) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * cell, rows * cell), (0, 0, 0))
        for k, img in enumerate(made):
            th = img.convert("RGB").copy()
            th.thumbnail((cell - 6, cell - 6))
            sheet.paste(th, ((k % cols) * cell + 3, (k // cols) * cell + 3))
        sheet.save(args.preview)


if __name__ == "__main__":
    main()
