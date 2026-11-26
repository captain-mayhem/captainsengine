"""Paints Torchdelve's interface art.

Writes into TDClient/textures:
  items/<name>.png   an icon per item, piece of equipment and spell (128 px, transparent),
                     rendered from small models (modellib.render); items/icons.dat lists them.
                     The client draws them in the inventory's and the shop's slots by the
                     item's name (data/items.dat, data/armory.dat, data/spells.dat).
  other/inventory.jpg, other/shop.jpg
                     the inventory and the shop (1920 x 1080): dark leather panels on the
                     dungeon wall with bronze-framed slots exactly where the client takes the
                     clicks (inventory.cpp, trade.cpp: the old 1024 x 768 layout, which the
                     client centres on its 16:9 screen, layout.h; y down).
  other/panel.jpg    the side panel behind the buttons on the right (440 x 1080).

Usage: python tools/make_ui_art.py [--preview <png>]
"""

import argparse
import math
import os
import random

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

import furniture as FU
import modellib as M
import texlib as T

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
TEX = os.path.join(ROOT, "TDClient", "textures")
FONT = os.path.join(ROOT, "art", "fonts", "CinzelDecorative-Bold.ttf")
ICON = 128
W, H = 1024, 768

# the slots as the client takes the clicks (x0, x1) and (y0, y1), y down
COLS_LEFT = [(65, 155), (210, 300), (355, 450)]
COLS_RIGHT = [(590, 680), (735, 825), (880, 975)]
ROWS = [(125, 280), (315, 475), (510, 670)]

PAL = FU.colors(FU.WOOD, FU.METAL, FU.MISC, FU.STONE, {
    "steel": (180, 182, 188), "steel_dark": (96, 98, 104), "leather": (110, 72, 42),
    "amber": (214, 140, 40), "murk": (88, 96, 60), "gem": (90, 200, 230), "cloak": (34, 40, 86),
    "silver": (220, 224, 232), "copper": (196, 104, 52), "ember": (255, 170, 60),
    # the spell classes (data/spellClasses.dat) and Umbrath's shadow
    "spell_stone": (130, 140, 110), "spell_ember": (230, 100, 40), "spell_gale": (150, 200, 230),
    "spell_tide": (60, 110, 210), "spell_shadow": (110, 50, 130), "seal": (150, 30, 30)})


# ---------------- icon models ----------------

def model(name):
    return M.Model(name, PAL)


def potion(name, swatch, shape="round"):
    m = model(name)
    if shape == "round":
        m.spheroid(swatch, (0, 1.4, 0), (1.3, 1.3, 1.3), segs=14, rings=10)
        m.tube("glass", (0, 2.4, 0), (0, 3.6, 0), 0.42, 0.36, segs=10)
    elif shape == "bottle":
        m.tube(swatch, (0, 0, 0), (0, 2.3, 0), 1.0, 1.0, segs=14)
        m.spheroid(swatch, (0, 2.3, 0), (1.0, 0.55, 1.0), segs=14, rings=6)
        m.tube("glass", (0, 2.6, 0), (0, 3.7, 0), 0.38, 0.34, segs=10)
    else:  # a thin vial
        m.tube(swatch, (0, 0.3, 0), (0, 3.0, 0), 0.6, 0.55, segs=12)
        m.spheroid(swatch, (0, 0.3, 0), (0.6, 0.35, 0.6), segs=12, rings=5)
        m.tube("glass", (0, 3.0, 0), (0, 3.6, 0), 0.3, 0.3, segs=8)
    m.tube("wood_light", (0, 3.5, 0), (0, 4.1, 0), 0.45, 0.4, segs=8)
    return m


