"""Torchdelve's figures (heroes and monsters) and the start field, built with modellib.

Every figure stands on y = 0 at the field's centre and looks towards +z; a
field is 8 x 8, so a figure stays within about 3.5 of its centre. humanoid()
builds the common body (legs, hips, chest, arms, neck, head) from proportions
of its height; each figure adds its own clothes, gear and features.

The file names follow the game's data (models/monsters/<name>.obj); among them:
  bogbrute      -> the Bogbrute: hunched, a mossy hump, a club and a tail
  ashenknight   -> the Ashen Knight: full plate, a red tabard, a greatsword
  duskcaller    -> the Duskcaller: a hooded sorcerer with a glowing orb and staff
"""

import math

import modellib as M
from modellib import add, mul, sub, norm


def palette(**extra):
    """The shared colours plus a figure's own."""
    base = {
        "skin": (190, 136, 100), "skin_dark": (150, 100, 72),
        "leather": (96, 62, 38), "leather_dark": (54, 36, 24),
        "steel": (176, 178, 184), "steel_dark": (88, 90, 96),
        "gold": (204, 162, 72), "eye": (24, 18, 16), "wood": (104, 70, 40),
        "cloth": (110, 90, 70),
    }
    base.update(extra)
    return base


def humanoid(m, H, sw=None, bulk=1.0, hunch=0.0, skin="skin", head_skin=None, legs="cloth",
             boots="leather_dark", torso="cloth", upper_arm=None, forearm=None, hands=None,
             hand_r=None, hand_l=None, head_scale=1.0, robe=False, eyes="eye", neck=True):
    """The common body; returns the places of the hands and the head (centre, radius)."""
    k = H / 6.5  # the warrior's proportions
    sw = sw if sw is not None else 1.2 * k * bulk
    upper_arm = upper_arm or skin
    forearm = forearm or upper_arm
    hands = hands or skin
    head_skin = head_skin or skin
    hz = hunch * H  # how far the upper body leans forward
    hip_y, knee_y, ankle_y = 3.05 * k, 1.7 * k, 0.3 * k
    leg_x = 0.48 * k * bulk
    if not robe:
        right = [
            m.spheroid(boots, (leg_x, 0.2 * k, 0.2 * k), (0.32 * k * bulk, 0.22 * k, 0.48 * k)),
            m.tube(boots, (leg_x, 0.0, 0.0), (leg_x, 1.05 * k, 0.0), 0.38 * k * bulk, 0.34 * k * bulk),
            m.tube(legs, (leg_x, 1.0 * k, 0.0), (leg_x * 1.04, knee_y + 0.5 * k, 0.03 * k), 0.31 * k * bulk, 0.36 * k * bulk),
            m.tube(legs, (leg_x * 1.04, knee_y + 0.45 * k, 0.03 * k), (leg_x * 1.08, hip_y, 0.0), 0.37 * k * bulk, 0.46 * k * bulk),
        ]
        m.mirror_x(right)
        m.spheroid(legs, (0.0, hip_y, 0.0), (0.95 * k * bulk, 0.45 * k, 0.62 * k * bulk))
    # belly and chest
    m.spheroid(torso, (0.0, 4.15 * k, hz * 0.5), (0.98 * k * bulk, 1.05 * k, 0.66 * k * bulk))
    m.spheroid(torso, (0.0, 4.7 * k, hz * 0.8 + 0.05 * k), (1.1 * k * bulk, 0.66 * k, 0.68 * k * bulk))
    # arms
    sh_y = 4.92 * k
    hand_r = hand_r or (sw + 0.33 * k, 2.75 * k, 0.45 * k + hz)
    hand_l = hand_l or (-hand_r[0], hand_r[1], hand_r[2])
    for side, hand in ((1, hand_r), (-1, hand_l)):
        shoulder = (side * sw, sh_y, hz * 0.8)
        elbow = (side * (sw + 0.32 * k * bulk), 3.85 * k, hz * 0.8 + 0.05 * k)
        # bend the elbow towards the hand if it's far forward or high
        elbow = add(mul(elbow, 0.6), mul(add(mul(shoulder, 0.5), mul(hand, 0.5)), 0.4))
        m.spheroid(upper_arm, shoulder, (0.48 * k * bulk, 0.44 * k, 0.46 * k * bulk))
        m.tube(upper_arm, add(shoulder, (0, -0.1 * k, 0)), elbow, 0.36 * k * bulk, 0.3 * k * bulk, segs=9)
        m.spheroid(forearm, elbow, (0.3 * k * bulk, 0.3 * k * bulk, 0.3 * k * bulk), segs=8, rings=6)
        wrist = add(hand, mul(norm(sub(elbow, hand)), 0.22 * k))
        m.tube(forearm, elbow, wrist, 0.29 * k * bulk, 0.23 * k * bulk, segs=9)
        m.spheroid(hands, hand, (0.23 * k * bulk, 0.25 * k, 0.23 * k * bulk), segs=8, rings=6)
    # neck and head
    head_r = 0.47 * k * head_scale
    head = (0.0, 5.86 * k + (head_scale - 1) * 0.3 * k, hz + 0.04 * k)
    if neck:
        m.tube(head_skin, (0.0, 5.1 * k, hz * 0.9), (0.0, 5.55 * k, hz + 0.02 * k), 0.32 * k * bulk, 0.28 * k)
    m.spheroid(head_skin, head, (head_r * 0.9, head_r * 1.06, head_r * 0.96), segs=14, rings=10)
    if eyes:
        for x in (-0.34, 0.34):
            m.spheroid(eyes, (head[0] + x * head_r, head[1] + 0.12 * head_r, head[2] + 0.88 * head_r),
                       (0.13 * head_r, 0.11 * head_r, 0.06 * head_r), segs=6, rings=4)
    return {"hand_r": hand_r, "hand_l": hand_l, "head": head, "head_r": head_r, "k": k, "hz": hz}


