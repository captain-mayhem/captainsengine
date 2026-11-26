"""Torchdelve's furniture, floor overlays (stairs, traps), the triggered traps' models and
the script marker, built with modellib.

The compiler (TDEditor/compiler.cpp, calcTrafo) centres a piece on its fields and turns it
to the wall it stands at; in its own frame a piece is W fields wide along x and D fields
deep along z (a field is 8), its back at -z and its front at +z, standing on y = 0.
The walls are 16 high.
"""

import math
import random

import modellib as M
from modellib import add, mul

WOOD = {"wood": (112, 74, 42), "wood_dark": (72, 46, 26), "wood_light": (140, 98, 58)}
METAL = {"iron": (62, 60, 60), "iron_light": (110, 106, 102), "gold": (204, 162, 72), "brass": (176, 130, 60)}
STONE = {"stone": (118, 110, 100), "stone_dark": (78, 72, 66), "stone_light": (150, 142, 128)}
MISC = {"red": (126, 34, 30), "cloth": (60, 40, 70), "glass": (150, 200, 190), "potion_r": (200, 50, 40),
        "potion_g": (70, 190, 90), "potion_b": (70, 110, 210), "wax": (226, 214, 180), "flame": (255, 210, 110),
        "ember": (240, 110, 40), "black": (14, 12, 12), "bone": (214, 204, 174), "paper": (222, 206, 164),
        "rope": (170, 140, 90), "book1": (120, 40, 34), "book2": (44, 70, 46), "book3": (52, 56, 100),
        "book4": (120, 96, 50), "book5": (80, 50, 70)}


def colors(*groups):
    out = {}
    for g in groups:
        out.update(g)
    return out


def legs(m, swatch, w, d, h, inset, thick):
    """Four square legs under a top at height h."""
    for sx in (-1, 1):
        for sz in (-1, 1):
            m.box(swatch, (sx * (w / 2 - inset), h / 2, sz * (d / 2 - inset)), (thick, h, thick))


def candle(m, pos, height=0.9):
    m.tube("wax", pos, add(pos, (0, height, 0)), 0.13, 0.12, segs=7)
    m.spheroid("flame", add(pos, (0, height + 0.18, 0)), (0.07, 0.17, 0.07), segs=6, rings=4)


def flask(m, pos, swatch, r=0.45, neck=0.5):
    m.spheroid(swatch, add(pos, (0, r, 0)), (r, r, r), segs=10, rings=7)
    m.tube("glass", add(pos, (0, 1.7 * r, 0)), add(pos, (0, 1.7 * r + neck, 0)), 0.12, 0.1, segs=6)


def book_row(m, rnd, x0, x1, y, z_back, depth, hmin, hmax, lean_last=True):
    """Books standing on a shelf from x0 to x1."""
    x = x0
    names = ["book1", "book2", "book3", "book4", "book5"]
    while x < x1 - 0.3:
        t = rnd.uniform(0.28, 0.55)
        if x + t > x1:
            break
        h = rnd.uniform(hmin, hmax)
        if rnd.random() < 0.08:
            x += t  # a gap
            continue
        m.box(rnd.choice(names), (x + t / 2, y + h / 2, z_back + depth / 2), (t * 0.95, h, depth * rnd.uniform(0.8, 1.0)))
        x += t


# ---------------- furniture ----------------

def table():
    """A long wooden table (3 x 2) with a tankard, a plate and a candle."""
    m = M.Model("table", colors(WOOD, METAL, MISC))
    w, d, h = 20.0, 11.0, 4.6
    m.box("wood", (0, h - 0.3, 0), (w, 0.6, d))
    m.box("wood_dark", (0, h - 0.75, 0), (w - 1.2, 0.3, d - 1.2))
    legs(m, "wood_dark", w, d, h - 0.6, 1.0, 0.9)
    m.box("wood_dark", (0, 1.0, 0), (w - 2.4, 0.4, 0.5))
    m.tube("iron_light", (-4.0, h, 1.0), (-4.0, h + 1.1, 1.0), 0.45, 0.5, segs=10)
    m.torus("iron", (-3.4, h + 0.6, 1.0), (0, 0, 1), 0.35, 0.07, segs=8, sides=4)
    m.tube("iron_light", (3.0, h, -1.5), (3.0, h + 0.1, -1.5), 1.2, 1.25, segs=14)
    m.spheroid("bone", (3.0, h + 0.35, -1.5), (0.6, 0.3, 0.45), segs=8, rings=5)
    candle(m, (0.0, h, 2.5))
    return m


