"""Makes a test game for the screen changes from the demo game (data/): the
demo, whose start cutscene then loads its two rooms in turn, each time with
another screen change (loadroom with a change's name), waiting between them.

    python make_screenchangetest.py <output folder> [change...]

The changes come one after the other, starting about 4 s after the start
(after the demo's title): by default fadeblack, rectangle, circle, shutters,
clock, blend, blendslow, each followed by WAIT seconds. Run the Adventure
with <output folder>/game.dat, or pack it for the web like the video test.
"""

import os
import shutil
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
DEMO = os.path.join(HERE, "data")
CHANGES = ["fadeblack", "rectangle", "circle", "shutters", "clock", "blend", "blendslow"]
# seconds after each change (the slow ones take 2 s)
WAIT = 3


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    out = sys.argv[1]
    changes = sys.argv[2:] or CHANGES
    os.makedirs(out, exist_ok=True)
    for name in os.listdir(DEMO):
        if name != "game.dat":
            shutil.copy(os.path.join(DEMO, name), os.path.join(out, name))

    with zipfile.ZipFile(os.path.join(DEMO, "game.dat")) as z:
        files = {i.filename: z.read(i.filename) for i in z.infolist()}
    # after the start cutscene's first room
    scripts = files["game.003"]
    start = b"//Cutscene start\r\nloadroom(FirstRoom)\nsetfocus(Horst)\r\n"
    assert scripts.count(start) == 1
    calls = b"wait(1)\n"
    for i, change in enumerate(changes):
        room = b"SecondRoom" if i % 2 == 0 else b"FirstRoom"
        calls += b"loadroom(%s; %s)\nwait(%d)\n" % (room, change.encode(), WAIT)
    files["game.003"] = scripts.replace(start, start + calls)
    with zipfile.ZipFile(os.path.join(out, "game.dat"), "w") as z:
        for name, data in files.items():
            z.writestr(name, data)
    print("%s: %s" % (out, ", ".join(changes)))


if __name__ == "__main__":
    main()