def arc(m, swatch, center, radius, a0, a1, thick, plane_u, plane_v, segs=8):
    """A bent rod (a bow, a horn) along a circle arc in the plane (u, v)."""
    pts = [add(center, add(mul(plane_u, math.cos(a) * radius), mul(plane_v, math.sin(a) * radius)))
           for a in (a0 + (a1 - a0) * s / segs for s in range(segs + 1))]
    for s in range(segs):
        m.tube(swatch, pts[s], pts[s + 1], thick, thick, segs=6, caps=(s in (0, segs - 1)))
        if s:
            m.spheroid(swatch, pts[s], (thick, thick, thick), segs=6, rings=4)
    return pts


# ---------------- heroes ----------------

def dwarf():
    m = M.Model("dwarf", palette(hair=(150, 92, 52), mail=(118, 120, 126), cloth=(84, 62, 46)))
    b = humanoid(m, 5.0, bulk=1.4, torso="mail", legs="cloth", upper_arm="mail", forearm="skin",
                 hand_r=(1.7, 2.3, 0.55), hand_l=(-1.6, 2.15, 0.45), head_scale=1.12)
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # belt
    m.tube("leather", (0, 3.2 * k, 0), (0, 3.55 * k, 0), 1.3 * k * 1.4 * 0.75, 1.3 * k * 1.4 * 0.75, segs=14, squash=0.72)
    m.box("gold", (0, 3.38 * k, 0.82 * 1.4 * k * 0.72 + 0.05), (0.42, 0.32, 0.1))
    # helmet with rim and nose guard
    m.spheroid("steel", (hd[0], hd[1] + 0.18 * hr, hd[2] - 0.02), (hr * 1.02, hr * 0.8, hr * 1.04), segs=14, rings=10)
    m.torus("steel_dark", (hd[0], hd[1] + 0.1 * hr, hd[2] - 0.02), (0, 1, 0), hr * 1.0, 0.07, segs=16, sides=5)
    m.box("steel_dark", (hd[0], hd[1] - 0.05 * hr, hd[2] + hr * 0.95), (0.12, hr * 0.7, 0.08))
    m.tube("steel_dark", (hd[0], hd[1] + 0.95 * hr, hd[2]), (hd[0], hd[1] + 1.35 * hr, hd[2]), 0.09, 0.0, segs=6)
    # the big beard with two braids
    chest_z = 0.68 * k * 1.4 + 0.12  # the beard lies on the chest
    m.spheroid("hair", (hd[0], hd[1] - 0.95 * hr, hd[2] + 0.55 * hr), (0.78 * hr, 0.95 * hr, 0.5 * hr), segs=12, rings=8)
    m.spheroid("hair", (hd[0], hd[1] - 1.75 * hr, chest_z - 0.2), (0.62 * hr, 0.75 * hr, 0.35 * hr), segs=10, rings=7)
    for x in (-0.28, 0.28):
        m.tube("hair", (x * hr, hd[1] - 1.9 * hr, chest_z), (x * hr * 1.1, hd[1] - 2.8 * hr, chest_z + 0.02),
               0.15 * hr, 0.09 * hr, segs=6)
        m.spheroid("gold", (x * hr * 1.1, hd[1] - 2.82 * hr, chest_z + 0.02), (0.11 * hr,) * 3, segs=6, rings=4)
    m.spheroid("hair", (hd[0], hd[1] - 0.05 * hr, hd[2] + 0.82 * hr), (0.5 * hr, 0.14 * hr, 0.12 * hr), segs=8, rings=4)
    # the battle axe: a haft in both hands, a crescent blade at the top
    hr_ = b["hand_r"]
    top = (hr_[0] - 0.25, 4.55, hr_[2] + 0.35)
    bottom = (hr_[0] + 0.05, 1.0, hr_[2] - 0.1)
    m.tube("wood", bottom, top, 0.1, 0.1, segs=6)
    m.spheroid("steel_dark", top, (0.13, 0.13, 0.13), segs=6, rings=4)
    blade = [(0.0, -0.2), (0.35, -0.35), (0.7, -0.75), (0.85, -0.2), (0.88, 0.25), (0.75, 0.6), (0.35, 0.3), (0.0, 0.22)]
    m.slab("steel", blade, add(top, (0.1, -0.38, 0.02)), (1, 0, 0), (0, 1, 0), 0.08)
    m.slab("steel_dark", [(0.0, -0.22), (0.12, -0.15), (0.12, 0.15), (0.0, 0.22)], add(top, (0.06, -0.38, 0.02)), (1, 0, 0), (0, 1, 0), 0.12)
    return m