def bookcase():
    """A tall bookcase against the wall (3 x 1), full of books."""
    m = M.Model("bookcase", colors(WOOD, METAL, MISC))
    rnd = random.Random(7)
    w, d, h = 21.0, 5.0, 13.0
    zb = -3.6
    m.box("wood_dark", (0, h / 2, zb + 0.2), (w, h, 0.4))
    for sx in (-1, 1):
        m.box("wood", (sx * (w / 2 - 0.4), h / 2, zb + d / 2), (0.8, h, d))
    m.box("wood", (0, h + 0.3, zb + d / 2 + 0.2), (w + 0.8, 0.6, d + 0.6))
    m.box("wood", (0, 0.4, zb + d / 2), (w, 0.8, d))
    shelves = [0.8, 3.9, 7.0, 10.1]
    for y in shelves[1:]:
        m.box("wood", (0, y, zb + d / 2), (w - 1.6, 0.35, d - 0.2))
    for y in shelves:
        book_row(m, rnd, -w / 2 + 0.9, w / 2 - 0.9, y + (0.18 if y > 1 else 0.0), zb + 0.5, d - 1.2, 1.8, 2.7)
    # a skull and a candle on the top shelf
    m.spheroid("bone", (6.5, 10.7, zb + 2.8), (0.6, 0.65, 0.65), segs=10, rings=7)
    candle(m, (-7.0, h + 0.6, zb + 2.5))
    return m


def cupboard():
    """A wide cupboard (3 x 1) with three doors."""
    m = M.Model("cupboard", colors(WOOD, METAL, MISC))
    w, d, h = 21.0, 5.5, 12.5
    zb = -3.6
    m.box("wood", (0, h / 2 + 0.6, zb + d / 2), (w, h - 1.2, d))
    m.box("wood_dark", (0, 0.3, zb + d / 2), (w + 0.2, 0.6, d + 0.2))
    m.box("wood_dark", (0, h + 0.1, zb + d / 2 + 0.2), (w + 1.0, 0.8, d + 0.8))
    for i in (-1, 0, 1):
        x = i * 6.8
        m.box("wood_light", (x, h / 2 + 0.6, zb + d + 0.1), (6.2, h - 2.6, 0.25))
        m.box("wood", (x, h / 2 + 0.6, zb + d + 0.25), (4.8, h - 4.4, 0.15))
        m.spheroid("brass", (x + 2.3 * (1 if i < 1 else -1), h / 2 + 0.6, zb + d + 0.45), (0.25, 0.25, 0.25), segs=6, rings=4)
    for x in (-9.5, 9.5):
        m.box("iron", (x, h - 1.5, zb + d + 0.3), (0.4, 0.9, 0.12))
    return m


def fireplace():
    """A stone fireplace (3 x 1) with burning logs, a mantel and a chimney breast."""
    m = M.Model("fireplace", colors(STONE, WOOD, METAL, MISC))
    w, h = 22.0, 15.9
    zb = -4.0
    # the chimney breast up to the ceiling
    m.box("stone", (0, h / 2, zb + 1.2), (13.0, h, 2.4))
    # the hearth's frame of big blocks
    for sx in (-1, 1):
        for k in range(4):
            m.box("stone_light" if k % 2 else "stone", (sx * 7.5, 0.9 + k * 1.7, zb + 2.2), (3.2 + (k % 2) * 0.6, 1.65, 4.4))
    m.box("stone_dark", (0, 0.25, zb + 3.2), (w, 0.5, 6.0))
    m.box("wood_dark", (0, 7.4, zb + 2.6), (w, 0.9, 5.2))
    m.box("stone_light", (0, 6.6, zb + 2.4), (16.0, 0.9, 4.6))
    # the dark opening and the fire
    # the chimney breast's front is at zb + 2.4; the fire burns in front of it, inside the frame
    m.box("black", (0, 3.3, zb + 2.5), (11.0, 6.2, 0.2))
    for a, z in ((0.3, 3.0), (-0.4, 3.6), (1.2, 3.2)):
        m.tube("wood_dark", (-3.2, 0.9, zb + z), (3.2, 1.0, zb + z + a * 0.3), 0.45, 0.42, segs=8)
    for x, s in ((-1.5, 1.0), (0.4, 1.4), (2.0, 0.9)):
        m.spheroid("ember", (x, 1.7, zb + 3.3), (0.8 * s, 1.3 * s, 0.5 * s), segs=8, rings=6)
        m.spheroid("flame", (x, 2.3 * s, zb + 3.4), (0.4 * s, 0.9 * s, 0.3 * s), segs=8, rings=6)
    # andirons
    for x in (-3.0, 3.0):
        m.box("iron", (x, 0.9, zb + 4.1), (0.3, 1.4, 0.3))
    # a candle and a jug on the mantel
    candle(m, (-5.5, 7.85, zb + 3.0))
    m.tube("stone_light", (5.0, 7.85, zb + 3.0), (5.0, 9.0, zb + 3.0), 0.5, 0.35, segs=10)
    return m


