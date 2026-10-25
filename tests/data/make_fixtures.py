"""Writes the small test files in this folder (Python standard library only).

The images all show the same 3x2 picture (rows top to bottom), so the tests
can check that every decoder delivers the same pixels. A width of 3 makes the
BMP rows need padding. image.jpg can't be written without an encoder; it's a
copy of heroquest/HQClient/textures/other/script.jpg.

The sounds (audio/): sine.wav is written here; beat.mp3 (its first second)
and click.ogg come from the Adventure's demo game
(Adventure/AdvEngine/test/data). The audio tests encode Ogg Vorbis
themselves.

Run it from anywhere: python make_fixtures.py
"""

import math
import os
import struct
import zipfile
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))

# the picture, rows top to bottom
PIXELS = [
    [(255, 0, 0), (0, 255, 0), (0, 0, 255)],
    [(255, 255, 255), (0, 0, 0), (128, 64, 32)],
]
WIDTH = len(PIXELS[0])
HEIGHT = len(PIXELS)


def write(name, data):
    with open(os.path.join(HERE, name), "wb") as f:
        f.write(data)


def bmp():
    """24-bit, uncompressed, rows bottom to top, padded to 4 bytes."""
    stride = (WIDTH * 3 + 3) // 4 * 4
    rows = b""
    for row in reversed(PIXELS):
        line = b"".join(bytes((b, g, r)) for r, g, b in row)
        rows += line + b"\0" * (stride - len(line))
    header_size = 14 + 40
    file_header = struct.pack("<2sIHHI", b"BM", header_size + len(rows), 0, 0, header_size)
    info_header = struct.pack("<IiiHHIIiiII", 40, WIDTH, HEIGHT, 1, 24, 0, len(rows), 2835, 2835, 0, 0)
    return file_header + info_header + rows


def png_chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)


def png(pixels, color_type):
    """8 bits per channel, colour type 2 (RGB) or 6 (RGBA), no filtering."""
    height = len(pixels)
    width = len(pixels[0])
    raw = b"".join(b"\0" + b"".join(bytes(p) for p in row) for row in pixels)
    return (b"\x89PNG\r\n\x1a\n"
            + png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, color_type, 0, 0, 0))
            + png_chunk(b"IDAT", zlib.compress(raw))
            + png_chunk(b"IEND", b""))


def gif():
    """GIF89a with an 8-colour global palette. The LZW data sends a clear code
    before every four pixels, so the code size stays at 4 bits and no real
    compression is needed."""
    palette = []
    for row in PIXELS:
        for p in row:
            if p not in palette:
                palette.append(p)
    palette += [(0, 0, 0)] * (8 - len(palette))
    indices = [palette.index(p) for row in PIXELS for p in row]

    min_code_size = 3
    clear = 1 << min_code_size
    end = clear + 1
    codes = []
    for i, index in enumerate(indices):
        if i % 4 == 0:
            codes.append(clear)
        codes.append(index)
    codes.append(end)
    bits = 0
    nbits = 0
    data = bytearray()
    for code in codes:
        bits |= code << nbits
        nbits += 4
        while nbits >= 8:
            data.append(bits & 0xFF)
            bits >>= 8
            nbits -= 8
    if nbits:
        data.append(bits & 0xFF)

    out = b"GIF89a"
    # screen descriptor: global colour table with 2^(2+1) entries
    out += struct.pack("<HHBBB", WIDTH, HEIGHT, 0x80 | 0x70 | 0x02, 0, 0)
    out += b"".join(bytes(p) for p in palette)
    # image descriptor, no local colour table, not interlaced
    out += b"," + struct.pack("<HHHHB", 0, 0, WIDTH, HEIGHT, 0)
    out += bytes((min_code_size, len(data))) + bytes(data) + b"\0"
    out += b";"
    return out


def zip_archive():
    path = os.path.join(HERE, "archive.zip")
    with zipfile.ZipFile(path, "w") as z:
        z.writestr("hello.txt", "Hello from the zip\n" * 20, compress_type=zipfile.ZIP_DEFLATED)
        z.writestr("dir/stored.bin", bytes(range(16)), compress_type=zipfile.ZIP_STORED)


# the atlas page atlas/page.png, 8x4: 2x2 blocks, rows top to bottom
ATLAS_BLOCKS = [
    [(255, 0, 0), (0, 255, 0), (0, 0, 255), (0, 0, 128)],
    [(0, 0, 64), (0, 0, 192), (255, 255, 255), (255, 255, 255)],
]

# the frames on it: name, block column and row; green is trimmed out of a 4x4
# image, the blue ones are an animation listed out of order
ATLAS_FRAMES = [
    ("red.png", 0, 0, None),
    ("green.png", 1, 0, (1, 2, 4, 4)),
    ("blue_1.png", 2, 0, None),
    ("blue_10.png", 3, 0, None),
    ("blue_0.png", 0, 1, None),
    ("blue_2.png", 1, 1, None),
]


