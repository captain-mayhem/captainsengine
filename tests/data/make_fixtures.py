"""Writes the small test files in this folder (Python standard library only).

The images all show the same 3x2 picture (rows top to bottom), so the tests
can check that every decoder delivers the same pixels. A width of 3 makes the
BMP rows need padding. image.jpg is written by the small baseline encoder
here (quality 100: every quantization step 1; no chroma subsampling), so it
decodes to the picture within a few levels.

The TIFFs (tiff/) show the picture in the ways TIFF can store it: byte
orders, compressions (none, PackBits, LZW, Deflate), the predictor, strips
and tiles, planar samples, palette, gray at 1 and 16 bits, and alpha;
tiff/gradient.tif is big enough (64x64) for the LZW codes to grow past 9
bits. tiff/gdiplus.tif comes from another encoder, Windows' GDI+ (LZW): it is
image.png saved by System.Drawing (PowerShell: [System.Drawing.Image]::
FromFile(...).Save(..., [System.Drawing.Imaging.ImageFormat]::Tiff)), so the
decoder isn't only checked against the encoder here. Only tiffs() writes
them: python -c "import make_fixtures; make_fixtures.tiffs()".

The sounds (audio/): sine.wav is written here; beat.mp3 (its first second)
and click.ogg come from the Adventure's demo game
(Adventure/AdvEngine/test/data). The audio tests encode Ogg Vorbis
themselves.

The glTF models (gltf/) are written by the small glTF writer here: parts
(meshes with every index size, primitive mode and attribute kind the reader
handles, two materials and a node hierarchy) as .gltf with external files,
as .gltf with data URIs and as .glb; one-quad models for the drawing tests;
and files with errors. Only gltf_models() writes them:
python -c "import make_fixtures; make_fixtures.gltf_models()".

The video (video/mpeg1.mpg) is made with ffmpeg, if it is on the PATH (it is
skipped otherwise): two seconds of MPEG-1 at 25 pictures per second, 160x120
(not a multiple of 16 high), with the quadrants red, green, blue and white
(left to right, top to bottom), and a 440 Hz sine in MP2 at 44.1 kHz. The
video tests encode Theora themselves.

Run it from anywhere: python make_fixtures.py
"""

import math
import os
import shutil
import struct
import subprocess
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