def chest():
    """A treasure chest (1 x 1) with iron bands and a lock."""
    m = M.Model("chest", colors(WOOD, METAL, MISC))
    w, d, h = 5.2, 3.4, 2.4
    m.box("wood", (0, h / 2, 0), (w, h, d))
    # the rounded lid: half a cylinder along x
    lid = m.tube("wood_light", (-w / 2, h, 0), (w / 2, h, 0), d / 2, d / 2, segs=14)
    m.box("wood", (0, h + 0.01, 0), (w, 0.02, d))
    for x in (-w / 2 + 0.5, 0.0, w / 2 - 0.5):
        m.box("iron", (x, h / 2, 0), (0.3, h + 0.04, d + 0.08))
        m.torus("iron", (x, h, 0), (1, 0, 0), d / 2 + 0.03, 0.12, segs=14, sides=4)
    m.box("gold", (0, h - 0.1, d / 2 + 0.06), (0.7, 0.8, 0.15))
    m.spheroid("black", (0, h - 0.2, d / 2 + 0.15), (0.1, 0.16, 0.05), segs=6, rings=4)
    # the lower half of the lid's cylinder is inside the box; cut it away by keeping only y >= h
    lid.tris = [t for t in lid.tris if min(lid.verts[i][0][1] for i in t) >= h - 1e-6]
    return m


def throne():
    """A high-backed throne (1 x 1) of dark wood with a red cushion and gold finials."""
    m = M.Model("throne", colors(WOOD, METAL, MISC, STONE))
    m.box("stone_dark", (0, 0.4, 0.4), (6.4, 0.8, 6.0))
    m.box("wood_dark", (0, 2.2, 0.5), (4.6, 2.6, 4.0))
    m.box("red", (0, 3.65, 0.6), (4.0, 0.5, 3.6))
    m.box("wood_dark", (0, 6.2, -1.6), (4.6, 10.0, 0.8))
    m.box("red", (0, 6.0, -1.15), (3.6, 4.8, 0.25))
    for sx in (-1, 1):
        m.box("wood", (sx * 2.4, 4.3, 0.6), (0.6, 0.5, 4.0))
        m.box("wood_dark", (sx * 2.4, 3.2, 2.3), (0.6, 2.2, 0.6))
        m.tube("wood", (sx * 2.1, 1.2, -1.6), (sx * 2.1, 11.0, -1.6), 0.35, 0.3, segs=8)
        m.spheroid("gold", (sx * 2.1, 11.3, -1.6), (0.45, 0.45, 0.45), segs=8, rings=6)
    m.slab("gold", [(-1.2, 0), (1.2, 0), (0.8, 0.9), (0.0, 1.5), (-0.8, 0.9)], (0, 10.4, -1.15), (1, 0, 0), (0, 1, 0), 0.15)
    return m