def sword(name, length, width, guard=1.4, swatch="steel"):
    m = model(name)
    d = M.norm((1, 1, 0))
    base = (-length * 0.3 * d[0], -length * 0.3 * d[1], 0)
    tip = M.add(base, M.mul(d, length))
    m.blade(swatch, base, tip, width, 0.12, width * 1.2)
    side = (-d[1], d[0], 0)
    m.tube("steel_dark", M.add(base, M.mul(side, guard / 2)), M.add(base, M.mul(side, -guard / 2)), 0.16, 0.16, segs=8)
    grip_end = M.add(base, M.mul(d, -1.3))
    m.tube("leather", base, grip_end, 0.14, 0.14, segs=8)
    m.spheroid("gold", grip_end, (0.26, 0.26, 0.26), segs=8, rings=6)
    return m


def axe(name, handle, blades, size=1.0):
    """An axe: a haft along the diagonal and one or two blades at its top."""
    m = model(name)
    top = (1.2 * handle, 1.6 * handle, 0)
    m.tube("wood", (-1.8 * handle, -2.0 * handle, 0), top, 0.16, 0.16, segs=8)
    for sx in blades:
        blade = [(0, -0.3), (1.2, -1.0), (1.4, 0), (1.2, 1.0), (0, 0.3)]
        m.slab("steel", [(sx * x * size, y * size) for x, y in blade], M.mul(top, 0.72), M.norm((0.77, -0.64, 0)), M.norm((0.64, 0.77, 0)), 0.14)
    return m