def ranger():
    m = M.Model("ranger", palette(hair=(222, 196, 128), cloak=(52, 88, 58), cloak_dark=(36, 60, 40),
                               leather=(110, 80, 50), skin=(214, 176, 142)))
    b = humanoid(m, 6.6, bulk=0.78, torso="leather", legs="cloak_dark", upper_arm="cloak", forearm="leather",
                 hand_r=(1.05, 3.0, 0.5), hand_l=(-1.15, 3.9, 0.95), head_scale=0.95)
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # the cloak: from the shoulders down to the calves, open in front
    m.tube("cloak", (0, 5.0 * k, -0.25), (0, 1.0 * k, -0.55), 0.95 * k, 1.25 * k, segs=14, caps=False, squash=0.6)
    m.spheroid("cloak", (0, 5.1 * k, -0.15), (1.05 * k, 0.4 * k, 0.6 * k))
    # the hood, down on the back; long hair
    m.spheroid("cloak_dark", (hd[0], hd[1] - 0.6 * hr, hd[2] - 0.9 * hr), (0.9 * hr, 0.6 * hr, 0.6 * hr))
    m.spheroid("hair", (hd[0], hd[1] + 0.15 * hr, hd[2] - 0.12 * hr), (hr * 0.98, hr * 1.0, hr * 1.0), segs=14, rings=10)
    m.tube("hair", (hd[0], hd[1], hd[2] - 0.6 * hr), (hd[0], hd[1] - 2.4 * hr, hd[2] - 0.95 * hr), 0.6 * hr, 0.25 * hr, segs=8, squash=0.5)
    # pointed ears
    for side in (1, -1):
        m.slab("skin", [(0, -0.12), (0.5, 0.35), (0.05, 0.12)], (side * hr * 0.82, hd[1] + 0.05, hd[2]),
               (side, 0.35, -0.3), (0, 1, 0), 0.05)
    # the belt
    m.tube("leather_dark", (0, 3.25 * k, 0), (0, 3.45 * k, 0), 0.8 * k, 0.8 * k, segs=12, squash=0.7)
    # the longbow in the left hand, the string, the quiver on the back
    hl = b["hand_l"]
    bow_c = add(hl, (0.0, 0.0, -2.2))
    pts = arc(m, "wood", bow_c, 2.25, math.radians(52), math.radians(128), 0.07, (0, 1, 0), (0, 0, 1), segs=10)
    m.tube("eye", pts[0], pts[-1], 0.02, 0.02, segs=4)
    m.tube("leather_dark", (0.45, 5.2 * k, -0.85), (-0.35, 3.6 * k, -0.85), 0.24, 0.2, segs=8)
    for dx in (-0.08, 0.06, 0.16):
        m.tube("wood", (0.45 + dx, 5.1 * k, -0.85), (0.55 + dx, 5.85 * k, -0.9), 0.03, 0.03, segs=4)
        m.slab("steel", [(0, 0), (0.08, 0.14), (0, 0.26), (-0.08, 0.14)], (0.55 + dx, 5.85 * k, -0.9), (1, 0, 0), (0, 1, 0), 0.02)
    return m


def mage():
    m = M.Model("mage", palette(robe=(48, 56, 112), robe_dark=(32, 36, 78), hair=(214, 214, 206),
                                  glow=(255, 226, 150), skin=(198, 150, 118)))
    b = humanoid(m, 6.8, bulk=0.9, robe=True, torso="robe", upper_arm="robe", forearm="robe_dark",
                 hand_r=(1.35, 3.6, 0.6), hand_l=(-1.25, 2.9, 0.55))
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # the long robe: a cone from the waist to the floor, a sash
    m.tube("robe", (0, 3.6 * k, 0), (0, 0.0, 0), 0.95 * k, 1.5 * k, segs=16, squash=0.8)
    m.tube("robe_dark", (0, 0.0, 0), (0, 0.22, 0), 1.52 * k, 1.5 * k, segs=16, squash=0.8)
    m.tube("gold", (0, 3.4 * k, 0), (0, 3.7 * k, 0), 0.96 * k, 0.96 * k, segs=14, squash=0.75)
    # wide sleeves
    for hand in (b["hand_r"], b["hand_l"]):
        sx = 1 if hand[0] > 0 else -1
        m.tube("robe_dark", add(hand, (-sx * 0.15, 0.4, -0.2)), add(hand, (-sx * 0.05, 0.05, 0.05)), 0.32, 0.42, segs=10, caps=False)
    # long white beard and hair, the pointed hat
    m.tube("hair", (hd[0], hd[1] - 0.2 * hr, hd[2] + 0.55 * hr), (hd[0], hd[1] - 2.6 * hr, hd[2] + 0.65 * hr),
           0.6 * hr, 0.05 * hr, segs=10, squash=0.55)
    m.spheroid("hair", (hd[0], hd[1] + 0.1 * hr, hd[2] - 0.25 * hr), (hr * 0.98, hr * 0.95, hr * 0.95), segs=12, rings=8)
    m.torus("robe_dark", (hd[0], hd[1] + 0.55 * hr, hd[2]), (0, 1, 0), hr * 1.45, 0.16, segs=18, sides=5)
    m.tube("robe", (hd[0], hd[1] + 0.5 * hr, hd[2]), (hd[0] - 0.25, hd[1] + 3.2 * hr, hd[2] - 0.4), hr * 1.12, 0.0, segs=12)
    # the staff with its glowing crystal
    hrr = b["hand_r"]
    m.tube("wood", (hrr[0] + 0.05, 0.0, hrr[2] + 0.1), (hrr[0] - 0.05, 7.2, hrr[2] - 0.05), 0.09, 0.07, segs=6)
    for a in range(3):
        ang = math.tau * a / 3
        m.tube("wood", (hrr[0] - 0.05, 7.1, hrr[2] - 0.05),
               (hrr[0] - 0.05 + 0.3 * math.cos(ang), 7.7, hrr[2] - 0.05 + 0.3 * math.sin(ang)), 0.05, 0.03, segs=4)
    m.spheroid("glow", (hrr[0] - 0.05, 7.62, hrr[2] - 0.05), (0.22, 0.3, 0.22), segs=8, rings=6)
    return m