def tomb():
    """A stone sarcophagus (3 x 2) with a carved lid and corner posts."""
    m = M.Model("tomb", colors(STONE, METAL, MISC))
    w, d = 20.0, 10.0
    m.box("stone_dark", (0, 0.4, 0), (w + 1.0, 0.8, d + 1.0))
    m.box("stone", (0, 2.0, 0), (w, 2.6, d))
    m.box("stone_light", (0, 3.55, 0), (w + 0.6, 0.6, d + 0.6))
    m.box("stone", (0, 4.0, 0), (w - 1.0, 0.4, d - 1.0))
    # a carved figure on the lid, arms crossed, and a sword
    m.spheroid("stone_light", (-7.0, 4.5, 0), (0.9, 0.55, 0.9), segs=10, rings=6)
    m.spheroid("stone_light", (-2.0, 4.4, 0), (4.0, 0.45, 2.2), segs=12, rings=6)
    m.spheroid("stone_light", (4.6, 4.35, 0), (3.2, 0.35, 1.6), segs=10, rings=6)
    m.box("stone_dark", (0.5, 4.75, 0), (8.0, 0.12, 0.35))
    m.box("stone_dark", (-3.2, 4.75, 0), (0.35, 0.12, 2.0))
    # panels on the sides
    for x in (-6.5, 0.0, 6.5):
        m.box("stone_dark", (x, 2.0, d / 2 + 0.05), (5.0, 1.6, 0.12))
    for sx in (-1, 1):
        for sz in (-1, 1):
            m.tube("stone_light", (sx * (w / 2 + 0.2), 0.0, sz * (d / 2 + 0.2)), (sx * (w / 2 + 0.2), 4.4, sz * (d / 2 + 0.2)), 0.6, 0.5, segs=8)
            candle(m, (sx * (w / 2 + 0.2), 4.4, sz * (d / 2 + 0.2)), 0.7)
    return m


def rack():
    """A rack (3 x 2): a heavy wooden frame with rollers, chains and ropes."""
    m = M.Model("rack", colors(WOOD, METAL, MISC))
    w, d, h = 19.0, 7.0, 3.4
    for sz in (-1, 1):
        m.box("wood_dark", (0, h, sz * d / 2), (w, 0.9, 0.9))
    legs(m, "wood_dark", w, d, h, 0.45, 0.9)
    for x in (-6.0, -2.0, 2.0, 6.0):
        m.box("wood", (x, h + 0.2, 0), (2.8, 0.35, d - 0.4))
    for sx in (-1, 1):
        m.tube("wood_light", (sx * (w / 2 - 1.2), h + 0.6, -d / 2), (sx * (w / 2 - 1.2), h + 0.6, d / 2), 0.55, 0.55, segs=10)
        for sz in (-1, 1):
            m.spheroid("iron", (sx * (w / 2 - 1.2), h + 0.6, sz * (d / 2 + 0.4)), (0.4, 0.4, 0.4), segs=8, rings=5)
        # the crank's spokes
        for a in range(4):
            ang = math.pi / 4 + a * math.pi / 2
            m.tube("wood", (sx * (w / 2 - 1.2), h + 0.6, d / 2 + 0.5),
                   (sx * (w / 2 - 1.2) + math.cos(ang) * 1.4, h + 0.6 + math.sin(ang) * 1.4, d / 2 + 0.5), 0.14, 0.12, segs=5)
        for sz in (-0.8, 0.8):
            m.tube("rope", (sx * (w / 2 - 1.2), h + 0.9, sz), (sx * (w / 2 - 3.5), h + 0.5, sz * 0.9), 0.09, 0.09, segs=5)
    return m