def build_icons():
    icons = {}
    icons["hearthbalm"] = potion("hearthbalm", "amber")
    icons["battle_ale"] = potion("battle_ale", "wood_dark", "bottle")
    icons["iron_tonic"] = potion("iron_tonic", "steel_dark")
    icons["fire_tonic"] = potion("fire_tonic", "potion_r", "bottle")
    icons["murky_vial"] = potion("murky_vial", "murk", "vial")
    icons["strongbox"] = FU.chest()
    m = model("twin_wand")
    # two woods twisted into one
    for k, wood in enumerate(("wood_dark", "wood_light")):
        pts = []
        for i in range(9):
            t = i / 8
            a = t * math.tau * 1.5 + k * math.pi
            c = (-1.6 + 3.0 * t, -1.6 + 3.0 * t, 0)
            pts.append(M.add(c, (math.cos(a) * 0.09, -math.cos(a) * 0.09, math.sin(a) * 0.12)))
        for a, b in zip(pts, pts[1:]):
            m.tube(wood, a, b, 0.1, 0.1, segs=6)
    m.torus("gold", (1.2, 1.2, 0), (1, 1, 0), 0.22, 0.07, segs=10, sides=4)
    m.spheroid("ember", (1.65, 1.65, 0), (0.36, 0.36, 0.36), segs=10, rings=7)
    icons["twin_wand"] = m
    # equipment
    m = model("trap_tools")
    m.box("leather", (0, 0.9, 0), (3.2, 1.8, 1.6))
    m.tube("leather", (-1.6, 1.8, 0), (1.6, 1.8, 0), 0.8, 0.8, segs=12)
    m.box("wood", (0.3, 2.6, 0.3), (0.25, 2.2, 0.25), rot_y=0, tilt=((0, 0, 1), 0.5))
    m.box("iron", (-0.25, 3.5, 0.3), (1.2, 0.4, 0.4), tilt=((0, 0, 1), 0.5))
    m.box("iron", (-0.7, 2.6, -0.3), (0.15, 1.8, 0.15), tilt=((0, 0, 1), -0.4))
    icons["trap_tools"] = m
    icons["dirk"] = sword("dirk", 2.8, 0.42, 0.8)
    icons["sidesword"] = sword("sidesword", 3.8, 0.45, 1.5)
    icons["falchion"] = sword("falchion", 4.2, 0.95, 1.4)
    m = model("halberd")
    m.tube("wood", (-2.0, -2.0, 0), (1.9, 1.9, 0), 0.13, 0.13, segs=8)
    m.slab("steel", [(0, -0.2), (1.0, -0.8), (1.2, 0.1), (0.9, 0.9), (0, 0.3)], (1.15, 1.15, 0), M.norm((0.71, -0.71, 0)), M.norm((0.71, 0.71, 0)), 0.12)
    m.slab("steel", [(0, -0.15), (-0.7, 0), (0, 0.15)], (1.15, 1.15, 0), M.norm((0.71, -0.71, 0)), M.norm((0.71, 0.71, 0)), 0.1)
    m.tube("steel", (1.9, 1.9, 0), (2.4, 2.4, 0), 0.1, 0.0, segs=6)
    icons["halberd"] = m
    icons["hand_axe"] = axe("hand_axe", 0.8, (1,), 1.0)
    icons["great_axe"] = axe("great_axe", 1.0, (1, -1), 1.1)
    m = model("iron_helm")
    m.spheroid("steel", (0, 0.2, 0), (1.5, 1.6, 1.6), segs=16, rings=10)
    m.torus("steel_dark", (0, 0.15, 0), (0, 1, 0), 1.5, 0.14, segs=18, sides=5)
    m.box("steel_dark", (0, 0.0, 1.5), (0.3, 1.2, 0.15))
    m.parts[0].tris = [t for t in m.parts[0].tris if min(m.parts[0].verts[i][0][1] for i in t) >= 0.15]
    icons["iron_helm"] = m
    m = model("buckler")
    m.tube("iron", (0, 0, -0.2), (0, 0, 0.2), 1.6, 1.6, segs=20)
    m.torus("brass", (0, 0, 0.22), (0, 0, 1), 1.55, 0.14, segs=20, sides=5)
    m.spheroid("iron_light", (0, 0, 0.3), (0.6, 0.6, 0.5), segs=10, rings=7)
    icons["buckler"] = m
    m = model("oak_staff")
    m.tube("wood_dark", (-1.8, -1.8, 0), (1.6, 1.6, 0), 0.18, 0.15, segs=8)
    m.torus("iron", (1.1, 1.1, 0), (1, 1, 0), 0.17, 0.06, segs=10, sides=4)
    m.spheroid("wood_dark", (1.75, 1.75, 0), (0.36, 0.36, 0.36), segs=10, rings=7)
    icons["oak_staff"] = m
    for name, mail, extra in (("mail_shirt", "steel_dark", None), ("full_plate", "steel", "steel_dark"), ("emberplate", "copper", "gold")):
        m = model(name)
        m.spheroid(mail, (0, 1.4, 0), (1.5, 1.6, 0.9), segs=14, rings=10)
        m.tube(mail, (0, 0.1, 0), (0, 0.9, 0), 1.3, 1.45, segs=14, squash=0.65)
        for sx in (-1, 1):
            m.spheroid(extra or mail, (sx * 1.45, 2.4, 0), (0.6, 0.45, 0.6), segs=10, rings=7)
        m.box("leather", (0, 0.65, 0.9), (2.6, 0.3, 0.2))
        icons[name] = m
    m = model("arbalest")
    m.box("wood", (0, 0, 0), (0.45, 3.8, 0.45), tilt=((0, 0, 1), -0.6))
    M_c = (0.9, 1.25, 0)
    pts = [M.add(M_c, (math.cos(a) * 1.9, math.sin(a) * 0.55 - 0.3, 0)) for a in (math.radians(v) for v in range(200, 341, 20))]
    for a, b in zip(pts, pts[1:]):
        m.tube("steel", M.rotate(a, (0, 0, 1), -0.6), M.rotate(b, (0, 0, 1), -0.6), 0.15, 0.15, segs=6)
    m.tube("rope", M.rotate(pts[0], (0, 0, 1), -0.6), M.rotate(pts[-1], (0, 0, 1), -0.6), 0.04, 0.04, segs=4)
    # the winch at the back
    m.tube("iron", M.rotate((-0.4, -1.5, -0.4), (0, 0, 1), -0.6), M.rotate((-0.4, -1.5, 0.4), (0, 0, 1), -0.6), 0.3, 0.3, segs=8)
    icons["arbalest"] = m
    m = model("lamp_charm")
    # a tiny lantern on a chain, its flame burning
    m.torus("brass", (0, 1.9, 0), (0, 0, 1), 0.35, 0.07, segs=12, sides=4)
    m.box("brass", (0, 1.35, 0), (1.2, 0.2, 1.2))
    m.box("brass", (0, -1.05, 0), (1.3, 0.25, 1.3))
    for sx in (-0.5, 0.5):
        for sz in (-0.5, 0.5):
            m.tube("brass", (sx, -1.0, sz), (sx, 1.3, sz), 0.07, 0.07, segs=5)
    m.spheroid("ember", (0, -0.2, 0), (0.38, 0.75, 0.38), segs=10, rings=7)
    icons["lamp_charm"] = m
    m = model("star_cloak")
    m.tube("cloak", (0, 2.0, 0), (0, -1.8, 0.1), 0.9, 2.1, segs=16, squash=0.5)
    m.spheroid("cloak", (0, 2.1, -0.3), (1.0, 0.7, 0.6), segs=12, rings=8)
    for x, y in ((-0.9, -0.6), (0.6, 0.2), (0.1, -1.3), (-0.3, 0.9), (1.2, -1.0)):
        m.spheroid("silver", (x, y, 0.95 - abs(x) * 0.25), (0.13, 0.13, 0.06), segs=6, rings=4)
    m.spheroid("silver", (0, 1.8, 0.45), (0.25, 0.25, 0.12), segs=8, rings=5)
    icons["star_cloak"] = m
    return icons


