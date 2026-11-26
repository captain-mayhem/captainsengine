"""Writes Torchdelve's 3D models into TDClient/models.

Each model is name.obj with name.mtl and its painted skin name_skin.png (see
modellib.py): the heroes (the warrior here, the others in figures.py),
the monsters and the start field (figures.py), the furniture, the floor
overlays (stairs, traps), the triggered traps' models and the script marker
(furniture.py).

Usage: python tools/make_models.py [--only <name>...] [--preview <folder>]
       --preview also renders each model from four sides into <folder>/<name>.png.
"""

import argparse
import os

import figures as F
import furniture as FU
import modellib as M

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
MODELS = os.path.join(ROOT, "TDClient", "models")

# shared colours (painterly, a bit muted; the torchlight warms them in the game)
SKIN_TONE = (190, 136, 100)
COLORS = {
    "skin": SKIN_TONE,
    "skin_dark": (150, 100, 72),
    "hair": (122, 56, 30),
    "fur": (112, 88, 62),
    "fur_dark": (76, 58, 42),
    "leather": (96, 62, 38),
    "leather_dark": (54, 36, 24),
    "steel": (176, 178, 184),
    "steel_dark": (88, 90, 96),
    "gold": (204, 162, 72),
    "eye": (24, 18, 16),
    "red": (122, 38, 30),
}


def warrior():
    m = M.Model("warrior", COLORS)
    right = []  # parts on the right side (+x); the left side is mirrored
    # legs and boots
    right.append(m.spheroid("leather_dark", (0.48, 0.22, 0.22), (0.34, 0.24, 0.5)))
    right.append(m.tube("leather_dark", (0.48, 0.0, 0.0), (0.48, 1.1, 0.0), 0.4, 0.36))
    right.append(m.tube("fur", (0.48, 1.0, 0.0), (0.48, 1.18, 0.0), 0.42, 0.42))
    right.append(m.tube("skin", (0.48, 1.1, 0.0), (0.5, 2.2, 0.02), 0.31, 0.36))
    right.append(m.tube("skin", (0.5, 2.15, 0.02), (0.52, 3.05, 0.0), 0.37, 0.46))
    # shoulder, arm, bracer, hand
    right.append(m.spheroid("skin", (1.22, 4.95, 0.0), (0.5, 0.46, 0.48)))
    right.append(m.tube("skin", (1.32, 4.85, 0.0), (1.58, 3.8, 0.05), 0.38, 0.31))
    right.append(m.spheroid("skin", (1.58, 3.8, 0.05), (0.31, 0.31, 0.31)))
    right.append(m.tube("skin", (1.58, 3.8, 0.05), (1.55, 2.95, 0.42), 0.3, 0.24))
    right.append(m.tube("leather", (1.575, 3.45, 0.2), (1.552, 2.98, 0.4), 0.33, 0.28))
    right.append(m.spheroid("skin", (1.55, 2.72, 0.48), (0.24, 0.26, 0.24)))
    m.mirror_x(right)
    # the fur kilt and the belt
    m.tube("fur", (0.0, 2.25, 0.0), (0.0, 3.4, 0.0), 1.18, 0.98, segs=14, squash=0.8)
    m.tube("fur_dark", (0.0, 2.2, 0.0), (0.0, 2.4, 0.0), 1.22, 1.17, segs=14, squash=0.8)
    m.tube("leather", (0.0, 3.3, 0.0), (0.0, 3.62, 0.0), 1.0, 1.0, segs=14, squash=0.8)
    m.box("gold", (0.0, 3.46, 0.82), (0.36, 0.28, 0.08))
    # the body: belly and chest
    m.spheroid("skin", (0.0, 4.2, 0.0), (1.0, 1.05, 0.68))
    m.spheroid("skin", (0.0, 4.72, 0.06), (1.18, 0.66, 0.7))
    # a fur pad on the left shoulder
    m.spheroid("fur", (-1.26, 5.12, 0.0), (0.64, 0.38, 0.62))
    # neck and head
    m.tube("skin", (0.0, 5.15, 0.0), (0.0, 5.55, 0.02), 0.32, 0.28)
    m.spheroid("skin", (0.0, 5.86, 0.04), (0.42, 0.5, 0.45), segs=14, rings=10)
    m.spheroid("hair", (0.0, 6.0, -0.08), (0.48, 0.46, 0.48), segs=14, rings=10)
    m.tube("hair", (0.0, 5.95, -0.32), (0.0, 4.85, -0.55), 0.36, 0.12, segs=8, squash=0.6)
    m.tube("hair", (0.0, 5.72, 0.3), (0.0, 5.12, 0.34), 0.34, 0.1, segs=10, squash=0.6)
    for x in (-0.16, 0.16):
        m.spheroid("eye", (x, 5.93, 0.43), (0.06, 0.05, 0.03), segs=6, rings=4)
        m.box("hair", (x, 6.02, 0.43), (0.2, 0.05, 0.06))
    m.spheroid("skin_dark", (0.0, 5.8, 0.48), (0.08, 0.12, 0.08), segs=6, rings=4)
    # the sword in the right hand, point up
    hand = (1.55, 2.72, 0.48)
    m.tube("leather_dark", (hand[0], 2.3, hand[2]), (hand[0], 3.05, hand[2]), 0.09, 0.09, segs=6)
    m.spheroid("gold", (hand[0], 2.22, hand[2]), (0.13, 0.13, 0.13), segs=8, rings=6)
    m.box("steel_dark", (hand[0], 3.1, hand[2]), (0.95, 0.13, 0.18))
    m.blade("steel", (hand[0], 3.15, hand[2]), (hand[0], 6.1, hand[2]), 0.34, 0.08, 0.45)
    return m


BUILDERS = {"warrior": ("heros", warrior)}
BUILDERS.update({name: ("heros", build) for name, build in F.HEROES.items()})
BUILDERS.update({name: ("monsters", build) for name, build in F.MONSTERS.items()})
BUILDERS.update({name: ("world", build) for name, build in F.WORLD.items()})
BUILDERS.update({name: ("furniture", build) for name, build in FU.FURNITURE.items()})
BUILDERS.update({name: ("overlays", build) for name, build in FU.OVERLAYS.items()})
BUILDERS.update({name: ("other", build) for name, build in FU.OTHER.items()})
# the triggered traps' models in models/world (models.dat); "world_" keeps their keys apart
BUILDERS.update({"world_" + name: ("world", build) for name, build in FU.TRAPS.items()})


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--only", nargs="*", help="names to make (" + ", ".join(BUILDERS) + ")")
    parser.add_argument("--preview", help="also render previews into this folder")
    args = parser.parse_args()
    for name in args.only or list(BUILDERS):
        folder, build = BUILDERS[name]
        model = build()
        skin = model.write_obj(os.path.join(MODELS, folder))
        lo, hi = model.bounds()
        print(f"wrote models/{folder}/{model.name}.obj: {model.triangle_count()} triangles, "
              f"x {lo[0]:.1f}..{hi[0]:.1f} y {lo[1]:.1f}..{hi[1]:.1f} z {lo[2]:.1f}..{hi[2]:.1f}")
        if args.preview:
            M.preview(model, skin, os.path.join(args.preview, name + ".png"))


if __name__ == "__main__":
    main()