def weapons_rack():
    """A rack (3 x 1) against the wall with swords, spears, an axe and a shield."""
    m = M.Model("weapons_rack", colors(WOOD, METAL, MISC))
    zb = -3.4
    for x in (-9.5, 9.5):
        m.box("wood_dark", (x, 3.8, zb + 0.6), (0.9, 7.6, 1.2))
    m.box("wood", (0, 1.2, zb + 1.2), (20.0, 0.6, 2.4))
    m.box("wood", (0, 6.3, zb + 0.8), (20.0, 0.6, 1.0))
    for i, x in enumerate((-7.5, -5.5, -3.5)):
        m.blade("iron_light", (x, 1.5, zb + 1.2), (x, 6.6, zb + 0.9), 0.4, 0.08, 0.5)
        m.box("iron", (x, 1.4, zb + 1.2), (1.1, 0.15, 0.25))
        m.tube("wood_dark", (x, 0.6, zb + 1.2), (x, 1.35, zb + 1.2), 0.1, 0.1, segs=6)
    for x in (-0.8, 0.8):
        m.tube("wood", (x, 0.9, zb + 1.3), (x, 7.6, zb + 0.9), 0.12, 0.12, segs=6)
        m.slab("iron_light", [(-0.25, 0), (0.25, 0), (0.0, 1.1)], (x, 7.6, zb + 0.9), (1, 0, 0), (0, 1, 0), 0.08)
    m.tube("wood", (4.0, 0.9, zb + 1.3), (4.0, 6.8, zb + 0.9), 0.13, 0.13, segs=6)
    m.slab("iron_light", [(0, -0.6), (0.9, -0.8), (1.1, 0), (0.9, 0.8), (0, 0.6)], (4.05, 6.1, zb + 0.92), (1, 0, 0), (0, 1, 0), 0.1)
    # a round shield hanging on the wall
    m.tube("wood", (7.2, 4.2, zb + 0.4), (7.2, 4.2, zb + 0.75), 1.6, 1.6, segs=16)
    m.torus("iron", (7.2, 4.2, zb + 0.78), (0, 0, 1), 1.55, 0.12, segs=16, sides=4)
    m.spheroid("iron_light", (7.2, 4.2, zb + 0.8), (0.45, 0.45, 0.3), segs=8, rings=5)
    return m


def alembic_bench():
    """A workbench (3 x 2) with flasks, a retort over a burner, jars and books."""
    m = M.Model("alembic_bench", colors(WOOD, METAL, MISC, STONE))
    rnd = random.Random(11)
    w, d, h = 20.0, 10.0, 4.4
    m.box("wood", (0, h - 0.3, 0), (w, 0.6, d))
    legs(m, "wood_dark", w, d, h - 0.6, 0.9, 0.9)
    m.box("wood_dark", (0, 1.2, 0), (w - 1.8, 0.4, d - 1.8))
    # a shelf at the back with jars
    m.box("wood_dark", (0, h + 2.2, -d / 2 + 0.6), (w - 2.0, 0.35, 1.2))
    for x in (-8.5, 8.5):
        m.box("wood_dark", (x, h + 1.1, -d / 2 + 0.6), (0.4, 2.2, 1.2))
    for x in range(-7, 8, 2):
        sw = rnd.choice(["potion_r", "potion_g", "potion_b", "glass"])
        m.tube(sw, (x, h + 2.4, -d / 2 + 0.6), (x, h + 3.2, -d / 2 + 0.6), 0.32, 0.3, segs=8)
        m.tube("cloth", (x, h + 3.2, -d / 2 + 0.6), (x, h + 3.35, -d / 2 + 0.6), 0.34, 0.34, segs=8)
    # flasks of all sizes on the bench
    for pos, sw, r in (((-6.5, h, 1.5), "potion_g", 0.6), ((-5.0, h, -1.0), "potion_r", 0.45), ((-3.6, h, 2.2), "potion_b", 0.5),
                       ((5.5, h, 2.0), "glass", 0.4), ((7.0, h, -0.5), "potion_g", 0.35)):
        flask(m, pos, sw, r, 0.6)
    # a retort over a burner
    m.tube("brass", (1.5, h, 0.5), (1.5, h + 1.0, 0.5), 0.6, 0.5, segs=10)
    m.spheroid("flame", (1.5, h + 1.25, 0.5), (0.2, 0.35, 0.2), segs=6, rings=4)
    m.spheroid("glass", (1.5, h + 2.1, 0.5), (0.75, 0.7, 0.75), segs=10, rings=7)
    M_arc_end = (4.2, h + 1.4, 0.8)
    m.tube("glass", (2.0, h + 2.4, 0.5), M_arc_end, 0.16, 0.1, segs=6)
    flask(m, (4.4, h, 0.8), "potion_r", 0.35, 0.4)
    # books and a mortar
    m.box("book3", (-1.5, h + 0.2, 3.3), (2.0, 0.4, 1.5))
    m.box("book1", (-1.4, h + 0.55, 3.2), (1.8, 0.3, 1.4))
    m.tube("stone_light", (5.5, h, -2.8), (5.5, h + 0.7, -2.8), 0.6, 0.75, segs=10)
    m.tube("wood_light", (5.5, h + 0.4, -2.8), (6.2, h + 1.6, -2.5), 0.12, 0.1, segs=5)
    return m


