"""Paints Torchdelve's dungeon textures.

Writes into TDClient/textures/world:
  floorWall.jpg   the walls (1024 px for 8 x 8 units of wall: big hewn blocks)
  ceil2.jpg       the ceiling, once per field (dark stone with a timber beam)
  floor.jpg       the corridors' floor, once per field (worn cobbles)
  room1..22.jpg   the rooms' floors, once per field (flagstones, one colour per
                  room, so the rooms stay apart on the 2D map too)
  door.jpg        the door leaf (planks with iron bands)

Every texture is painted procedurally (Pillow only) with a fixed seed, so a
run gives the same pictures again. The style: dark painterly stone, warm
torchlight comes from the game's lighting.

Usage: python tools/make_textures.py [--only <name>...] [--preview <png>]
       --preview writes a contact sheet of everything it made.
"""

import argparse
import math
import os
import random

from PIL import Image, ImageChops, ImageDraw, ImageFilter

import texlib as T

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
WORLD = os.path.join(ROOT, "TDClient", "textures", "world")
QUALITY = 90

# the rooms' stone colours: muted, but different enough to tell the rooms
# apart on the 2D map (a field is only ~40 px there)
ROOM_COLORS = [
    (92, 96, 104),    # 1 slate
    (150, 122, 86),   # 2 sandstone
    (128, 70, 58),    # 3 rust
    (84, 104, 70),    # 4 moss
    (156, 128, 60),   # 5 ochre
    (98, 76, 104),    # 6 plum
    (66, 104, 106),   # 7 teal
    (70, 68, 66),     # 8 charcoal
    (118, 88, 64),    # 9 umber
    (168, 156, 132),  # 10 bone
    (140, 84, 66),    # 11 brick
    (110, 112, 70),   # 12 olive
    (150, 108, 108),  # 13 dusty rose
    (88, 108, 132),   # 14 steel blue
    (170, 120, 64),   # 15 amber
    (86, 122, 104),   # 16 verdigris
    (112, 58, 70),    # 17 burgundy
    (140, 130, 96),   # 18 khaki
    (120, 116, 134),  # 19 lavender grey
    (150, 96, 70),    # 20 copper
    (64, 88, 66),     # 21 forest
    (128, 126, 120),  # 22 ash
]