# ---------------- monsters ----------------

def goblin():
    m = M.Model("goblin", palette(skin=(118, 132, 64), skin_dark=(84, 96, 44), cloth=(96, 74, 50),
                                  eye=(230, 200, 60)))
    b = humanoid(m, 4.3, bulk=0.85, hunch=0.06, torso="skin", legs="skin", boots="skin_dark",
                 hand_r=(1.2, 1.9, 0.7), hand_l=(-1.15, 1.75, 0.55), head_scale=1.45)
    k, hd, hr = b["k"], b["head"], b["head_r"]
    m.tube("cloth", (0, 2.95 * k, 0), (0, 2.1 * k, 0.05), 0.86 * k, 1.0 * k, segs=10, squash=0.75)
    # long ears, a big nose, teeth
    for side in (1, -1):
        m.slab("skin_dark", [(0, -0.16), (0.62, 0.22), (0.08, 0.2)], (side * hr * 0.85, hd[1] + 0.1, hd[2] - 0.05),
               (side, 0.25, -0.2), (0, 1, 0), 0.06)
    m.tube("skin_dark", (0, hd[1] - 0.05, hd[2] + hr * 0.85), (0, hd[1] - 0.25, hd[2] + hr * 1.35), 0.14, 0.05, segs=6)
    m.box("steel", (0, hd[1] - 0.48 * hr, hd[2] + hr * 0.86), (0.4 * hr, 0.08, 0.06))
    # a jagged dagger
    hrr = b["hand_r"]
    m.tube("leather_dark", add(hrr, (0, -0.25, 0)), add(hrr, (0, 0.25, 0.05)), 0.07, 0.07, segs=5)
    m.blade("steel_dark", add(hrr, (0, 0.25, 0.08)), add(hrr, (0, 1.55, 0.5)), 0.26, 0.06, 0.35)
    return m


def skeleton():
    m = M.Model("skeleton", palette(bone=(214, 204, 174), bone_dark=(150, 140, 116), eye=(10, 8, 8),
                                    rust=(124, 78, 52)))
    k = 6.2 / 6.5
    B = "bone"
    # legs
    right = []
    for (p0, p1, r) in [((0.42, 0.05, 0.2), (0.42, 0.15, -0.1), 0.12), ((0.42, 0.15, 0.0), (0.44, 1.7 * k, 0.05), 0.1),
                        ((0.44, 1.7 * k, 0.05), (0.42, 3.0 * k, 0.0), 0.12)]:
        right.append(m.tube(B, p0, p1, r, r * 0.85, segs=6))
    right.append(m.spheroid(B, (0.44, 1.7 * k, 0.07), (0.16, 0.16, 0.16), segs=6, rings=4))
    right.append(m.spheroid("bone_dark", (0.42, 0.12, 0.1), (0.2, 0.12, 0.35), segs=6, rings=4))
    m.mirror_x(right)
    # pelvis, spine, ribs
    m.spheroid(B, (0, 3.1 * k, 0), (0.62, 0.3, 0.38))
    m.tube("bone_dark", (0, 3.2 * k, -0.15), (0, 5.4 * k, -0.15), 0.1, 0.1, segs=6)
    for i, (y, r) in enumerate([(4.05, 0.62), (4.4, 0.72), (4.75, 0.76), (5.08, 0.7)]):
        m.torus(B, (0, y * k, 0.1), (0, 1, 0), r, 0.055, segs=14, sides=4, squash=0.72)
    m.tube(B, (-0.95, 5.25 * k, -0.05), (0.95, 5.25 * k, -0.05), 0.08, 0.08, segs=6)
    # arms
    for side, hand in ((1, (1.15, 2.85 * k, 0.55)), (-1, (-1.1, 3.0 * k, 0.35))):
        sh = (side * 0.98, 5.2 * k, -0.05)
        el = (side * 1.18, 4.0 * k, 0.05)
        m.spheroid(B, sh, (0.17, 0.17, 0.17), segs=6, rings=4)
        m.tube(B, sh, el, 0.09, 0.08, segs=6)
        m.spheroid(B, el, (0.12, 0.12, 0.12), segs=6, rings=4)
        m.tube(B, el, hand, 0.08, 0.06, segs=6)
        m.spheroid("bone_dark", hand, (0.14, 0.18, 0.14), segs=6, rings=4)
    # the skull with dark sockets and a jaw
    hd = (0, 5.95 * k, 0.05)
    m.tube("bone_dark", (0, 5.35 * k, -0.1), hd, 0.09, 0.09, segs=6)
    m.spheroid(B, hd, (0.42, 0.46, 0.46), segs=12, rings=9)
    for x in (-0.15, 0.15):
        m.spheroid("eye", (x, hd[1] + 0.03, hd[2] + 0.36), (0.11, 0.1, 0.06), segs=6, rings=4)
    m.box(B, (0, hd[1] - 0.38, hd[2] + 0.18), (0.42, 0.16, 0.36))
    m.box("eye", (0, hd[1] - 0.28, hd[2] + 0.37), (0.3, 0.04, 0.02))
    # a rusty sword
    hand = (1.15, 2.85 * k, 0.55)
    m.box("rust", add(hand, (0, 0.28, 0)), (0.6, 0.09, 0.12))
    m.blade("rust", add(hand, (0, 0.3, 0.02)), add(hand, (0.0, 2.6, 0.45)), 0.26, 0.06, 0.35)
    return m