def scrying_table():
    """A table (3 x 2) under a dark cloth with a crystal ball, a grimoire, candles and a skull."""
    m = M.Model("scrying_table", colors(WOOD, METAL, MISC))
    w, d, h = 20.0, 10.0, 4.6
    m.box("cloth", (0, h - 0.25, 0), (w + 0.6, 0.5, d + 0.6))
    m.box("cloth", (0, h - 1.3, d / 2 + 0.28), (w + 0.6, 2.1, 0.05))
    m.box("cloth", (0, h - 1.3, -d / 2 - 0.28), (w + 0.6, 2.1, 0.05))
    legs(m, "wood_dark", w, d, h - 0.5, 1.2, 0.8)
    # the crystal ball on a stand
    m.tube("brass", (0, h, 0), (0, h + 0.7, 0), 0.9, 0.6, segs=10)
    m.spheroid("potion_b", (0, h + 1.8, 0), (1.2, 1.2, 1.2), segs=14, rings=10)
    # an open grimoire
    for sx in (-1, 1):
        m.box("paper", (-6.0 + sx * 1.05, h + 0.25, 1.5), (2.0, 0.2, 2.8), rot_y=0.0, tilt=((0, 0, 1), -sx * 0.12))
    m.box("book5", (-6.0, h + 0.08, 1.5), (4.4, 0.16, 3.1))
    # candles, a skull, a dagger
    for pos in ((4.5, h, 2.5), (6.0, h, -1.5), (-8.0, h, -2.5)):
        candle(m, pos, 1.2)
    m.spheroid("bone", (6.5, h + 0.65, 1.8), (0.7, 0.7, 0.75), segs=10, rings=7)
    m.spheroid("black", (6.25, h + 0.75, 2.4), (0.15, 0.13, 0.08), segs=6, rings=4)
    m.spheroid("black", (6.75, h + 0.75, 2.4), (0.15, 0.13, 0.08), segs=6, rings=4)
    m.blade("iron_light", (-2.0, h + 0.1, -2.5), (1.0, h + 0.1, -2.0), 0.3, 0.06, 0.4)
    return m


# ---------------- floor overlays ----------------

def stairs():
    """A spiral stair (2 x 2) winding up into the ceiling: the way out."""
    m = M.Model("stairs", colors(STONE, METAL, WOOD))
    steps = 18
    m.tube("stone_dark", (0, 0, 0), (0, 16.0, 0), 1.2, 1.2, segs=12)
    for i in range(steps):
        a0 = i * math.tau / 12
        a1 = a0 + math.tau / 12 * 1.05
        y = 0.45 + i * 0.85
        outline = [(math.cos(a) * r, math.sin(a) * r) for r, a in ((1.0, a0), (6.8, a0), (6.8, (a0 + a1) / 2), (6.8, a1), (1.0, a1))]
        m.slab("stone" if i % 2 else "stone_light", outline, (0, y, 0), (1, 0, 0), (0, 0, 1), 0.5)
    # a low round wall around it
    m.torus("stone_dark", (0, 0.0, 0), (0, 1, 0), 7.3, 0.45, segs=24, sides=5)
    return m


def pittrap():
    """A discovered pit trap (1 x 1): a dark hole with broken edges in the floor."""
    m = M.Model("pittrap", colors(STONE, MISC))
    rnd = random.Random(3)
    hole = [(math.cos(a) * 3.0 * (1 + rnd.uniform(-0.12, 0.12)), math.sin(a) * 3.0 * (1 + rnd.uniform(-0.12, 0.12)))
            for a in (math.tau * k / 14 for k in range(14))]
    m.slab("black", hole, (0, 0.03, 0), (1, 0, 0), (0, 0, 1), 0.02)
    for k in range(10):
        a = math.tau * k / 10 + rnd.uniform(-0.2, 0.2)
        r = 3.2 + rnd.uniform(0, 0.4)
        m.spheroid("stone_dark", (math.cos(a) * r, 0.05, math.sin(a) * r), (rnd.uniform(0.3, 0.6), 0.18, rnd.uniform(0.3, 0.6)), segs=6, rings=4)
    return m


