"""Small helpers for Torchdelve's painted textures (Pillow only, no numpy).

Everything here wraps around the image's edges, so the textures tile without
seams: drawing repeats shapes across the borders, blurs work on a 3x3 tiling,
and ImageChops.offset (which wraps) gives the light and shadow of a height map.
"""

import random

from PIL import Image, ImageChops, ImageDraw, ImageFilter


def wrap_filter(img, flt):
    """Applies a Pillow filter as if the image repeated in every direction."""
    w, h = img.size
    big = Image.new(img.mode, (w * 3, h * 3))
    for i in range(3):
        for j in range(3):
            big.paste(img, (i * w, j * h))
    return big.filter(flt).crop((w, h, w * 2, h * 2))


def blur(img, radius):
    return wrap_filter(img, ImageFilter.GaussianBlur(radius))


def value_noise(size, cells, seed):
    """Seamless smooth noise (L): random values on a cells x cells grid, upscaled bicubically."""
    rnd = random.Random(seed)
    small = Image.new("L", (cells, cells))
    small.putdata([rnd.randrange(256) for _ in range(cells * cells)])
    big = Image.new("L", (cells * 3, cells * 3))
    for i in range(3):
        for j in range(3):
            big.paste(small, (i * cells, j * cells))
    big = big.resize((size * 3, size * 3), Image.BICUBIC)
    return big.crop((size, size, size * 2, size * 2))


def fbm(size, seed, octaves=((4, 0.45), (8, 0.25), (16, 0.15), (48, 0.1), (128, 0.05))):
    """Fractal noise (L) from several octaves of value noise, normalized to 0..255."""
    acc = None
    total = 0.0
    for k, (cells, weight) in enumerate(octaves):
        layer = value_noise(size, cells, seed * 101 + k)
        if acc is None:
            acc = layer
            total = weight
        else:
            total += weight
            acc = Image.blend(acc, layer, weight / total)
    return acc.point(_stretch(acc))


def _stretch(img):
    lo, hi = img.getextrema()
    span = max(1, hi - lo)
    return lambda v: max(0, min(255, (v - lo) * 255 // span))


def draw_wrapped(size, shapes, mode="L", background=0):
    """Draws polygons (points, fill) repeated across the borders so they wrap."""
    w, h = size
    img = Image.new(mode, size, background)
    d = ImageDraw.Draw(img)
    for points, fill in shapes:
        for dx in (-w, 0, w):
            for dy in (-h, 0, h):
                d.polygon([(x + dx, y + dy) for x, y in points], fill=fill)
    return img


def shade(height, strength=2.0, dx=2, dy=2):
    """Light from the top left on a height map (L): 128 is flat, brighter faces the light."""
    # (h - h shifted towards the bottom right) * strength + 128
    return ImageChops.subtract(height, ImageChops.offset(height, dx, dy), scale=1.0 / strength, offset=128)


def apply_light(color, light):
    """Multiplies an RGB image by a light map (L) where 128 means unchanged."""
    lrgb = Image.merge("RGB", (light, light, light))
    return ImageChops.add(ImageChops.multiply(color, lrgb), ImageChops.multiply(color, lrgb))


def modulate(color, noise, amount):
    """Darkens and brightens an RGB image by a noise (L) around its mean, by amount (0..1)."""
    light = noise.point(lambda v: int(128 + (v - 128) * amount))
    return apply_light(color, light)


def jitter_color(rgb, rnd, amount, hue=0.03):
    """A colour varied in brightness (by amount) and a little in hue (warmer or cooler)."""
    k = 1.0 + rnd.uniform(-amount, amount)
    warm = rnd.uniform(-hue, hue)
    r, g, b = rgb
    return tuple(max(0, min(255, int(v))) for v in (r * k * (1 + warm), g * k, b * k * (1 - warm)))


def desaturate(img, amount):
    """Moves an RGB image towards its grey by amount (0..1)."""
    grey = img.convert("L").convert("RGB")
    return Image.blend(img, grey, amount)


def rounded_rect(x0, y0, x1, y1, r, steps=4):
    """A rectangle with cut corners as a polygon (a rough rounding)."""
    pts = []
    import math
    corners = ((x1 - r, y0 + r, -90), (x1 - r, y1 - r, 0), (x0 + r, y1 - r, 90), (x0 + r, y0 + r, 180))
    for cx, cy, start in corners:
        for s in range(steps + 1):
            a = math.radians(start + 90 * s / steps)
            pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


def wobble(points, rnd, amount):
    """Moves every point a little, for hand-cut stones."""
    return [(x + rnd.uniform(-amount, amount), y + rnd.uniform(-amount, amount)) for x, y in points]