def zombie():
    m = M.Model("zombie", palette(skin=(130, 140, 112), skin_dark=(90, 98, 80), cloth=(78, 70, 76),
                                  legs=(70, 62, 52), eye=(200, 210, 150)))
    b = humanoid(m, 6.0, bulk=0.95, hunch=0.1, torso="cloth", legs="legs", boots="skin_dark",
                 upper_arm="cloth", forearm="skin", hand_r=(0.95, 4.4, 2.2), hand_l=(-1.0, 3.9, 1.95))
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # torn shirt edge, exposed belly, a wound
    m.torus("cloth", (0, 3.55 * k, 0.4), (0, 1, 0), 0.95 * k, 0.08, segs=14, sides=4, squash=0.7)
    m.spheroid("skin_dark", (0.3, 3.95 * k, 0.62 * k + 0.35), (0.25, 0.3, 0.12), segs=8, rings=5)
    # sunken eye sockets, the jaw hanging open
    for x in (-0.34, 0.34):
        m.spheroid("skin_dark", (x * hr, hd[1] + 0.12 * hr, hd[2] + 0.8 * hr), (0.24 * hr, 0.22 * hr, 0.1 * hr), segs=6, rings=4)
    m.box("eye", (0, hd[1] - 0.55 * hr, hd[2] + 0.8 * hr), (0.45 * hr, 0.18 * hr, 0.08))
    m.spheroid("skin_dark", (0.25 * hr, hd[1] + 0.8 * hr, hd[2]), (0.4 * hr, 0.25 * hr, 0.4 * hr), segs=8, rings=5)
    return m


def orc():
    m = M.Model("orc", palette(skin=(98, 118, 72), skin_dark=(66, 82, 48), cloth=(74, 58, 44),
                               plate=(92, 86, 80), eye=(220, 60, 30), bone=(220, 210, 180)))
    b = humanoid(m, 6.8, bulk=1.3, hunch=0.05, torso="skin", legs="cloth", boots="leather_dark",
                 upper_arm="skin", hand_r=(1.95, 3.0, 0.8), hand_l=(-1.85, 2.7, 0.5), head_scale=1.08)
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # crude armour: a breastplate, a spiked pauldron, a belt
    m.spheroid("plate", (0, 4.6 * k, 0.12 + b["hz"] * 0.8), (1.15 * k * 1.3 * 0.92, 0.72 * k, 0.62 * k * 1.3), segs=12, rings=7)
    m.spheroid("plate", (1.55 * k, 5.05 * k, 0.3), (0.7, 0.42, 0.66))
    for a in range(3):
        m.tube("steel_dark", (1.4 * k + 0.25 * a, 5.3 * k, 0.3), (1.45 * k + 0.32 * a, 5.95 * k, 0.3), 0.12, 0.0, segs=5)
    m.tube("leather", (0, 3.15 * k, 0), (0, 3.5 * k, 0), 1.2 * k * 1.3 * 0.8, 1.2 * k * 1.3 * 0.8, segs=14, squash=0.75)
    # tusks, a heavy brow, small ears
    for x in (-0.3, 0.3):
        m.tube("bone", (x * hr, hd[1] - 0.45 * hr, hd[2] + 0.8 * hr), (x * hr * 1.3, hd[1] + 0.05 * hr, hd[2] + 1.0 * hr), 0.08, 0.0, segs=5)
        m.slab("skin_dark", [(0, -0.12), (0.45, 0.25), (0.05, 0.15)], (x / 0.3 * hr * 0.85, hd[1] + 0.05, hd[2] - 0.05),
               (x / 0.3, 0.2, -0.3), (0, 1, 0), 0.06)
    m.box("skin_dark", (0, hd[1] + 0.35 * hr, hd[2] + 0.78 * hr), (1.0 * hr, 0.18 * hr, 0.3 * hr))
    # the cleaver
    hrr = b["hand_r"]
    m.tube("leather_dark", add(hrr, (0, -0.3, 0)), add(hrr, (0, 0.4, 0)), 0.09, 0.09, segs=6)
    m.slab("steel_dark", [(-0.3, 0), (0.4, 0), (0.55, 2.0), (-0.3, 1.7)], add(hrr, (0, 0.4, 0.05)), (0, 0, 1), (0, 1, 0), 0.09)
    return m


