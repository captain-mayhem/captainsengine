"""Building blocks for Torchdelve's low-poly models (Python, no numpy).

A model is built from parts (spheroids, tapered cylinders between two points,
boxes, prisms). Every vertex has its own normal from the shape, so curved
parts shade smoothly and the edges between parts stay sharp. Each part takes
its colour from a swatch of the model's skin: the skin is a 512 px texture of
8 x 8 swatches, each a painted colour with some grain. A part's UVs spread
over the inside of its swatch (a projection of the part), so the grain shows.

write_obj() writes name.obj, name.mtl and name_skin.png: the MTL's map_Kd
names the skin, and the engine's mesh uses its material's texture over any
texture the game sets (Engine/mesh/mesh.cpp). The OBJ loader is simple:
v, vt, vn, f with a/b/c, usemtl, mtllib, and every line ends in \\n.

Units: a field is 8 x 8, a hero about 6 high, y is up, the front is +z.
"""

import math
import os
import random

from PIL import Image, ImageDraw

import texlib as T

SKIN = 512
GRID = 8
CELL = SKIN // GRID
MARGIN = 0.18  # of a swatch, so that filtering doesn't bleed in the neighbours


# ---- vectors ----
def add(a, b): return (a[0] + b[0], a[1] + b[1], a[2] + b[2])
def sub(a, b): return (a[0] - b[0], a[1] - b[1], a[2] - b[2])
def mul(a, s): return (a[0] * s, a[1] * s, a[2] * s)
def dot(a, b): return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
def cross(a, b): return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])
def length(a): return math.sqrt(dot(a, a))


def norm(a):
    ln = length(a)
    return (a[0] / ln, a[1] / ln, a[2] / ln) if ln > 1e-9 else (0.0, 1.0, 0.0)


def basis(axis):
    """Two unit vectors perpendicular to axis (and to each other)."""
    a = norm(axis)
    helper = (0.0, 0.0, 1.0) if abs(a[2]) < 0.9 else (1.0, 0.0, 0.0)
    u = norm(cross(helper, a))
    v = cross(a, u)
    return u, v


class Part:
    """Triangles of one colour: vertices (position, normal) and index triples."""

    def __init__(self, swatch):
        self.swatch = swatch
        self.verts = []
        self.tris = []

    def vert(self, p, n):
        self.verts.append((p, norm(n)))
        return len(self.verts) - 1


