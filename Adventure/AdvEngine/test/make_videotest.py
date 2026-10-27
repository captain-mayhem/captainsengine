"""Makes a test game for videos from the demo game (data/): the demo, whose
start cutscene first plays the given videos (playvideo, waiting for each).
The first one covers the screen, the others play at (160, 120) in 320x240.

    python make_videotest.py <output folder> <video>...

A video's name in the game is its file name without the extension, in lower
case. Escape skips the cutscene, and with it the video that plays.
"""

import io
import os
import shutil
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
DEMO = os.path.join(HERE, "data")


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    out = sys.argv[1]
    videos = sys.argv[2:]
    names = [os.path.splitext(os.path.basename(v))[0].lower() for v in videos]
    os.makedirs(out, exist_ok=True)
    for name in os.listdir(DEMO):
        if name not in ("game.dat", "movie.dat"):
            shutil.copy(os.path.join(DEMO, name), os.path.join(out, name))

    with zipfile.ZipFile(os.path.join(DEMO, "game.dat")) as z:
        files = {i.filename: z.read(i.filename) for i in z.infolist()}
    # the movies: name and path, up to an empty line
    project = files["game.001"]
    movies = b"".join(b"%s\r\nC:\\videos\\%s\r\n" % (n.encode(), os.path.basename(v).encode())
                      for n, v in zip(names, videos))
    assert project.count(b"Movie :\r\n") == 1
    files["game.001"] = project.replace(b"Movie :\r\n", b"Movie :\r\n" + movies)
    # the start cutscene plays them before its title
    scripts = files["game.003"]
    start = b"//Cutscene realstart\r\n"
    assert scripts.count(start) == 1
    calls = b""
    for i, n in enumerate(names):
        if i == 0:
            calls += b"playvideo(%s; true)\r\n" % n.encode()
        else:
            calls += b"playvideo(%s; true; 160; 120; 320; 240)\r\n" % n.encode()
    files["game.003"] = scripts.replace(start, start + calls)
    with zipfile.ZipFile(os.path.join(out, "game.dat"), "w") as z:
        for name, data in files.items():
            z.writestr(name, data)

    with zipfile.ZipFile(os.path.join(out, "movie.dat"), "w") as z:
        for v in videos:
            z.write(v, os.path.basename(v))
    print("%s: %s" % (out, ", ".join(names)))


if __name__ == "__main__":
    main()