def bogbrute():
    """The Bogbrute: hunched, a mossy hump, a club and a tail."""
    m = M.Model("bogbrute", palette(skin=(96, 112, 88), skin_dark=(64, 76, 58), moss=(70, 96, 48),
                                 cloth=(70, 56, 40), eye=(230, 170, 50)))
    b = humanoid(m, 6.2, bulk=1.5, hunch=0.16, torso="skin", legs="skin", boots="skin_dark",
                 hand_r=(2.0, 1.5, 1.0), hand_l=(-2.0, 1.7, 0.9), head_scale=0.85, neck=False)
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # the hump with moss, a loincloth, a tail
    m.spheroid("moss", (0, 5.0 * k, -0.35), (1.25 * k * 1.3, 0.75 * k, 0.75 * k))
    m.tube("cloth", (0, 3.0 * k, 0.05), (0, 2.0 * k, 0.15), 1.3 * k, 1.45 * k, segs=12, squash=0.7)
    m.tube("skin_dark", (0, 2.9 * k, -0.7), (0, 0.6, -2.6), 0.45, 0.08, segs=8)
    # a heavy brow, a wide jaw
    m.box("skin_dark", (0, hd[1] + 0.3 * hr, hd[2] + 0.75 * hr), (1.2 * hr, 0.25 * hr, 0.35 * hr))
    m.spheroid("skin", (0, hd[1] - 0.6 * hr, hd[2] + 0.35 * hr), (0.85 * hr, 0.45 * hr, 0.7 * hr), segs=10, rings=6)
    # the club
    hrr = b["hand_r"]
    m.tube("wood", add(hrr, (0, 0.3, 0)), add(hrr, (0.1, -1.2, 1.2)), 0.12, 0.26, segs=7)
    m.spheroid("wood", add(hrr, (0.1, -1.3, 1.3)), (0.38, 0.38, 0.38), segs=8, rings=6)
    return m


def mummy():
    m = M.Model("mummy", palette(wrap=(196, 180, 140), wrap_dark=(140, 124, 92), eye=(120, 220, 200)))
    b = humanoid(m, 6.3, bulk=0.9, hunch=0.05, skin="wrap", torso="wrap", legs="wrap", boots="wrap_dark",
                 hand_r=(1.0, 4.2, 1.8), hand_l=(-1.25, 3.0, 0.8), eyes=None)
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # bandages around the body and the limbs (rings of the darker cloth)
    for y in (1.2, 1.9, 2.6, 3.6, 4.2, 4.8):
        r = 0.74 * k if y > 3.3 else 0.37 * k
        if y > 3.3:
            # a band around the body: just inside the torso's outline, so it looks wrapped
            m.torus("wrap_dark", (0, y * k, b["hz"] * 0.6 + 0.04), (0, 1, 0.12), r, 0.09, segs=16, sides=5, squash=0.72)
        else:
            for x in (-0.48 * k * 0.9, 0.48 * k * 0.9):
                m.torus("wrap_dark", (x, y * k, 0.02), (0, 1, 0.15), r, 0.04, segs=10, sides=4)
    for y in (0.2, -0.35):
        m.torus("wrap_dark", (hd[0], hd[1] + y * hr, hd[2]), (0, 1, 0.3), hr * 0.92, 0.04, segs=14, sides=4)
    # glowing eyes in a slit
    m.box("eye", (0, hd[1] + 0.1 * hr, hd[2] + 0.9 * hr), (0.7 * hr, 0.12 * hr, 0.05))
    # a loose bandage hanging from the raised arm
    hrr = b["hand_r"]
    m.slab("wrap_dark", [(0, 0), (0.18, 0), (0.25, -1.3), (0.08, -1.25)], add(hrr, (-0.1, -0.15, -0.3)), (1, 0, 0), (0, 1, 0), 0.03)
    return m