class Model:
    def __init__(self, name, palette):
        self.name = name
        self.palette = palette  # swatch name -> rgb (the order gives the swatch's place)
        self.order = list(palette)
        self.parts = []

    def part(self, swatch):
        if swatch not in self.palette:
            raise KeyError(f"{self.name}: no swatch {swatch}")
        p = Part(swatch)
        self.parts.append(p)
        return p

    # ---- shapes ----
    def spheroid(self, swatch, center, radii, segs=12, rings=8, axis=None):
        """An ellipsoid; with axis, its y radius lies along that direction."""
        p = self.part(swatch)
        rx, ry, rz = radii
        if axis is None:
            ex, ey, ez = (1, 0, 0), (0, 1, 0), (0, 0, 1)
        else:
            ey = norm(axis)
            ex, ez = basis(ey)
        rows = []
        for r in range(rings + 1):
            th = math.pi * r / rings
            row = []
            for s in range(segs + 1):
                ph = math.tau * s / segs
                lx, ly, lz = math.sin(th) * math.cos(ph), math.cos(th), math.sin(th) * math.sin(ph)
                pos = add(center, add(mul(ex, lx * rx), add(mul(ey, ly * ry), mul(ez, lz * rz))))
                nl = (lx / rx, ly / ry, lz / rz)
                nrm = add(mul(ex, nl[0]), add(mul(ey, nl[1]), mul(ez, nl[2])))
                row.append(p.vert(pos, nrm))
            rows.append(row)
        for r in range(rings):
            for s in range(segs):
                a, b, c, d = rows[r][s], rows[r][s + 1], rows[r + 1][s + 1], rows[r + 1][s]
                if r > 0:
                    p.tris.append((a, c, b))
                if r < rings - 1:
                    p.tris.append((a, d, c))
        return p

    def tube(self, swatch, p0, p1, r0, r1, segs=10, caps=True, squash=1.0):
        """A tapered cylinder from p0 (radius r0) to p1 (radius r1); squash flattens it (front to back)."""
        p = self.part(swatch)
        axis = sub(p1, p0)
        ln = length(axis)
        a = norm(axis)
        u, v = basis(a)
        if abs(a[1]) > 0.7:  # upright: squash front to back (z)
            u, v = (1.0, 0.0, 0.0), norm(cross(a, (1.0, 0.0, 0.0)))
        slope = (r0 - r1) / ln
        ring0, ring1 = [], []
        for s in range(segs + 1):
            ph = math.tau * s / segs
            c, sn = math.cos(ph), math.sin(ph) * squash
            radial = add(mul(u, c), mul(v, sn))
            nrm = add(add(mul(u, c), mul(v, math.sin(ph) / max(squash, 1e-3))), mul(a, slope))
            ring0.append(p.vert(add(p0, mul(radial, r0)), nrm))
            ring1.append(p.vert(add(p1, mul(radial, r1)), nrm))
        for s in range(segs):
            p.tris.append((ring0[s], ring1[s], ring1[s + 1]))
            p.tris.append((ring0[s], ring1[s + 1], ring0[s + 1]))
        if caps:
            for center, rad, sign in ((p0, r0, -1), (p1, r1, 1)):
                if rad <= 1e-4:
                    continue
                n = mul(a, sign)
                mid = p.vert(center, n)
                ring = []
                for s in range(segs):
                    ph = math.tau * s / segs
                    radial = add(mul(u, math.cos(ph)), mul(v, math.sin(ph) * squash))
                    ring.append(p.vert(add(center, mul(radial, rad)), n))
                for s in range(segs):
                    t = (mid, ring[s], ring[(s + 1) % segs])
                    p.tris.append(t if sign > 0 else (t[0], t[2], t[1]))
        return p

    def box(self, swatch, center, size, rot_y=0.0, tilt=None):
        """A box (flat faces); rot_y turns it about y (radians), tilt=(axis, angle) turns it further."""
        p = self.part(swatch)
        hx, hy, hz = size[0] / 2, size[1] / 2, size[2] / 2
        faces = [((1, 0, 0), (0, 1, 0), (0, 0, 1)), ((-1, 0, 0), (0, 0, 1), (0, 1, 0)),
                 ((0, 1, 0), (0, 0, 1), (1, 0, 0)), ((0, -1, 0), (1, 0, 0), (0, 0, 1)),
                 ((0, 0, 1), (1, 0, 0), (0, 1, 0)), ((0, 0, -1), (0, 1, 0), (1, 0, 0))]

        def place(q):
            q = rotate(q, (0, 1, 0), rot_y)
            if tilt:
                q = rotate(q, tilt[0], tilt[1])
            return q
        for n, a, b in faces:
            c = (n[0] * hx, n[1] * hy, n[2] * hz)
            ea = (a[0] * hx, a[1] * hy, a[2] * hz)
            eb = (b[0] * hx, b[1] * hy, b[2] * hz)
            corners = [add(c, add(mul(ea, sa), mul(eb, sb))) for sa, sb in ((-1, -1), (1, -1), (1, 1), (-1, 1))]
            nn = place(n)
            idx = [p.vert(add(center, place(q)), nn) for q in corners]
            p.tris.append((idx[0], idx[1], idx[2]))
            p.tris.append((idx[0], idx[2], idx[3]))
        return p

    def blade(self, swatch, base, tip, width, thick, tip_len):
        """A flat blade from base to tip (y up in its frame), diamond cross-section, pointed."""
        p = self.part(swatch)
        a = norm(sub(tip, base))
        side = norm(cross(a, (0.0, 0.0, 1.0))) if abs(a[2]) < 0.9 else (1.0, 0.0, 0.0)
        face = cross(side, a)
        shoulder = add(tip, mul(a, -tip_len))
        sections = [(base, 1.0), (shoulder, 1.0), (tip, 0.0)]
        rings = []
        for pos, k in sections:
            pts = [add(pos, mul(side, width / 2 * k)), add(pos, mul(face, thick / 2 * k)),
                   add(pos, mul(side, -width / 2 * k)), add(pos, mul(face, -thick / 2 * k))]
            rings.append(pts)
        for r in range(2):
            for s in range(4):
                q = [rings[r][s], rings[r][(s + 1) % 4], rings[r + 1][(s + 1) % 4], rings[r + 1][s]]
                n = norm(cross(sub(q[1], q[0]), sub(q[3], q[0])))
                # outwards: away from the blade's axis
                mid = mul(add(add(q[0], q[1]), add(q[2], q[3])), 0.25)
                on_axis = add(base, mul(a, dot(sub(mid, base), a)))
                if dot(n, sub(mid, on_axis)) < 0:
                    n = mul(n, -1)
                idx = [p.vert(x, n) for x in q]
                p.tris.append((idx[0], idx[2], idx[1]))
                p.tris.append((idx[0], idx[3], idx[2]))
        return p

    def torus(self, swatch, center, axis, radius, thick, segs=16, sides=6, squash=1.0):
        """A ring around axis (radius to the tube's middle, thick the tube's radius); squash flattens it."""
        p = self.part(swatch)
        a = norm(axis)
        u, v = basis(a)
        if abs(a[1]) > 0.7:
            u, v = (1.0, 0.0, 0.0), norm(cross(a, (1.0, 0.0, 0.0)))
        rows = []
        for s in range(segs + 1):
            ph = math.tau * s / segs
            radial = add(mul(u, math.cos(ph)), mul(v, math.sin(ph) * squash))
            mid = add(center, mul(radial, radius))
            radial_n = norm(radial)
            row = []
            for k in range(sides + 1):
                th = math.tau * k / sides
                n = add(mul(radial_n, math.cos(th)), mul(a, math.sin(th)))
                row.append(p.vert(add(mid, mul(n, thick)), n))
            rows.append(row)
        for s in range(segs):
            for k in range(sides):
                q = (rows[s][k], rows[s + 1][k], rows[s + 1][k + 1], rows[s][k + 1])
                p.tris.append((q[0], q[1], q[2]))
                p.tris.append((q[0], q[2], q[3]))
        return p

    def slab(self, swatch, outline, origin, u, v, thick):
        """A flat shape: the outline (2D points in the plane origin + x*u + y*v) extruded by thick
        (centred). Convex or star-shaped around the outline's mean; flat normals."""
        p = self.part(swatch)
        n = norm(cross(u, v))
        cx = sum(x for x, _ in outline) / len(outline)
        cy = sum(y for _, y in outline) / len(outline)

        def at(x, y, side):
            return add(add(origin, add(mul(u, x), mul(v, y))), mul(n, side * thick / 2))
        for side in (1, -1):
            nn = mul(n, side)
            mid = p.vert(at(cx, cy, side), nn)
            ring = [p.vert(at(x, y, side), nn) for x, y in outline]
            for k in range(len(outline)):
                p.tris.append((mid, ring[k], ring[(k + 1) % len(outline)]))
        m = len(outline)
        for k in range(m):
            (x0, y0), (x1, y1) = outline[k], outline[(k + 1) % m]
            q = [at(x0, y0, 1), at(x1, y1, 1), at(x1, y1, -1), at(x0, y0, -1)]
            en = norm(cross(sub(q[1], q[0]), sub(q[3], q[0])))
            mid = at((x0 + x1) / 2, (y0 + y1) / 2, 0)
            if dot(en, sub(mid, at(cx, cy, 0))) < 0:
                en = mul(en, -1)
            idx = [p.vert(x, en) for x in q]
            p.tris.append((idx[0], idx[1], idx[2]))
            p.tris.append((idx[0], idx[2], idx[3]))
        return p

    def mirror_x(self, parts):
        """Copies parts mirrored at x = 0 (left from right)."""
        out = []
        for src in parts:
            p = self.part(src.swatch)
            for pos, n in src.verts:
                p.verts.append(((-pos[0], pos[1], pos[2]), (-n[0], n[1], n[2])))
            p.tris = [(a, c, b) for a, b, c in src.tris]
            out.append(p)
        return out

    # ---- output ----
    def bounds(self):
        pts = [v[0] for p in self.parts for v in p.verts]
        return [min(q[i] for q in pts) for i in range(3)], [max(q[i] for q in pts) for i in range(3)]

    def triangle_count(self):
        return sum(len(p.tris) for p in self.parts)

    def uv_of(self, part, pos, lo, hi):
        """The UV of a vertex: its place in the part's bounds, inside the part's swatch."""
        k = self.order.index(part.swatch)
        sx, sy = k % GRID, k // GRID
        span_u = max(hi[0] - lo[0], hi[2] - lo[2], 1e-6)
        span_v = max(hi[1] - lo[1], 1e-6)
        fu = ((pos[0] - lo[0]) + (pos[2] - lo[2])) / (2 * span_u)
        fv = (pos[1] - lo[1]) / span_v
        u = (sx + MARGIN + (1 - 2 * MARGIN) * fu) / GRID
        v = (sy + MARGIN + (1 - 2 * MARGIN) * fv) / GRID
        return u, 1.0 - v

    def paint_skin(self, seed):
        """The skin: a swatch per colour with grain and a little light from above."""
        img = Image.new("RGB", (SKIN, SKIN), (255, 0, 255))
        for k, name in enumerate(self.order):
            rgb = self.palette[name]
            sw = Image.new("RGB", (CELL, CELL), rgb)
            noise = T.fbm(CELL, seed + k, ((3, 0.5), (8, 0.3), (24, 0.2)))
            sw = T.modulate(sw, noise, 0.28)
            # brighter towards the swatch's top (= the part's top), as if lit from above
            grad = Image.new("L", (1, CELL))
            grad.putdata([int(128 * (1.12 - 0.24 * (y / (CELL - 1)))) for y in range(CELL)])
            sw = T.apply_light(sw, grad.resize((CELL, CELL)))
            img.paste(sw, ((k % GRID) * CELL, (k // GRID) * CELL))
        return img

    def fix_winding(self):
        """Turns every triangle counter-clockwise as seen from outside (where its normals point)."""
        flipped = 0
        for part in self.parts:
            out = []
            for a, b, c in part.tris:
                pa, pb, pc = (part.verts[i][0] for i in (a, b, c))
                geo = cross(sub(pb, pa), sub(pc, pa))
                n = add(add(part.verts[a][1], part.verts[b][1]), part.verts[c][1])
                if dot(geo, n) < 0:
                    out.append((a, c, b))
                    flipped += 1
                else:
                    out.append((a, b, c))
            part.tris = out
        return flipped

    def write_obj(self, folder, seed=1):
        os.makedirs(folder, exist_ok=True)
        self.fix_winding()
        lines = [f"# {self.name}: written by tools/make_models.py", f"mtllib {self.name}.mtl"]
        vts, vns, faces = [], [], []
        base = 0
        for part in self.parts:
            pts = [v[0] for v in part.verts]
            lo = [min(q[i] for q in pts) for i in range(3)]
            hi = [max(q[i] for q in pts) for i in range(3)]
            for pos, n in part.verts:
                lines.append("v %.4f %.4f %.4f" % pos)
                vns.append("vn %.4f %.4f %.4f" % n)
                vts.append("vt %.4f %.4f" % self.uv_of(part, pos, lo, hi))
            for a, b, c in part.tris:
                i, j, k = a + base + 1, b + base + 1, c + base + 1
                faces.append(f"f {i}/{i}/{i} {j}/{j}/{j} {k}/{k}/{k}")
            base += len(part.verts)
        lines += vts + vns + ["usemtl skin"] + faces
        with open(os.path.join(folder, self.name + ".obj"), "w", newline="\n") as f:
            f.write("\n".join(lines) + "\n")
        with open(os.path.join(folder, self.name + ".mtl"), "w", newline="\n") as f:
            f.write("newmtl skin\nKa 0.2 0.2 0.2\nKd 1 1 1\nKs 0.1 0.1 0.1\nNs 12\nd 1\n"
                    f"map_Kd {self.name}_skin.png\n")
        skin = self.paint_skin(seed)
        skin.save(os.path.join(folder, self.name + "_skin.png"), optimize=True)
        return skin


def rotate(q, axis, angle):
    """Rotates q about a unit axis by angle (Rodrigues)."""
    if angle == 0:
        return q
    k = norm(axis)
    c, s = math.cos(angle), math.sin(angle)
    return add(add(mul(q, c), mul(cross(k, q), s)), mul(k, dot(k, q) * (1 - c)))


# ---- a tiny software renderer (painter's algorithm) ----
def render(model, skin, yaw, pitch, size, frame=None, background=(0, 0, 0, 0), light=(-0.5, 0.8, 0.6),
           ambient=0.32, ss=3, ortho_fit=None):
    """Renders the model into an RGBA image of size (w, h), lit by a directional light.

    The camera looks along -z after turning the model by yaw (about y) and pitch (about x).
    frame: (x0, y0, x1, y1) in camera space that fills the image (default: the model's bounds,
    centred, with some room). Triangles are sorted back to front; each is flat, its colour the
    centre of its part's swatch times the light (smooth enough at token sizes).
    """
    w, h = size
    light = norm(light)
    tris = []
    for part in model.parts:
        k = model.order.index(part.swatch)
        base_col = skin.getpixel(((k % GRID) * CELL + CELL // 2, (k // GRID) * CELL + CELL // 2))
        for a, b, c in part.tris:
            n = norm(add(add(part.verts[a][1], part.verts[b][1]), part.verts[c][1]))
            cam = [rotate(rotate(part.verts[i][0], (0, 1, 0), -yaw), (1, 0, 0), pitch) for i in (a, b, c)]
            nv = rotate(rotate(n, (0, 1, 0), -yaw), (1, 0, 0), pitch)
            if nv[2] < -0.05:
                continue
            lam = max(0.0, dot(nv, light)) * (1.0 - ambient) + ambient
            col = tuple(min(255, int(ch * lam)) for ch in base_col[:3])
            tris.append((sum(q[2] for q in cam) / 3, cam, col))
    if frame is None:
        xs = [q[0] for _, cam, _ in tris for q in cam]
        ys = [q[1] for _, cam, _ in tris for q in cam]
        cx, cy = (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2
        half = max(max(xs) - min(xs), (max(ys) - min(ys)) * w / h) / 2 * 1.1
        frame = (cx - half, cy - half * h / w, cx + half, cy + half * h / w)
    x0, y0, x1, y1 = frame
    img = Image.new("RGBA", (w * ss, h * ss), background)
    d = ImageDraw.Draw(img)
    tris.sort(key=lambda t: t[0])
    for _, cam, col in tris:
        scr = [((q[0] - x0) / (x1 - x0) * w * ss, (y1 - q[1]) / (y1 - y0) * h * ss) for q in cam]
        d.polygon(scr, fill=col + (255,))
    return img.resize((w, h), Image.LANCZOS)


# ---- preview: a tiny software renderer (painter's algorithm) ----
def preview(model, skin, path, views=((0.35, 0.25), (0.0, 0.15), (math.pi / 2, 0.15), (math.pi, 0.2)), size=360):
    """Renders the model from a few directions (yaw, pitch) into one picture, lit from the top left front."""
    lo, hi = model.bounds()
    center = [(lo[i] + hi[i]) / 2 for i in range(3)]
    extent = max(hi[i] - lo[i] for i in range(3))
    light = norm((-0.5, 0.8, 0.6))
    ss = 2
    sheet = Image.new("RGB", (size * len(views), size), (24, 20, 18))
    for vi, (yaw, pitch) in enumerate(views):
        img = Image.new("RGB", (size * ss, size * ss), (24, 20, 18))
        d = ImageDraw.Draw(img)
        tris = []
        for part in model.parts:
            k = model.order.index(part.swatch)
            base_col = skin.getpixel(((k % GRID) * CELL + CELL // 2, (k // GRID) * CELL + CELL // 2))
            for a, b, c in part.tris:
                pa, pb, pc = (part.verts[i][0] for i in (a, b, c))
                n = norm(add(add(part.verts[a][1], part.verts[b][1]), part.verts[c][1]))
                cam = []
                for q in (pa, pb, pc):
                    q = sub(q, center)
                    q = rotate(q, (0, 1, 0), -yaw)
                    q = rotate(q, (1, 0, 0), pitch)
                    cam.append(q)
                nv = rotate(rotate(n, (0, 1, 0), -yaw), (1, 0, 0), pitch)
                if nv[2] < -0.05:
                    continue  # facing away (the camera looks along -z)
                depth = sum(q[2] for q in cam) / 3
                lam = max(0.0, dot(n, light)) * 0.75 + 0.3
                col = tuple(min(255, int(ch * lam)) for ch in base_col)
                scr = [((q[0] / extent * 0.85 + 0.5) * size * ss, (0.5 - q[1] / extent * 0.85) * size * ss) for q in cam]
                tris.append((depth, scr, col))
        tris.sort(key=lambda t: t[0])
        for _, scr, col in tris:
            d.polygon(scr, fill=col)
        sheet.paste(img.resize((size, size), Image.LANCZOS), (vi * size, 0))
    sheet.save(path)