def stone_surface(size, stones, mortar_rgb, seed, bevel=6.0, grain=0.55, cracks=0.35, wear=None):
    """Paints stones (polygons with colours) on mortar: colour, grain, bevelled edges, cracks.

    stones: list of (points, rgb). Returns an RGB image.
    """
    rnd = random.Random(seed)
    w = h = size
    # the colours
    color = T.draw_wrapped((w, h), [(p, c) for p, c in stones], "RGB", mortar_rgb)
    # the height: stones up, mortar down; blurred for rounded edges
    mask = T.draw_wrapped((w, h), [(p, 255) for p, _ in stones], "L", 0)
    height = T.blur(mask, bevel)
    # surface relief inside the stones
    relief = T.fbm(size, seed + 7, ((6, 0.35), (16, 0.25), (40, 0.2), (96, 0.12), (200, 0.08)))
    height = ImageChops.add(height.point(lambda v: v * 0.75), relief.point(lambda v: v * 0.25))
    light = T.shade(height, strength=2.6, dx=2, dy=2)
    light = T.blur(light, 0.6)
    # grain: fine and coarse noise over everything
    noise = T.fbm(size, seed + 13)
    fine = T.fbm(size, seed + 17, ((64, 0.4), (128, 0.35), (256, 0.25)))
    color = T.modulate(color, noise, grain)
    color = T.modulate(color, fine, grain * 0.5)
    # cracks: thin dark lines across some stones
    d = ImageDraw.Draw(color)
    for points, _ in stones:
        if rnd.random() > cracks:
            continue
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        x0, x1, y0, y1 = min(xs), max(xs), min(ys), max(ys)
        x, y = rnd.uniform(x0, x1), rnd.uniform(y0, y1)
        ang = rnd.uniform(0, math.tau)
        segs = []
        for _ in range(rnd.randint(3, 7)):
            ang += rnd.uniform(-0.7, 0.7)
            step = rnd.uniform(6, 16) * size / 512
            nx, ny = x + math.cos(ang) * step, y + math.sin(ang) * step
            if not (x0 < nx < x1 and y0 < ny < y1):
                break
            segs.append(((x, y), (nx, ny)))
            x, y = nx, ny
        for a, b in segs:
            d.line([a, b], fill=tuple(int(c * 0.45) for c in mortar_rgb), width=max(1, size // 512))
    color = T.apply_light(color, light)
    # dirt in the mortar: darker where the stones are low
    occl = T.blur(mask, bevel * 2.5).point(lambda v: 150 + v * 105 // 255)
    color = ImageChops.multiply(color, Image.merge("RGB", (occl, occl, occl)))
    if wear:
        # worn, lighter patches (feet, centuries)
        patch = T.value_noise(size, wear, seed + 23).point(lambda v: max(0, v - 150) * 2)
        patch = T.blur(patch, size / 64)
        lighter = color.point(lambda v: min(255, int(v * 1.25)))
        color = Image.composite(lighter, color, patch)
    return T.desaturate(color, 0.2)


def _subdivide(points, step):
    """Adds points along a polygon's edges, about every step pixels (for wobbling)."""
    out = []
    n = len(points)
    for k in range(n):
        (x0, y0), (x1, y1) = points[k], points[(k + 1) % n]
        parts = max(1, int(math.hypot(x1 - x0, y1 - y0) / step))
        for s in range(parts):
            out.append((x0 + (x1 - x0) * s / parts, y0 + (y1 - y0) * s / parts))
    return out


def ashlar(size, rows, mean_len, seed, base, gap):
    """Rows of hewn blocks (walls): each row splits the full width into blocks."""
    rnd = random.Random(seed)
    stones = []
    row_h = size / rows
    for r in range(rows):
        y0 = r * row_h
        # block lengths that fill the row exactly (it wraps around)
        lengths = []
        while sum(lengths) < size * 0.98:
            lengths.append(mean_len * rnd.uniform(0.7, 1.35))
        lengths = [ln * size / sum(lengths) for ln in lengths]
        x = rnd.uniform(0, size)
        for ln in lengths:
            pts = T.rounded_rect(x + gap, y0 + gap, x + ln - gap, y0 + row_h - gap, gap * 1.2, steps=3)
            # chipped edges: more points along the long sides
            stones.append((T.wobble(_subdivide(pts, gap * 4), rnd, gap * 0.45), T.jitter_color(base, rnd, 0.1)))
            x += ln
    return stones


def flagstones(size, seed, base, gap, grid=3):
    """Flagstones on a grid, some merged into longer slabs (floors)."""
    rnd = random.Random(seed)
    cell = size / grid
    used = set()
    stones = []
    for j in range(grid):
        for i in range(grid):
            if (i, j) in used:
                continue
            wcells, hcells = 1, 1
            if i + 1 < grid and (i + 1, j) not in used and rnd.random() < 0.35:
                wcells = 2
            elif j + 1 < grid and (i, j + 1) not in used and rnd.random() < 0.35:
                hcells = 2
            for a in range(wcells):
                for b in range(hcells):
                    used.add((i + a, j + b))
            x0, y0 = i * cell, j * cell
            x1, y1 = x0 + wcells * cell, y0 + hcells * cell
            pts = T.rounded_rect(x0 + gap, y0 + gap, x1 - gap, y1 - gap, gap * 1.5, steps=3)
            stones.append((T.wobble(_subdivide(pts, gap * 4), rnd, gap * 0.5), T.jitter_color(base, rnd, 0.08)))
    return stones


def cobbles(size, seed, base, count):
    """Small rounded stones in a jittered grid (corridors)."""
    rnd = random.Random(seed)
    cell = size / count
    stones = []
    for j in range(count):
        off = (j % 2) * cell / 2
        for i in range(count):
            cx = i * cell + off + rnd.uniform(-cell * 0.12, cell * 0.12)
            cy = j * cell + cell / 2 + rnd.uniform(-cell * 0.12, cell * 0.12)
            rx = cell * rnd.uniform(0.46, 0.54)
            ry = cell * rnd.uniform(0.44, 0.52)
            rot = rnd.uniform(0, math.pi)
            pts = []
            for k in range(14):
                a = math.tau * k / 14
                r = 1 + rnd.uniform(-0.08, 0.08)
                x, y = math.cos(a) * rx * r, math.sin(a) * ry * r
                pts.append((cx + x * math.cos(rot) - y * math.sin(rot), cy + x * math.sin(rot) + y * math.cos(rot)))
            stones.append((pts, T.jitter_color(base, rnd, 0.14)))
    return stones


def make_wall():
    size = 1024
    stones = ashlar(size, rows=4, mean_len=380, seed=11, base=(104, 92, 80), gap=7)
    return stone_surface(size, stones, (28, 24, 21), seed=11, bevel=7, grain=0.6, cracks=0.4)


def make_floor():
    size = 512
    stones = cobbles(size, seed=21, base=(112, 104, 94), count=7)
    return stone_surface(size, stones, (30, 27, 24), seed=21, bevel=4, grain=0.5, cracks=0.1, wear=6)


def make_room(index, rgb):
    size = 512
    stones = flagstones(size, seed=100 + index, base=rgb, gap=5, grid=3 if index % 3 else 2)
    mortar = tuple(int(c * 0.3) for c in rgb)
    return stone_surface(size, stones, mortar, seed=100 + index, bevel=5, grain=0.5, cracks=0.3, wear=5)


def make_ceiling():
    """Dark stone with a timber beam across the field (once per field, so the beams line up)."""
    size = 512
    stones = ashlar(size, rows=4, mean_len=200, seed=31, base=(58, 52, 48), gap=4)
    img = stone_surface(size, stones, (18, 16, 14), seed=31, bevel=4, grain=0.5, cracks=0.2)
    # the beam: wood grain from stretched noise
    beam_h = size // 5
    y0 = (size - beam_h) // 2
    grain = T.value_noise(size, 6, 33).resize((size, beam_h))
    streaks = T.value_noise(size, 64, 34).resize((size, beam_h), Image.BICUBIC)
    wood = Image.new("RGB", (size, beam_h), (78, 50, 30))
    wood = T.modulate(wood, grain, 0.4)
    wood = T.modulate(wood, streaks, 0.5)
    # rounded edges of the beam: darker at top and bottom
    edge = Image.new("L", (1, beam_h))
    edge.putdata([int(128 * (0.55 + 0.45 * math.sin(math.pi * (k + 0.5) / beam_h))) for k in range(beam_h)])
    wood = T.apply_light(wood, edge.resize((size, beam_h)))
    # its shadow on the stone
    shadow = Image.new("L", (size, size), 255)
    ImageDraw.Draw(shadow).rectangle((0, y0 - 6, size, y0 + beam_h + 14), fill=60)
    shadow = T.blur(shadow, 10)
    img = ImageChops.multiply(img, Image.merge("RGB", (shadow, shadow, shadow)))
    img.paste(wood, (0, y0))
    return img


def make_door():
    """Vertical planks with two iron bands, studs and a ring (512 x 1024)."""
    w, h = 512, 1024
    rnd = random.Random(41)
    planks = 5
    pw = w / planks
    img = Image.new("RGB", (w, h))
    for k in range(planks):
        base = T.jitter_color((92, 60, 34), rnd, 0.1)
        plank = Image.new("RGB", (int(pw) + 1, h), base)
        grain = T.value_noise(256, 48, 50 + k).resize((int(pw) + 1, h), Image.BICUBIC)
        streak = T.value_noise(256, 6, 60 + k).resize((int(pw) + 1, h), Image.BICUBIC)
        plank = T.modulate(plank, grain.transpose(Image.Transpose.ROTATE_90).resize((int(pw) + 1, h)), 0.35)
        plank = T.modulate(plank, streak, 0.35)
        # darker plank edges
        edge = Image.new("L", (int(pw) + 1, 1))
        edge.putdata([int(128 * (0.6 + 0.4 * math.sin(math.pi * (i + 0.5) / (pw + 1)))) for i in range(int(pw) + 1)])
        plank = T.apply_light(plank, edge.resize((int(pw) + 1, h)))
        img.paste(plank, (int(k * pw), 0))
    d = ImageDraw.Draw(img)
    # iron bands with studs
    for by in (h * 0.2, h * 0.75):
        y0, y1 = int(by - 28), int(by + 28)
        d.rectangle((0, y0, w, y1), fill=(40, 38, 38))
        d.rectangle((0, y0, w, y0 + 5), fill=(70, 66, 64))
        d.rectangle((0, y1 - 4, w, y1), fill=(22, 20, 20))
        for sx in range(30, w, 70):
            d.ellipse((sx - 9, by - 9, sx + 9, by + 9), fill=(58, 54, 52))
            d.ellipse((sx - 5, by - 7, sx + 2, by - 1), fill=(96, 90, 86))
    # the ring handle
    cx, cy = int(w * 0.78), int(h * 0.5)
    d.ellipse((cx - 22, cy - 22, cx + 22, cy + 22), fill=(36, 34, 34))
    d.ellipse((cx - 40, cy, cx + 40, cy + 80), outline=(70, 64, 58), width=10)
    d.arc((cx - 40, cy, cx + 40, cy + 80), 200, 300, fill=(120, 110, 96), width=4)
    # rust and dirt
    noise = T.fbm(256, 70).resize((w, h), Image.BICUBIC)
    img = T.modulate(img, noise, 0.35)
    # a darker frame towards the edges
    vign = Image.new("L", (w, h), 0)
    ImageDraw.Draw(vign).rectangle((14, 14, w - 14, h - 14), fill=255)
    vign = vign.filter(ImageFilter.GaussianBlur(18)).point(lambda v: 70 + v * 58 // 255)
    return T.apply_light(img, vign)


JOBS = {
    "wall": ("floorWall.jpg", make_wall),
    "ceiling": ("ceil2.jpg", make_ceiling),
    "floor": ("floor.jpg", make_floor),
    "door": ("door.jpg", make_door),
}
for _i, _rgb in enumerate(ROOM_COLORS, 1):
    JOBS[f"room{_i}"] = (f"room{_i}.jpg", (lambda i=_i, c=_rgb: make_room(i, c)))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--only", nargs="*", help="names to make (" + ", ".join(JOBS) + ")")
    parser.add_argument("--preview", help="write a contact sheet here")
    args = parser.parse_args()
    names = args.only or list(JOBS)
    made = []
    for name in names:
        filename, make = JOBS[name]
        img = make()
        img.save(os.path.join(WORLD, filename), quality=QUALITY, optimize=True)
        made.append((name, img))
        print(f"wrote textures/world/{filename} {img.size[0]}x{img.size[1]}")
    if args.preview:
        cell = 200
        cols = 6
        rows = (len(made) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * cell, rows * cell), (0, 0, 0))
        for k, (name, img) in enumerate(made):
            th = img.copy()
            th.thumbnail((cell - 6, cell - 6))
            sheet.paste(th, ((k % cols) * cell + 3, (k // cols) * cell + 3))
        sheet.save(args.preview)


if __name__ == "__main__":
    main()