def ashenknight():
    """The Ashen Knight: full plate, a red tabard, a greatsword."""
    m = M.Model("ashenknight", palette(plate=(70, 72, 80), plate_dark=(40, 40, 46), red=(116, 30, 26),
                                        eye=(255, 90, 40)))
    b = humanoid(m, 7.0, bulk=1.25, torso="plate", legs="plate", boots="plate_dark", upper_arm="plate",
                 forearm="plate_dark", hands="plate_dark", head_skin="plate", hand_r=(0.6, 3.2, 1.3),
                 hand_l=(-0.4, 3.35, 1.3), eyes=None)
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # the tabard and the belt
    m.slab("red", [(-0.55, 0), (0.55, 0), (0.45, -1.9), (-0.45, -1.9)], (0, 3.6 * k, 0.72 * k * 1.25 + 0.05), (1, 0, 0), (0, 1, 0), 0.05)
    m.tube("plate_dark", (0, 3.25 * k, 0), (0, 3.6 * k, 0), 1.12 * k * 1.25 * 0.8, 1.12 * k * 1.25 * 0.8, segs=14, squash=0.72)
    # big spiked pauldrons
    for side in (1, -1):
        m.spheroid("plate_dark", (side * 1.45 * k, 5.1 * k, 0.05), (0.78, 0.5, 0.72))
        m.tube("steel", (side * 1.5 * k, 5.45 * k, 0.05), (side * 1.85 * k, 6.25 * k, 0.0), 0.14, 0.0, segs=5)
    # the helmet: a crest, a visor slit with a glow, horns
    m.spheroid("plate", (hd[0], hd[1] + 0.05 * hr, hd[2]), (hr * 1.02, hr * 1.12, hr * 1.05), segs=14, rings=10)
    m.box("eye", (0, hd[1] + 0.12 * hr, hd[2] + 0.98 * hr), (0.9 * hr, 0.1 * hr, 0.06))
    m.slab("plate_dark", [(-0.08, 0), (0.08, 0), (0.0, 0.7)], (0, hd[1] + 0.95 * hr, hd[2] + 0.1), (0, 0, 1), (0, 1, 0), 0.08)
    for side in (1, -1):
        arc(m, "plate_dark", (side * hr * 1.5, hd[1] + 0.3 * hr, hd[2]), hr * 0.65, math.pi * 0.95, math.pi * 0.35, 0.09,
            (side, 0, 0), (0, 1, 0), segs=5)
    # the greatsword, held point down in front
    grip = add(add(b["hand_r"], b["hand_l"]), (0, 0, 0))
    grip = mul(grip, 0.5)
    m.tube("leather_dark", add(grip, (0, 0.55, 0)), add(grip, (0, -0.45, 0)), 0.09, 0.09, segs=6)
    m.spheroid("steel_dark", add(grip, (0, 0.65, 0)), (0.15, 0.15, 0.15), segs=6, rings=4)
    m.box("steel_dark", add(grip, (0, -0.5, 0)), (1.2, 0.14, 0.2))
    m.blade("steel", add(grip, (0, -0.55, 0.02)), add(grip, (0, -3.15, 0.25)), 0.4, 0.09, 0.5)
    return m


def gargoyle():
    m = M.Model("gargoyle", palette(stone=(116, 114, 108), stone_dark=(78, 76, 72), wing=(92, 90, 86),
                                    eye=(240, 140, 40)))
    b = humanoid(m, 5.9, bulk=1.1, hunch=0.14, skin="stone", torso="stone", legs="stone", boots="stone_dark",
                 hand_r=(1.55, 2.2, 1.3), hand_l=(-1.55, 2.2, 1.3), head_scale=0.95)
    k, hd, hr = b["k"], b["head"], b["head_r"]
    # bat wings from the back
    for side in (1, -1):
        wing = [(0, 0), (1.2, 1.2), (2.6, 1.9), (3.1, 0.9), (2.5, 0.2), (2.6, -0.8), (1.7, -0.6), (1.6, -1.7), (0.8, -1.0)]
        m.slab("wing", wing, (side * 0.4, 4.9 * k, -0.6), (side * 0.92, 0.0, -0.38), (0, 1, 0), 0.07)
        m.tube("stone_dark", (side * 0.4, 4.9 * k, -0.6), (side * (0.4 + 2.6 * 0.92), 4.9 * k + 1.9, -0.6 - 2.6 * 0.38), 0.12, 0.05, segs=5)
    # horns, claws, a tail
    for side in (1, -1):
        arc(m, "stone_dark", (side * hr * 0.6, hd[1] + 0.7 * hr, hd[2] - 0.2), hr * 0.8, math.pi * 0.0, math.pi * 0.55, 0.1,
            (side, 0, 0), (0, 0.6, -0.8), segs=4)
        hand = b["hand_r"] if side > 0 else b["hand_l"]
        for d in (-0.12, 0.0, 0.12):
            m.tube("stone_dark", add(hand, (d, -0.1, 0.1)), add(hand, (d * 1.6, -0.45, 0.4)), 0.05, 0.0, segs=4)
    m.tube("stone_dark", (0, 3.0 * k, -0.6), (0.4, 0.4, -2.4), 0.3, 0.06, segs=7)
    m.slab("stone_dark", [(0, 0), (0.25, 0.35), (0, 0.6), (-0.25, 0.35)], (0.4, 0.4, -2.45), (1, 0, 0), (0, 0, -1), 0.06)
    # a snout with fangs
    m.spheroid("stone", (0, hd[1] - 0.35 * hr, hd[2] + 0.7 * hr), (0.55 * hr, 0.4 * hr, 0.5 * hr), segs=8, rings=6)
    for x in (-0.25, 0.25):
        m.tube("eye", (x * hr, hd[1] - 0.55 * hr, hd[2] + 1.05 * hr), (x * hr, hd[1] - 0.85 * hr, hd[2] + 1.05 * hr), 0.05, 0.0, segs=4)
    return m


