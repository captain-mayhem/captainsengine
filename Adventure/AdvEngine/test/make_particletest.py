"""Makes a test game for the particles from the demo game (data/): the demo,
whose start cutscene then sets the particles (setparticles) to copies of an
object falling slanted and turning, starts them at once (startparticles
fast), loads the second room and the first again, stops them (they fly out)
and starts them again slowly.

    python make_particletest.py <output folder> [object]

The object is Money by default. The particles start about 1 s after the
demo's title; the room changes come 4 s apart, the stop 4 s later, the
second start 6 s after that. Run the Adventure with <output folder>/game.dat,
or pack it for the web like the video test.
"""

import os
import shutil
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
DEMO = os.path.join(HERE, "data")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    out = sys.argv[1]
    obj = sys.argv[2] if len(sys.argv) > 2 else "Money"
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
    # speed 4 px per 40 ms, at most 40, 20 degrees from straight down,
    # turning 6 degrees per 40 ms, the speed varying by 50%
    calls = (b"wait(1)\n"
             b"setparticles(%s; 4; 40; 20; 6; 50)\n"
             b"startparticles(fast)\n"
             b"wait(4)\n"
             b"loadroom(SecondRoom)\n"
             b"wait(4)\n"
             b"loadroom(FirstRoom)\n"
             b"wait(4)\n"
             b"stopparticles()\n"
             b"wait(6)\n"
             b"startparticles()\n") % obj.encode()
    files["game.003"] = scripts.replace(start, start + calls)
    with zipfile.ZipFile(os.path.join(out, "game.dat"), "w") as z:
        for name, data in files.items():
            z.writestr(name, data)
    print("%s: particles of %s" % (out, obj))


if __name__ == "__main__":
    main()