def rocktrap():
    """Fallen rocks (1 x 1), blocking the field."""
    m = M.Model("rocktrap", colors(STONE))
    rubble(m, random.Random(5), 3.6, 4.5, 14)
    return m


def speartrap():
    """A discovered spear trap (1 x 1): iron spikes out of holes in the floor."""
    m = M.Model("speartrap", colors(STONE, METAL, MISC))
    for i in range(3):
        for j in range(3):
            x, z = (i - 1) * 2.2, (j - 1) * 2.2
            m.tube("black", (x, 0.02, z), (x, 0.05, z), 0.5, 0.5, segs=8)
            m.tube("iron_light", (x, 0.0, z), (x, 2.2 + 0.3 * ((i + j) % 2), z), 0.22, 0.0, segs=6)
    return m


def rubble(m, rnd, radius, height, count):
    for k in range(count):
        a = rnd.uniform(0, math.tau)
        r = rnd.uniform(0, radius) * (1 - k / (count * 1.4))
        size = rnd.uniform(0.9, 1.8)
        y = size * 0.5 + (height - size) * (1 - r / radius) * rnd.uniform(0.3, 1.0)
        m.spheroid(rnd.choice(["stone", "stone_dark", "stone_light"]), (math.cos(a) * r, y, math.sin(a) * r),
                   (size * rnd.uniform(0.8, 1.2), size * rnd.uniform(0.55, 0.8), size * rnd.uniform(0.8, 1.2)), segs=7, rings=5)


# ---------------- the triggered traps (models.dat) and the script marker ----------------

def pit():
    """The field after a hero fell into the pit: a floor with a hole, walls down 4, spikes at the bottom."""
    m = M.Model("pit", colors(STONE, METAL, MISC))
    h = 4.0
    r = 3.0
    # the floor around the hole: four slabs
    for (x0, z0, x1, z1) in ((-4, -4, 4, -r), (-4, r, 4, 4), (-4, -r, -r, r), (r, -r, 4, r)):
        m.box("stone", ((x0 + x1) / 2, -0.05, (z0 + z1) / 2), (x1 - x0, 0.1, z1 - z0))
    # the hole's walls and bottom
    for rot in range(4):
        ang = rot * math.pi / 2
        c = (math.sin(ang) * r, -h / 2, math.cos(ang) * r)
        m.box("stone_dark", c, (2 * r, h, 0.2), rot_y=ang)
    m.box("black", (0, -h, 0), (2 * r, 0.1, 2 * r))
    for i in range(3):
        for j in range(3):
            m.tube("iron_light", ((i - 1) * 1.8, -h, (j - 1) * 1.8), ((i - 1) * 1.8, -h + 1.0, (j - 1) * 1.8), 0.18, 0.0, segs=5)
    return m


def world_rocktrap():
    """The field after the rocks fell: a big pile up to the ceiling."""
    m = M.Model("rocktrap", colors(STONE))
    rubble(m, random.Random(9), 3.9, 8.5, 24)
    return m


def script():
    """The marker of a scripted place: a rolled parchment hovering over the floor."""
    m = M.Model("script", colors(MISC, WOOD, METAL))
    m.tube("paper", (-1.2, 3.0, 0), (1.2, 3.0, 0), 0.45, 0.45, segs=12)
    for x in (-1.3, 1.3):
        m.tube("wood_dark", (x, 3.0, 0), (x * 1.08, 3.0, 0), 0.25, 0.25, segs=8)
    m.torus("red", (0, 3.0, 0), (1, 0, 0), 0.47, 0.06, segs=12, sides=4)
    m.spheroid("red", (0, 3.0, 0.5), (0.2, 0.2, 0.06), segs=6, rings=4)
    return m


FURNITURE = {"table": table, "bookcase": bookcase, "cupboard": cupboard, "fireplace": fireplace, "chest": chest,
             "throne": throne, "tomb": tomb, "rack": rack, "weapons_rack": weapons_rack,
             "alembic_bench": alembic_bench, "scrying_table": scrying_table}
OVERLAYS = {"stairs": stairs, "pittrap": pittrap, "rocktrap": rocktrap, "speartrap": speartrap}
TRAPS = {"pit": pit, "rocktrap": world_rocktrap}  # models/world, named in models/models.dat
OTHER = {"script": script}