def spell_icon(name, element):
    """A spell: a rolled scroll in its element's colour with a wax seal."""
    m = model(name)
    m.tube("paper", (-1.9, -0.9, 0), (1.9, 0.9, 0), 0.75, 0.75, segs=14)
    for t in (-1.0, 1.0):
        c = (t * 2.05, t * 0.97, 0)
        m.tube(element, M.add(c, (-0.12, -0.06, 0)), M.add(c, (0.12, 0.06, 0)), 0.5, 0.5, segs=10)
    axis = M.norm((1.9, 0.9, 0))
    m.tube(element, M.mul(axis, -0.18), M.mul(axis, 0.18), 0.8, 0.8, segs=14)
    m.spheroid("seal", (0.1, -0.05, 0.8), (0.32, 0.32, 0.12), segs=10, rings=6)
    return m


def render_icon(m):
    skin = m.paint_skin(3)
    img = M.render(m, skin, 0.35, 0.3, (ICON * 2, ICON * 2), light=(-0.5, 0.75, 0.75), ambient=0.38)
    img = img.resize((ICON, ICON), Image.LANCZOS)
    # a soft dark outline, so the icon reads on any background
    alpha = img.getchannel("A")
    halo = alpha.filter(ImageFilter.MaxFilter(5)).filter(ImageFilter.GaussianBlur(2)).point(lambda v: v * 0.7)
    out = Image.new("RGBA", img.size, (8, 6, 5, 0))
    out.putalpha(halo)
    out.alpha_composite(img)
    return out


# ---------------- panels ----------------

def wall_background(size=(W, H), start=(0, 0)):
    """The dungeon wall, darkened; its tiles start at start (so a bigger wall continues a smaller one)."""
    wall = Image.open(os.path.join(TEX, "world", "floorWall.jpg")).convert("RGB").resize((512, 512))
    bg = Image.new("RGB", size)
    for x in range(start[0], size[0], 512):
        for y in range(start[1], size[1], 512):
            bg.paste(wall, (x, y))
    return T.apply_light(bg, Image.new("L", size, 48))