def duskcaller():
    """The Duskcaller: a hooded sorcerer with a glowing orb and a staff."""
    m = M.Model("duskcaller", palette(robe=(46, 30, 54), robe_dark=(28, 18, 34), skin=(176, 168, 160),
                                        glow=(150, 255, 140), eye=(150, 255, 140), trim=(140, 110, 60)))
    b = humanoid(m, 6.8, bulk=0.9, robe=True, torso="robe", upper_arm="robe", forearm="robe_dark",
                 hand_r=(1.3, 3.8, 0.7), hand_l=(-1.05, 4.3, 1.4), eyes="eye")
    k, hd, hr = b["k"], b["head"], b["head_r"]
    m.tube("robe", (0, 3.7 * k, 0), (0, 0.0, 0), 0.95 * k, 1.6 * k, segs=16, squash=0.8)
    m.tube("trim", (0, 0.0, 0), (0, 0.2, 0), 1.62 * k, 1.6 * k, segs=16, squash=0.8)
    m.slab("trim", [(-0.12, 0), (0.12, 0), (0.12, -3.2), (-0.12, -3.2)], (0, 3.7 * k, 0.78 * k + 0.08), (1, 0, 0), (0, 1, 0), 0.04)
    for hand in (b["hand_r"], b["hand_l"]):
        sx = 1 if hand[0] > 0 else -1
        m.tube("robe_dark", add(hand, (-sx * 0.15, 0.4, -0.25)), add(hand, (-sx * 0.05, 0.05, 0.05)), 0.32, 0.45, segs=10, caps=False)
    # the deep hood over the head, its face in the shadow
    m.spheroid("robe_dark", (hd[0], hd[1] + 0.15 * hr, hd[2] - 0.15 * hr), (hr * 1.3, hr * 1.35, hr * 1.3), segs=14, rings=10)
    m.tube("robe_dark", (hd[0], hd[1] + 1.0 * hr, hd[2] - 0.6 * hr), (hd[0], hd[1] + 1.7 * hr, hd[2] - 1.3 * hr), 0.5 * hr, 0.0, segs=8)
    m.box("robe_dark", (hd[0], hd[1] - 0.1 * hr, hd[2] + 1.0 * hr), (1.3 * hr, 1.4 * hr, 0.08))
    for x in (-0.3, 0.3):
        m.spheroid("eye", (x * hr, hd[1] + 0.05 * hr, hd[2] + 1.08 * hr), (0.12 * hr, 0.08 * hr, 0.04), segs=6, rings=4)
    # a glow in the left hand, the staff with an orb
    m.spheroid("glow", add(b["hand_l"], (0, 0.45, 0.1)), (0.24, 0.24, 0.24), segs=8, rings=6)
    hrr = b["hand_r"]
    m.tube("robe_dark", (hrr[0], 0.0, hrr[2]), (hrr[0], 7.0, hrr[2]), 0.08, 0.07, segs=6)
    arc(m, "trim", (hrr[0], 7.25, hrr[2]), 0.3, -math.pi * 0.5, math.pi * 1.2, 0.05, (1, 0, 0), (0, 1, 0), segs=8)
    m.spheroid("glow", (hrr[0], 7.3, hrr[2]), (0.2, 0.2, 0.2), segs=8, rings=6)
    return m


def startpos():
    """The start field: a rune circle on the floor (the compiler places it 4 above the floor)."""
    m = M.Model("startpos", palette(rune=(255, 196, 90), stone=(60, 54, 48), ember=(230, 110, 40)))
    y = -3.96
    m.tube("stone", (0, y - 0.02, 0), (0, y + 0.05, 0), 3.1, 3.1, segs=24)
    m.torus("rune", (0, y + 0.06, 0), (0, 1, 0), 2.8, 0.09, segs=28, sides=4, squash=1.0)
    m.torus("ember", (0, y + 0.06, 0), (0, 1, 0), 2.2, 0.06, segs=24, sides=4)
    for i in range(8):
        a = math.tau * i / 8
        c = (math.cos(a) * 2.5, y + 0.07, math.sin(a) * 2.5)
        u = (math.cos(a + math.pi / 2), 0, math.sin(a + math.pi / 2))
        rune = [(-0.18, -0.12), (0.0, 0.2), (0.18, -0.12), (0.0, -0.02)]
        m.slab("rune", rune, c, u, (math.cos(a), 0, math.sin(a)), 0.03)
    # a star in the middle
    star = []
    for i in range(10):
        a = math.tau * i / 10
        r = 1.2 if i % 2 == 0 else 0.5
        star.append((math.cos(a) * r, math.sin(a) * r))
    m.slab("ember", star, (0, y + 0.07, 0), (1, 0, 0), (0, 0, 1), 0.03)
    return m


HEROES = {"dwarf": dwarf, "ranger": ranger, "mage": mage}
MONSTERS = {"goblin": goblin, "skeleton": skeleton, "zombie": zombie, "orc": orc, "bogbrute": bogbrute,
            "mummy": mummy, "ashenknight": ashenknight, "gargoyle": gargoyle, "duskcaller": duskcaller}
WORLD = {"startpos": startpos}