def atlas_page():
    rows = []
    for block_row in ATLAS_BLOCKS:
        line = [tuple(c) + (255,) for c in block_row for _ in range(2)]
        rows += [line, line]
    return png(rows, 6)


def atlas_frame(name, col, row, trim, scale):
    frame = {"frame": {"x": col * 2, "y": row * 2, "w": 2, "h": 2}, "rotated": False}
    if trim:
        x, y, w, h = trim
        frame["trimmed"] = True
        frame["spriteSourceSize"] = {"x": x, "y": y, "w": 2, "h": 2}
        frame["sourceSize"] = {"w": w, "h": h}
        frame["pivot"] = {"x": 0.25, "y": 1}
    return frame


def atlases():
    """The same frames as TexturePacker's JSON (Hash), JSON (Array) and
    Phaser 3 (two pages, at scale 2) formats."""
    import json
    os.makedirs(os.path.join(HERE, "atlas"), exist_ok=True)
    write("atlas/page.png", atlas_page())
    meta = {"image": "page.png", "size": {"w": 8, "h": 4}, "scale": "1"}
    frames = {name: atlas_frame(name, c, r, t, 1) for name, c, r, t in ATLAS_FRAMES}
    write("atlas/hash.json", json.dumps({"frames": frames, "meta": meta}, indent=1).encode())
    array = [dict(filename=name, **atlas_frame(name, c, r, t, 1)) for name, c, r, t in ATLAS_FRAMES]
    write("atlas/array.json", json.dumps({"frames": array, "meta": meta}, indent=1).encode())
    pages = [
        {"image": "page.png", "format": "RGBA8888", "size": {"w": 8, "h": 4}, "scale": "2", "frames": array[:3]},
        {"image": "page.png", "format": "RGBA8888", "size": {"w": 8, "h": 4}, "scale": 2, "frames": array[3:]},
    ]
    write("atlas/pages.json", json.dumps({"textures": pages}, indent=1).encode())


DEMO_DATA = os.path.join(HERE, "..", "..", "Adventure", "AdvEngine", "test", "data")


def sine_wav(rate=44100, seconds=0.5, freq=440.0, amplitude=0.5):
    """16-bit mono PCM."""
    frames = int(rate * seconds)
    samples = b"".join(struct.pack("<h", int(round(amplitude * 32767 * math.sin(2 * math.pi * freq * i / rate))))
                       for i in range(frames))
    fmt = struct.pack("<HHIIHH", 1, 1, rate, rate * 2, 2, 16)
    body = b"WAVE" + b"fmt " + struct.pack("<I", len(fmt)) + fmt + b"data" + struct.pack("<I", len(samples)) + samples
    return b"RIFF" + struct.pack("<I", len(body)) + body


def mp3_start(data, seconds):
    """The whole MPEG-1 Layer III frames of the first seconds."""
    rates = [44100, 48000, 32000]
    bitrates = [0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320]
    pos = 0
    if data[:3] == b"ID3":
        pos = 10 + ((data[6] << 21) | (data[7] << 14) | (data[8] << 7) | data[9])
    start = pos
    duration = 0.0
    while duration < seconds and pos + 4 <= len(data):
        header = data[pos:pos + 4]
        if header[0] != 0xFF or (header[1] & 0xFE) != 0xFA:
            raise ValueError("not an MPEG-1 Layer III frame at %d" % pos)
        rate = rates[(header[2] >> 2) & 3]
        length = 144 * bitrates[header[2] >> 4] * 1000 // rate + ((header[2] >> 1) & 1)
        pos += length
        duration += 1152 / rate
    return data[start:pos]


def sounds():
    os.makedirs(os.path.join(HERE, "audio"), exist_ok=True)
    write("audio/sine.wav", sine_wav())
    with zipfile.ZipFile(os.path.join(DEMO_DATA, "music.dat")) as z:
        write("audio/beat.mp3", mp3_start(z.read("crazybeat.mp3"), 1.0))
    with zipfile.ZipFile(os.path.join(DEMO_DATA, "sfx.dat")) as z:
        write("audio/click.ogg", z.read("click.ogg"))


def main():
    write("image.bmp", bmp())
    write("image.png", png(PIXELS, 2))
    write("rgba.png", png([[(255, 0, 0, 255), (0, 0, 255, 0)]], 6))
    write("image.gif", gif())
    zip_archive()
    os.makedirs(os.path.join(HERE, "folder"), exist_ok=True)
    write("folder/a.txt", b"first\n")
    write("folder/b.txt", b"second\n")
    atlases()
    sounds()


if __name__ == "__main__":
    main()