# the client's 16:9 screen and where it puts the old 1024 x 768 layout (layout.h)
SCREEN = (1920, 1080)
FRAME = ((1920 - W) // 2, (1080 - H) // 2)


def widescreen(img):
    """The 1024 x 768 picture centred on a 1920 x 1080 wall whose tiles continue its own."""
    big = wall_background(SCREEN, (FRAME[0] % 512 - 512, FRAME[1] % 512 - 512))
    big.paste(img, FRAME)
    return big


def side_panel():
    """The side panel behind the buttons: leather with a bronze frame, full height."""
    w, h = 440, 1080
    img = leather((w, h), 7)
    d = ImageDraw.Draw(img)
    d.rectangle((0, 0, w - 1, h - 1), outline=(150, 104, 48), width=4)
    d.rectangle((10, 10, w - 11, h - 11), outline=(84, 56, 26), width=2)
    for cy in (30, h - 30):
        for cx in (30, w - 30):
            d.ellipse((cx - 8, cy - 8, cx + 8, cy + 8), fill=(150, 112, 56), outline=(70, 48, 20), width=2)
            d.ellipse((cx - 4, cy - 6, cx, cy - 2), fill=(220, 186, 120))
    # a darker band at the left edge, as if the panel stood out of the wall
    edge = Image.new("L", (w, h), 0)
    ImageDraw.Draw(edge).rectangle((0, 0, 18, h), fill=110)
    img.paste((0, 0, 0), (0, 0), edge.filter(ImageFilter.GaussianBlur(8)))
    return img


def leather(size, seed):
    w, h = size
    img = Image.new("RGB", size, (58, 38, 24))
    noise = T.fbm(256, seed, ((6, 0.4), (24, 0.35), (80, 0.25))).resize(size, Image.BICUBIC)
    img = T.modulate(img, noise, 0.4)
    # darker towards the edges
    vign = Image.new("L", size, 0)
    ImageDraw.Draw(vign).rectangle((20, 20, w - 20, h - 20), fill=255)
    vign = vign.filter(ImageFilter.GaussianBlur(30)).point(lambda v: 80 + v * 50 // 255)
    return T.apply_light(img, vign)


def panel(img, box, title, font, seed, title_x=None):
    x0, y0, x1, y1 = box
    shadow = Image.new("L", (W, H), 0)
    ImageDraw.Draw(shadow).rounded_rectangle((x0 + 8, y0 + 10, x1 + 8, y1 + 10), 14, fill=200)
    img.paste((0, 0, 0), (0, 0), shadow.filter(ImageFilter.GaussianBlur(10)))
    lth = leather((x1 - x0, y1 - y0), seed)
    mask = Image.new("L", lth.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, lth.size[0] - 1, lth.size[1] - 1), 14, fill=255)
    img.paste(lth, (x0, y0), mask)
    d = ImageDraw.Draw(img)
    d.rounded_rectangle(box, 14, outline=(150, 104, 48), width=4)
    d.rounded_rectangle((x0 + 8, y0 + 8, x1 - 8, y1 - 8), 10, outline=(84, 56, 26), width=2)
    # corner studs
    for cx, cy in ((x0 + 22, y0 + 22), (x1 - 22, y0 + 22), (x0 + 22, y1 - 22), (x1 - 22, y1 - 22)):
        d.ellipse((cx - 7, cy - 7, cx + 7, cy + 7), fill=(150, 112, 56), outline=(70, 48, 20), width=2)
        d.ellipse((cx - 4, cy - 5, cx, cy - 1), fill=(220, 186, 120))
    if title:  # (None: no title)
        tw = d.textlength(title, font=font)
        cx = title_x if title_x is not None else (x0 + x1) / 2
        d.line((cx - tw / 2 - 66, y0 + 52, cx - tw / 2 - 16, y0 + 52), fill=(150, 104, 48), width=2)
        d.line((cx + tw / 2 + 16, y0 + 52, cx + tw / 2 + 66, y0 + 52), fill=(150, 104, 48), width=2)
        d.text((cx - tw / 2 + 1, y0 + 34), title, font=font, fill=(20, 12, 6))
        d.text((cx - tw / 2, y0 + 32), title, font=font, fill=(226, 184, 104))


def slot(img, x0, x1, y0, y1):
    """A recessed slot with a bronze frame; the client draws the icon in its upper part and
    prints the item's name over its lower edge and the count under it."""
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((x0 - 7, y0 - 7, x1 + 7, y1 + 28), 8, fill=(30, 20, 13))
    d.rounded_rectangle((x0 - 6, y0 - 6, x1 + 6, y1 + 6), 6, fill=(112, 78, 36), outline=(60, 40, 18), width=2)
    d.rounded_rectangle((x0 - 2, y0 - 2, x1 + 2, y1 + 2), 4, fill=(14, 11, 9))
    # an inner shadow at the top, a faint glow at the bottom
    inner = Image.new("L", (x1 - x0, y1 - y0), 0)
    ImageDraw.Draw(inner).rectangle((0, 0, x1 - x0, 14), fill=120)
    img.paste((0, 0, 0), (x0, y0), inner.filter(ImageFilter.GaussianBlur(6)))
    # the strip of the name (the client prints it at y1 - 13 to y1 + 3) and the count below
    d.rectangle((x0 - 12, y1 - 16, x1 + 12, y1 + 24), fill=(24, 17, 12))
    d.line((x0 - 12, y1 - 17, x1 + 12, y1 - 17), fill=(96, 68, 32), width=1)


def page_plate(img, x0, x1, y):
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((x0, y - 12, x1, y + 14), 6, fill=(30, 20, 13), outline=(112, 78, 36), width=2)


def make_inventory(font):
    img = wall_background()
    panel(img, (28, 22, 488, 746), "Inventory", font, 1)
    # no title on the hero's sheet: the client prints the stats at its top (y 52 to 328)
    panel(img, (548, 22, 1000, 746), None, font, 2)
    for x0, x1 in COLS_LEFT:
        for y0, y1 in ROWS:
            slot(img, x0, x1, y0, y1)
    page_plate(img, 168, 336, 710)
    # the lines of the character sheet (the client prints the stats at y 68 to 328)
    d = ImageDraw.Draw(img)
    for y in (112, 152, 336):
        d.line((590, y, 960, y), fill=(84, 56, 26), width=2)
    return img


def make_shop(font):
    img = wall_background()
    # the client prints the money at x 350, y 72 to 88: the title moves to the left
    panel(img, (28, 22, 488, 746), "Inventory", font, 3, title_x=215)
    panel(img, (536, 22, 1000, 746), "Shop", font, 4)
    for cols in (COLS_LEFT, COLS_RIGHT):
        for x0, x1 in cols:
            for y0, y1 in ROWS:
                slot(img, x0, x1, y0, y1)
    page_plate(img, 168, 336, 710)
    page_plate(img, 694, 862, 710)
    return img


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--preview", help="write a contact sheet of the icons and the panels here")
    args = parser.parse_args()
    out = os.path.join(TEX, "items")
    os.makedirs(out, exist_ok=True)
    icons = {name: render_icon(m) for name, m in build_icons().items()}
    with open(os.path.join(TEX, "..", "data", "spells.dat")) as f:
        for line in f:
            words = line.split()
            if "class:" in words:
                name, element = words[0], words[words.index("class:") + 1]
                icons[name] = render_icon(spell_icon(name, "spell_" + element))
    for name, img in icons.items():
        img.save(os.path.join(out, name + ".png"), optimize=True)
    with open(os.path.join(out, "icons.dat"), "w", newline="\r\n") as f:
        f.write("".join(name + "\n" for name in icons))
    print(f"wrote {len(icons)} icons into textures/items")
    font = ImageFont.truetype(FONT, 34)
    inv = make_inventory(font)
    shop = make_shop(font)
    widescreen(inv).save(os.path.join(TEX, "other", "inventory.jpg"), quality=92, optimize=True)
    widescreen(shop).save(os.path.join(TEX, "other", "shop.jpg"), quality=92, optimize=True)
    side_panel().save(os.path.join(TEX, "other", "panel.jpg"), quality=92, optimize=True)
    print("wrote textures/other/inventory.jpg, shop.jpg, panel.jpg")
    if args.preview:
        cols = 11
        rows = (len(icons) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * ICON, rows * ICON + H // 2), (40, 30, 22))
        for k, img in enumerate(icons.values()):
            sheet.paste(img, ((k % cols) * ICON, (k // cols) * ICON), img)
        sheet.paste(inv.resize((W // 2, H // 2)), (0, rows * ICON))
        sheet.paste(shop.resize((W // 2, H // 2)), (W // 2 + 20, rows * ICON))
        sheet.save(args.preview)


if __name__ == "__main__":
    main()
