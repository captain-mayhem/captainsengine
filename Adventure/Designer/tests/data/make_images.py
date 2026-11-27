"""Writes the images of the StoryDesigner's image tests (test_stateframes.cpp)
into images/: a PNG next to the project, one in the folder of its media
pool folder, and a gfx.dat with a .pnj and its
.pna alpha, as the C# StoryDesigner packs a game's images. Needs Pillow."""

import io
import os
import zipfile

from PIL import Image

here = os.path.join(os.path.dirname(os.path.abspath(__file__)), "images")
os.makedirs(here, exist_ok=True)

# 4x2, the top left pixel red and opaque, the bottom right half transparent
wisp = Image.new("RGBA", (4, 2), (0, 0, 255, 255))
wisp.putpixel((0, 0), (255, 0, 0, 255))
wisp.putpixel((3, 1), (0, 255, 0, 0))
wisp.save(os.path.join(here, "wisp.png"))

# in the folder of its media pool folder, as an unpacked .adz has it
os.makedirs(os.path.join(here, "CHARACTER"), exist_ok=True)
Image.new("RGBA", (3, 5), (255, 255, 255, 255)).save(os.path.join(here, "CHARACTER", "deep.png"))

# 16x8 green, its alpha white on the left and black on the right
def jpeg(img):
  buf = io.BytesIO()
  img.save(buf, "JPEG", quality=95)
  return buf.getvalue()

box = Image.new("RGB", (16, 8), (0, 200, 0))
alpha = Image.new("L", (16, 8), 0)
for y in range(8):
  for x in range(8):
    alpha.putpixel((x, y), 255)
with zipfile.ZipFile(os.path.join(here, "gfx.dat"), "w", zipfile.ZIP_DEFLATED) as z:
  # fixed dates so that the file only changes with its images
  for name, data in (("box.pnj", jpeg(box)), ("box.pna", jpeg(alpha))):
    info = zipfile.ZipInfo(name, date_time=(2026, 11, 27, 0, 0, 0))
    info.compress_type = zipfile.ZIP_DEFLATED
    z.writestr(info, data)