# the zigzag order of an 8x8 block's coefficients
ZIGZAG = sorted(range(64), key=lambda i: (i // 8 + i % 8, (i // 8) if (i // 8 + i % 8) % 2 else (i % 8)))


def huffman_codes(bits, values):
    """The canonical codes of a JPEG Huffman table: {value: (code, length)}."""
    codes = {}
    code = 0
    k = 0
    for length in range(1, 17):
        for _ in range(bits[length - 1]):
            codes[values[k]] = (code, length)
            code += 1
            k += 1
        code <<= 1
    return codes


def jpeg(pixels):
    """A baseline JFIF: 3 components at 1x1 sampling, all quantization steps 1.

    One DC and one AC table for all components, with the code lengths of the
    standard luminance tables (JPEG Annex K.3); the AC table lists every
    run/size symbol a block of this quality can need.
    """
    width, height = len(pixels[0]), len(pixels)
    dc_bits = [0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0]
    dc_values = list(range(12))
    ac_bits = [0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125]
    ac_values = [0x00, 0xF0] + [(run << 4) | size for size in range(1, 11) for run in range(16)]
    dc_codes = huffman_codes(dc_bits, dc_values)
    ac_codes = huffman_codes(ac_bits, ac_values)

    out = bytearray()
    acc = [0, 0]  # bit buffer, bit count

    def put(code, length):
        acc[0] = (acc[0] << length) | code
        acc[1] += length
        while acc[1] >= 8:
            acc[1] -= 8
            byte = (acc[0] >> acc[1]) & 0xFF
            out.append(byte)
            if byte == 0xFF:
                out.append(0)  # byte stuffing
        acc[0] &= (1 << acc[1]) - 1

    def put_value(v):
        size = abs(v).bit_length()
        bits = v if v >= 0 else v + (1 << size) - 1
        return size, bits

    def ycc(r, g, b):
        return (0.299 * r + 0.587 * g + 0.114 * b,
                -0.168736 * r - 0.331264 * g + 0.5 * b + 128,
                0.5 * r - 0.418688 * g - 0.081312 * b + 128)

    cos = [[math.cos((2 * x + 1) * u * math.pi / 16) for x in range(8)] for u in range(8)]
    previous_dc = [0, 0, 0]
    for by in range(0, height, 8):
        for bx in range(0, width, 8):
            for c in range(3):
                # the block, edge pixels repeated past the picture, shifted to -128..127
                block = [[ycc(*pixels[min(by + y, height - 1)][min(bx + x, width - 1)])[c] - 128
                          for x in range(8)] for y in range(8)]
                coefficients = [0] * 64
                for v in range(8):
                    for u in range(8):
                        total = sum(block[y][x] * cos[u][x] * cos[v][y] for y in range(8) for x in range(8))
                        cu = 1 / math.sqrt(2) if u == 0 else 1
                        cv = 1 / math.sqrt(2) if v == 0 else 1
                        coefficients[v * 8 + u] = int(round(0.25 * cu * cv * total))
                zz = [coefficients[i] for i in ZIGZAG]
                size, bits = put_value(zz[0] - previous_dc[c])
                previous_dc[c] = zz[0]
                put(*dc_codes[size])
                if size:
                    put(bits, size)
                run = 0
                for v in zz[1:]:
                    if v == 0:
                        run += 1
                        continue
                    while run > 15:
                        put(*ac_codes[0xF0])
                        run -= 16
                    size, bits = put_value(v)
                    put(*ac_codes[(run << 4) | size])
                    put(bits, size)
                    run = 0
                if run:
                    put(*ac_codes[0x00])
    if acc[1]:
        put((1 << (8 - acc[1])) - 1, 8 - acc[1])  # pad with ones

    def segment(marker, data):
        return struct.pack(">BBH", 0xFF, marker, len(data) + 2) + data

    def dht(kind_id, bits, values):
        return bytes([kind_id]) + bytes(bits) + bytes(values)

    return (b"\xFF\xD8"
            + segment(0xE0, b"JFIF\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00")
            + segment(0xDB, b"\x00" + bytes([1] * 64))
            + segment(0xC0, struct.pack(">BHHB", 8, height, width, 3)
                      + b"\x01\x11\x00\x02\x11\x00\x03\x11\x00")
            + segment(0xC4, dht(0x00, dc_bits, dc_values) + dht(0x10, ac_bits, ac_values))
            + segment(0xDA, b"\x03\x01\x00\x02\x00\x03\x00\x00\x3F\x00")
            + bytes(out) + b"\xFF\xD9")


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


def videos():
    ffmpeg = shutil.which("ffmpeg")
    if not ffmpeg:
        print("ffmpeg not found: video/mpeg1.mpg is kept as it is")
        return
    os.makedirs(os.path.join(HERE, "video"), exist_ok=True)
    quadrant = "s=80x60:r=25:d=2"
    graph = (f"color=c=red:{quadrant}[a];color=c=lime:{quadrant}[b];"
             f"color=c=blue:{quadrant}[c];color=c=white:{quadrant}[d];"
             "[a][b][c][d]xstack=inputs=4:layout=0_0|w0_0|0_h0|w0_h0[v];"
             # ffmpeg's sine has an amplitude of 1/8
             "sine=frequency=440:sample_rate=44100:duration=2,volume=4[s]")
    subprocess.run([ffmpeg, "-y", "-loglevel", "error", "-filter_complex", graph,
                    "-map", "[v]", "-map", "[s]", "-c:v", "mpeg1video", "-q:v", "2",
                    "-c:a", "mp2", "-b:a", "128k", "-f", "mpeg",
                    os.path.join(HERE, "video", "mpeg1.mpg")], check=True)
    print("video/mpeg1.mpg")


def tiff_lzw(data):
    """TIFF's LZW: 9 to 12 bit codes, most significant bit first, starting with
    a clear code; the width grows when the next code would need it, which the
    decoder sees one code early (TIFF's "early change")."""
    out = bytearray()
    acc = 0
    nbits = 0

    def put(code, width):
        nonlocal acc, nbits
        acc = (acc << width) | code
        nbits += width
        while nbits >= 8:
            nbits -= 8
            out.append((acc >> nbits) & 0xFF)

    table = {bytes([i]): i for i in range(256)}
    next_code = 258
    width = 9
    put(256, width)
    word = b""
    for value in data:
        longer = word + bytes([value])
        if longer in table:
            word = longer
            continue
        put(table[word], width)
        table[longer] = next_code
        next_code += 1
        if next_code >= (1 << width) and width < 12:
            width += 1
        if next_code >= 4094:
            put(256, width)
            table = {bytes([i]): i for i in range(256)}
            next_code = 258
            width = 9
        word = bytes([value])
    if word:
        put(table[word], width)
        next_code += 1
        if next_code >= (1 << width) and width < 12:
            width += 1
    put(257, width)
    if nbits:
        out.append((acc << (8 - nbits)) & 0xFF)
    return bytes(out)


def packbits(data):
    """Literal runs only, at most 128 bytes each (a valid PackBits stream)."""
    out = bytearray()
    for i in range(0, len(data), 128):
        chunk = data[i:i + 128]
        out.append(len(chunk) - 1)
        out += chunk
    return bytes(out)


def tiff(samples, width, height, spp, bps=8, big_endian=False, compression=1, predictor=1,
         photometric=2, planar=1, rows_per_strip=None, tile=None, extra=None, color_map=None):
    """A TIFF of one image. 'samples' are the sample values row by row,
    pixel by pixel (all samples of a pixel together), whatever the layout
    in the file is."""
    e = ">" if big_endian else "<"

    def pack_row(values):
        if bps == 16:
            return b"".join(struct.pack(e + "H", v) for v in values)
        if bps == 8:
            return bytes(values)
        bits = "".join(format(v, "0%db" % bps) for v in values)
        bits += "0" * (-len(bits) % 8)
        return bytes(int(bits[i:i + 8], 2) for i in range(0, len(bits), 8))

    def predict(values, step):
        mask = (1 << bps) - 1
        return [values[i] if i < step else (values[i] - values[i - step]) & mask for i in range(len(values))]

    planes = spp if planar == 2 else 1
    per_chunk = 1 if planar == 2 else spp
    if tile:
        cw, ch = tile
    else:
        cw, ch = width, rows_per_strip or height
    across = (width + cw - 1) // cw
    down = (height + ch - 1) // ch
    chunks = []
    for plane in range(planes):
        for cy in range(down):
            for cx in range(across):
                rows = ch if tile else min(ch, height - cy * ch)
                data = b""
                for r in range(rows):
                    y = cy * ch + r
                    values = []
                    for c in range(cw):
                        x = cx * cw + c
                        for s in range(per_chunk):
                            sample = plane + s
                            inside = x < width and y < height
                            values.append(samples[(y * width + x) * spp + sample] if inside else 0)
                    if predictor == 2:
                        values = predict(values, per_chunk)
                    data += pack_row(values)
                if compression == 5:
                    data = tiff_lzw(data)
                elif compression in (8, 32946):
                    data = zlib.compress(data)
                elif compression == 32773:
                    data = packbits(data)
                chunks.append(data)

    entries = []

    def tag(number, kind, values):
        entries.append((number, kind, values))

    tag(256, 4, [width])
    tag(257, 4, [height])
    tag(258, 3, [bps] * spp)
    tag(259, 3, [compression])
    tag(262, 3, [photometric])
    tag(277, 3, [spp])
    tag(284, 3, [planar])
    if predictor != 1:
        tag(317, 3, [predictor])
    if color_map:
        tag(320, 3, color_map)
    if extra:
        tag(338, 3, extra)
    if tile:
        tag(322, 3, [cw])
        tag(323, 3, [ch])
        offsets_tag, counts_tag = 324, 325
    else:
        tag(278, 4, [ch])
        offsets_tag, counts_tag = 273, 279
    # the image data follows the header; the directory follows the data
    data_start = 8
    offsets = []
    pos = data_start
    for chunk in chunks:
        offsets.append(pos)
        pos += len(chunk) + (len(chunk) & 1)
    tag(offsets_tag, 4, offsets)
    tag(counts_tag, 4, [len(c) for c in chunks])
    entries.sort()
    ifd = pos
    extra_start = ifd + 2 + 12 * len(entries) + 4
    directory = struct.pack(e + "H", len(entries))
    extra_data = b""
    for number, kind, values in entries:
        size = 2 if kind == 3 else 4
        raw = b"".join(struct.pack(e + ("H" if kind == 3 else "I"), v) for v in values)
        if len(raw) <= 4:
            field = raw + b"\0" * (4 - len(raw))
        else:
            field = struct.pack(e + "I", extra_start + len(extra_data))
            extra_data += raw + b"\0" * (len(raw) & 1)
        directory += struct.pack(e + "HHI", number, kind, len(values)) + field
    directory += struct.pack(e + "I", 0)
    header = (b"MM" if big_endian else b"II") + struct.pack(e + "HI", 42, ifd)
    body = b"".join(c + b"\0" * (len(c) & 1) for c in chunks)
    return header + body + directory + extra_data


def tiffs():
    os.makedirs(os.path.join(HERE, "tiff"), exist_ok=True)
    rgb = [v for row in PIXELS for p in row for v in p]
    rgba = [v for row in PIXELS for p in row for v in p + (255,)]
    write("tiff/rgb.tif", tiff(rgb, WIDTH, HEIGHT, 3))
    write("tiff/rgb_be_lzw_predictor.tif", tiff(rgb, WIDTH, HEIGHT, 3, big_endian=True, compression=5,
                                                predictor=2, rows_per_strip=1))
    write("tiff/rgba_deflate_predictor.tif", tiff(rgba, WIDTH, HEIGHT, 4, compression=8, predictor=2, extra=[2]))
    write("tiff/rgb_packbits_planar.tif", tiff(rgb, WIDTH, HEIGHT, 3, compression=32773, planar=2))
    write("tiff/rgb_tiled.tif", tiff(rgb, WIDTH, HEIGHT, 3, compression=32946, tile=(16, 16)))
    # a 4-bit palette with the picture's six colors at 0..5
    colors = [p for row in PIXELS for p in row]
    color_map = ([c[0] * 257 for c in colors] + [0] * 10 + [c[1] * 257 for c in colors] + [0] * 10
                 + [c[2] * 257 for c in colors] + [0] * 10)
    write("tiff/palette4.tif", tiff(list(range(6)), WIDTH, HEIGHT, 1, bps=4, photometric=3, color_map=color_map))
    # gray: 16 bits, big-endian, with the predictor; and 1 bit with 0 as white
    write("tiff/gray16.tif", tiff([0x0000, 0x8080, 0xFFFF, 0x1234, 0xFEDC, 0x4000], WIDTH, HEIGHT, 1, bps=16,
                                  big_endian=True, photometric=1, compression=8, predictor=2))
    write("tiff/bilevel.tif", tiff([1, 0, 1, 0, 1, 0], WIDTH, HEIGHT, 1, bps=1, photometric=0, compression=32773))
    # premultiplied: half transparent red is stored as (128, 0, 0, 128)
    write("tiff/premultiplied.tif", tiff([128, 0, 0, 128, 0, 0, 0, 0], 2, 1, 4, extra=[1]))
    # long enough runs and enough different strings for 10 and 11 bit LZW codes
    gradient = [((x // 3) * 7 + y * 13 + (x * y) % 5) & 0xFF for y in range(64) for x in range(64)]
    write("tiff/gradient.tif", tiff(gradient, 64, 64, 1, photometric=1, compression=5))


# glTF 2.0 models (gltf/), for the ModelFile tests (test_modelfile.cpp) and
# the drawing tests (test_drawing.cpp)

GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_FLOAT = 5120, 5121, 5122, 5123, 5125, 5126
ARRAY_BUFFER, ELEMENT_ARRAY_BUFFER = 34962, 34963


class Gltf:
    """A glTF document with one binary buffer and PNG images. It is written
    as .gltf with an external .bin and image files, as .gltf with data URIs,
    or as .glb with the images in the binary chunk."""

    def __init__(self):
        self.doc = {"asset": {"version": "2.0", "generator": "make_fixtures.py"}}
        self.bin = bytearray()
        self.images = []  # (file name, PNG bytes)

    def add(self, key, item):
        self.doc.setdefault(key, []).append(item)
        return len(self.doc[key]) - 1

    def view(self, data, target=None):
        while len(self.bin) % 4:
            self.bin.append(0)
        view = {"buffer": 0, "byteOffset": len(self.bin), "byteLength": len(data)}
        if target:
            view["target"] = target
        self.bin += data
        return self.add("bufferViews", view)

    def accessor(self, values, kind, component, normalized=False, indices=False):
        """values: tuples, or numbers for SCALAR."""
        fmt = {GL_BYTE: "b", GL_UNSIGNED_BYTE: "B", GL_SHORT: "h", GL_UNSIGNED_SHORT: "H",
               GL_UNSIGNED_INT: "I", GL_FLOAT: "f"}[component]
        rows = [v if isinstance(v, tuple) else (v,) for v in values]
        flat = [x for row in rows for x in row]
        data = struct.pack("<%d%s" % (len(flat), fmt), *flat)
        accessor = {"bufferView": self.view(data, ELEMENT_ARRAY_BUFFER if indices else ARRAY_BUFFER),
                    "componentType": component, "count": len(rows), "type": kind}
        if normalized:
            accessor["normalized"] = True
        if kind == "VEC3" and component == GL_FLOAT and not indices:
            # POSITION needs its bounds
            accessor["min"] = [min(r[i] for r in rows) for i in range(3)]
            accessor["max"] = [max(r[i] for r in rows) for i in range(3)]
        return self.add("accessors", accessor)

    def image(self, name, data):
        self.images.append((name, data))
        index = self.add("images", {"name": name})
        return self.add("textures", {"source": index})

    def write(self, path, variant):
        """variant: "gltf" (external files), "embedded" (data URIs) or "glb"."""
        import base64
        import copy
        import json
        doc = copy.deepcopy(self.doc)
        data = bytearray(self.bin)
        folder = os.path.dirname(path)
        base = os.path.splitext(os.path.basename(path))[0]
        for i, (name, png_data) in enumerate(self.images):
            image = doc["images"][i]
            if variant == "gltf":
                image["uri"] = name
                write(os.path.join(folder, name), png_data)
            elif variant == "embedded":
                image["uri"] = "data:image/png;base64," + base64.b64encode(png_data).decode()
            else:
                while len(data) % 4:
                    data.append(0)
                doc.setdefault("bufferViews", []).append({"buffer": 0, "byteOffset": len(data), "byteLength": len(png_data)})
                data += png_data
                image["bufferView"] = len(doc["bufferViews"]) - 1
                image["mimeType"] = "image/png"
        if data:
            buffer = {"byteLength": len(data)}
            if variant == "gltf":
                buffer["uri"] = base + ".bin"
                write(os.path.join(folder, base + ".bin"), bytes(data))
            elif variant == "embedded":
                buffer["uri"] = "data:application/octet-stream;base64," + base64.b64encode(bytes(data)).decode()
            doc["buffers"] = [buffer]
        text = json.dumps(doc, indent=1).encode()
        if variant != "glb":
            write(path, text)
            return
        while len(text) % 4:
            text += b" "
        while len(data) % 4:
            data.append(0)
        chunks = struct.pack("<I4s", len(text), b"JSON") + text
        if data:
            chunks += struct.pack("<I4s", len(data), b"BIN\0") + bytes(data)
        write(path, struct.pack("<4sII", b"glTF", 2, 12 + len(chunks)) + chunks)


def quad_positions(x0, y0, x1, y1):
    """A quad in the xy plane, counter-clockwise seen from +z."""
    return [(x0, y0, 0.0), (x1, y0, 0.0), (x1, y1, 0.0), (x0, y1, 0.0)]


def parts_model():
    """Meshes with the attributes, index sizes, primitive modes and
    materials the reader handles, and a node hierarchy with every kind of
    transformation. test_modelfile.cpp checks what it reads against this."""
    g = Gltf()
    picture = g.image("picture.png", png(PIXELS, 2))
    g.add("materials", {
        "name": "picture",
        "pbrMetallicRoughness": {"baseColorFactor": [0.5, 0.5, 0.5, 1.0], "baseColorTexture": {"index": picture},
                                 "metallicFactor": 0.0, "roughnessFactor": 1.0},
        "alphaMode": "MASK", "alphaCutoff": 0.25, "doubleSided": True})
    g.add("materials", {
        "name": "metal",
        "pbrMetallicRoughness": {"baseColorFactor": [1.0, 0.0, 0.0, 0.5], "metallicFactor": 1.0, "roughnessFactor": 0.5},
        "alphaMode": "BLEND"})
    # mesh 0 "box": a quad with normals, texture coordinates and 16-bit
    # indices, and a quad with vertex colours and 8-bit indices
    top = {
        "attributes": {
            "POSITION": g.accessor(quad_positions(0.0, 0.0, 1.0, 1.0), "VEC3", GL_FLOAT),
            "NORMAL": g.accessor([(0.0, 0.0, 1.0)] * 4, "VEC3", GL_FLOAT),
            "TEXCOORD_0": g.accessor([(0.0, 1.0), (1.0, 1.0), (1.0, 0.0), (0.0, 0.0)], "VEC2", GL_FLOAT),
        },
        "indices": g.accessor([0, 1, 2, 0, 2, 3], "SCALAR", GL_UNSIGNED_SHORT, indices=True),
        "material": 0}
    side = {
        "attributes": {
            "POSITION": g.accessor([(0.0, 0.0, 0.0), (0.0, 0.0, 1.0), (0.0, 1.0, 1.0), (0.0, 1.0, 0.0)], "VEC3", GL_FLOAT),
            "NORMAL": g.accessor([(-1.0, 0.0, 0.0)] * 4, "VEC3", GL_FLOAT),
            "COLOR_0": g.accessor([(255, 0, 0, 255), (0, 255, 0, 255), (0, 0, 255, 255), (255, 255, 255, 51)],
                                  "VEC4", GL_UNSIGNED_BYTE, normalized=True),
        },
        "indices": g.accessor([0, 1, 2, 0, 2, 3], "SCALAR", GL_UNSIGNED_BYTE, indices=True),
        "material": 1}
    g.add("meshes", {"name": "box", "primitives": [top, side]})
    # mesh 1 "strip": a triangle strip with 32-bit indices, quantized texture
    # coordinates, no normals and no material
    g.add("meshes", {"name": "strip", "primitives": [{
        "attributes": {
            "POSITION": g.accessor([(0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (1.0, 1.0, 0.0)], "VEC3", GL_FLOAT),
            "TEXCOORD_0": g.accessor([(0, 0), (65535, 0), (0, 65535), (65535, 65535)], "VEC2", GL_UNSIGNED_SHORT,
                                     normalized=True),
        },
        "indices": g.accessor([0, 1, 2, 3], "SCALAR", GL_UNSIGNED_INT, indices=True),
        "mode": 5}]})
    # mesh 2 (no name): a triangle fan without indices
    g.add("meshes", {"primitives": [{
        "attributes": {"POSITION": g.accessor(quad_positions(0.0, 0.0, 1.0, 1.0), "VEC3", GL_FLOAT)},
        "mode": 6}]})
    s = math.sqrt(0.5)
    g.doc["nodes"] = [
        {"name": "body", "mesh": 0, "translation": [1.0, 2.0, 3.0], "children": [1, 2, 3]},
        {"name": "socket_left", "translation": [0.5, 0.0, 0.0], "rotation": [0.0, s, 0.0, s]},
        {"name": "socket_right", "scale": [2.0, 2.0, 2.0], "children": [4]},
        # column-major: a translation by (0, 1, 0)
        {"name": "arm", "mesh": 1, "matrix": [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1]},
        {"name": "tip", "mesh": 2},
        {"name": "marker"},
        # the second user of mesh 0
        {"name": "body_copy", "mesh": 0, "translation": [-1.0, 0.0, 0.0]},
        # not in the scene
        {"name": "unused"},
    ]
    g.doc["scenes"] = [{"nodes": [0, 5, 6]}]
    g.doc["scene"] = 0
    return g


def quad_model(name, x0, y0, x1, y1, material, colors=None, texcoords=True, clockwise=False, normal=(0.0, 0.0, 1.0)):
    """A one-quad model for the drawing tests. Clockwise: seen from +z, the
    quad shows its back."""
    g = Gltf()
    positions = quad_positions(x0, y0, x1, y1)
    if clockwise:
        positions = [positions[0], positions[3], positions[2], positions[1]]
    attributes = {
        "POSITION": g.accessor(positions, "VEC3", GL_FLOAT),
        "NORMAL": g.accessor([normal] * 4, "VEC3", GL_FLOAT),
    }
    if texcoords:
        # v from the top of the image
        uv = [(0.0, 1.0), (1.0, 1.0), (1.0, 0.0), (0.0, 0.0)]
        if clockwise:
            uv = [uv[0], uv[3], uv[2], uv[1]]
        attributes["TEXCOORD_0"] = g.accessor(uv, "VEC2", GL_FLOAT)
    if colors:
        attributes["COLOR_0"] = g.accessor(colors, "VEC3", GL_FLOAT)
    # images of the material's textures: (file name, PNG bytes) under
    # "image" (the base colour), "mr_image", "normal_image",
    # "occlusion_image" and "emissive_image"
    material = dict(material)
    pbr = dict(material.get("pbrMetallicRoughness", {}))
    for key, texture in (("image", "baseColorTexture"), ("mr_image", "metallicRoughnessTexture")):
        if key in material:
            image = material.pop(key)
            pbr[texture] = {"index": g.image(image[0], image[1])}
    if pbr:
        material["pbrMetallicRoughness"] = pbr
    for key, texture in (("normal_image", "normalTexture"), ("occlusion_image", "occlusionTexture"),
                         ("emissive_image", "emissiveTexture")):
        if key in material:
            image = material.pop(key)
            info = dict(material.get(texture, {}))
            info["index"] = g.image(image[0], image[1])
            material[texture] = info
    if "extensions" in material:
        g.doc["extensionsUsed"] = sorted(material["extensions"])
    g.add("materials", material)
    g.add("meshes", {"primitives": [{"attributes": attributes,
                                     "indices": g.accessor([0, 1, 2, 0, 2, 3], "SCALAR", GL_UNSIGNED_SHORT, indices=True),
                                     "material": 0}]})
    g.doc["nodes"] = [{"name": name, "mesh": 0}]
    g.doc["scenes"] = [{"nodes": [0]}]
    return g


def gltf_models():
    os.makedirs(os.path.join(HERE, "gltf"), exist_ok=True)
    path = lambda name: os.path.join(HERE, "gltf", name)
    parts = parts_model()
    parts.write(path("parts.gltf"), "gltf")
    parts.write(path("parts_embedded.gltf"), "embedded")
    parts.write(path("parts.glb"), "glb")

    # the drawing tests: image.png on a quad of 3x2 pixels; grey vertex
    # colours (linear 0.5, sRGB 188) on a white quad; rgba.png (an opaque red
    # and a clear blue pixel) on a 2x1 quad with an alpha mask; the back of a
    # green double-sided quad, its normals away from the viewer. All have
    # texture coordinates: a lit draw without them makes NVIDIA's driver
    # recompile the vertex shader once, which its debug output reports
    white = {"pbrMetallicRoughness": {"metallicFactor": 0.0}}
    quad_model("picture", 0, 0, 3, 2, dict(white, image=("picture.png", png(PIXELS, 2)))).write(
        path("quad_picture.gltf"), "embedded")
    quad_model("grey", 0, 0, 3, 2, white, colors=[(0.5, 0.5, 0.5)] * 4).write(
        path("quad_colors.gltf"), "embedded")
    quad_model("mask", 0, 0, 2, 1, dict(white, alphaMode="MASK", alphaCutoff=0.5,
                                         image=("rgba.png", png([[(255, 0, 0, 255), (0, 0, 255, 0)]], 6)))).write(
        path("quad_mask.gltf"), "embedded")
    green = {"pbrMetallicRoughness": {"baseColorFactor": [0.0, 1.0, 0.0, 1.0], "metallicFactor": 0.0},
             "doubleSided": True}
    quad_model("back", 0, 0, 3, 2, green, clockwise=True, normal=(0.0, 0.0, -1.0)).write(
        path("quad_back.gltf"), "embedded")

    # the metallic-roughness material (BuiltinShader::Pbr): a red rough metal
    # and a red rough dielectric side by side on a 2x1 quad, by the factors
    # and by a metallic-roughness texture; a black quad giving off orange
    # light (linear 0.5, 0.25, 0 with the strength 2); a 3x1 white quad with
    # a normal map: flat, turned 60 degrees towards +x and towards +y (up)
    red_metal = {"pbrMetallicRoughness": {"baseColorFactor": [1.0, 0.0, 0.0, 1.0], "metallicFactor": 1.0,
                                          "roughnessFactor": 1.0}}
    red_dielectric = {"pbrMetallicRoughness": {"baseColorFactor": [1.0, 0.0, 0.0, 1.0], "metallicFactor": 0.0,
                                               "roughnessFactor": 1.0}}
    quad_model("metal", 0, 0, 1, 1, red_metal).write(path("quad_metal.gltf"), "embedded")
    quad_model("dielectric", 1, 0, 2, 1, red_dielectric).write(path("quad_dielectric.gltf"), "embedded")
    quad_model("metal_texture", 0, 0, 2, 1, dict(red_metal, mr_image=(
        "metal_roughness.png", png([[(0, 255, 255), (0, 255, 0)]], 2)))).write(path("quad_metal_texture.gltf"), "embedded")
    quad_model("emissive", 0, 0, 1, 1, {
        "pbrMetallicRoughness": {"baseColorFactor": [0.0, 0.0, 0.0, 1.0], "metallicFactor": 0.0},
        "emissiveFactor": [0.5, 0.25, 0.0],
        "extensions": {"KHR_materials_emissive_strength": {"emissiveStrength": 2.0}}}).write(
        path("quad_emissive.gltf"), "embedded")
    normals = [[(128, 128, 255), (238, 128, 191), (128, 238, 191)]]
    quad_model("normals", 0, 0, 3, 1, dict(white, normal_image=("normals.png", png(normals, 2)))).write(
        path("quad_normals.gltf"), "embedded")

    # every part of the metallic-roughness material, for test_modelfile.cpp
    quad_model("pbr", 0, 0, 1, 1, {
        "name": "pbr",
        "pbrMetallicRoughness": {"baseColorFactor": [0.5, 0.5, 0.5, 1.0], "metallicFactor": 0.25,
                                 "roughnessFactor": 0.75},
        "mr_image": ("metal_roughness.png", png([[(0, 255, 255), (0, 255, 0)]], 2)),
        "normal_image": ("normals.png", png(normals, 2)), "normalTexture": {"scale": 0.5},
        "occlusion_image": ("occlusion.png", png([[(128, 128, 128)]], 2)), "occlusionTexture": {"strength": 0.8},
        "emissive_image": ("emissive.png", png([[(255, 255, 255)]], 2)), "emissiveFactor": [1.0, 0.5, 0.0],
        "extensions": {"KHR_materials_emissive_strength": {"emissiveStrength": 2.0}}}).write(
        path("pbr.gltf"), "embedded")

    # files with errors: the reader says why instead of crashing
    write("gltf/broken.gltf", b'{"asset": {"version": "2.0"}, "nodes": [')
    write("gltf/missing_buffer.gltf", json_bytes({
        "asset": {"version": "2.0"}, "buffers": [{"uri": "nothere.bin", "byteLength": 12}]}))
    write("gltf/draco.gltf", json_bytes({
        "asset": {"version": "2.0"}, "extensionsUsed": ["KHR_draco_mesh_compression"],
        "extensionsRequired": ["KHR_draco_mesh_compression"]}))
    with open(path("parts.glb"), "rb") as f:
        write("gltf/truncated.glb", f.read()[:100])
    # an index past the vertices: the file is invalid
    g = Gltf()
    g.add("meshes", {"name": "bad", "primitives": [{
        "attributes": {"POSITION": g.accessor(quad_positions(0.0, 0.0, 1.0, 1.0), "VEC3", GL_FLOAT)},
        "indices": g.accessor([0, 1, 7], "SCALAR", GL_UNSIGNED_SHORT, indices=True)}]})
    g.doc["nodes"] = [{"name": "bad", "mesh": 0}]
    g.write(path("bad_index.gltf"), "embedded")
    # lines next to a triangle: the lines are left out
    g = Gltf()
    positions = g.accessor(quad_positions(0.0, 0.0, 1.0, 1.0), "VEC3", GL_FLOAT)
    g.add("meshes", {"name": "mixed", "primitives": [
        {"attributes": {"POSITION": positions}, "mode": 1},
        {"attributes": {"POSITION": positions},
         "indices": g.accessor([0, 1, 2], "SCALAR", GL_UNSIGNED_SHORT, indices=True)}]})
    g.doc["nodes"] = [{"name": "mixed", "mesh": 0}]
    g.write(path("lines.gltf"), "embedded")
    # images that can't be decoded: not an image, a PNG and a JPEG with
    # broken data; the model is read without their textures
    for name, data in (("garbage", b"this is no image"),
                       ("broken_png", b"\x89PNG\r\n\x1a\n" + b"\0" * 40),
                       ("broken_jpeg", b"\xff\xd8\xff\xe0" + b"\x12" * 40)):
        g = quad_model(name, 0, 0, 1, 1, dict(white, image=(name + ".png", data)))
        g.write(path("bad_image_%s.gltf" % name), "embedded")


def json_bytes(doc):
    import json
    return json.dumps(doc, indent=1).encode()


def main():
    write("image.bmp", bmp())
    write("image.png", png(PIXELS, 2))
    write("rgba.png", png([[(255, 0, 0, 255), (0, 0, 255, 0)]], 6))
    write("image.gif", gif())
    write("image.jpg", jpeg(PIXELS))
    tiffs()
    zip_archive()
    os.makedirs(os.path.join(HERE, "folder"), exist_ok=True)
    write("folder/a.txt", b"first\n")
    write("folder/b.txt", b"second\n")
    atlases()
    gltf_models()
    sounds()
    videos()


if __name__ == "__main__":
    main()
