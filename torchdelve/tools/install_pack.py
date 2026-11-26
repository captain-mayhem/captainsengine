"""Installs a content pack over an installed Torchdelve and compiles its quests.

A content pack is a folder laid out like the game's own (TDClient/, TDServer/,
TDEditor/): its files replace the installed ones of the same path, so a pack
holds only what it changes (data/*.dat, scripts/*.lua, quests, level lists,
textures/items/icons.dat, ...). An optional pack.ini in its root sets:
  appName = <name>   the folder of the heroes' saves (TDClient/engine.ini's
                     appName), so the pack's heroes stay apart from the game's

Install the pack into an install folder of its own, so no files of the game's
content are left over, e.g. on Windows:
  cmake --install build/windows --prefix build/install/windows-<pack>
  python torchdelve/tools/install_pack.py build/install/windows-<pack> --pack <pack folder>

Then (and without --pack, for the game's own content) the quests are compiled:
for each level package in TDClient/levels/<package>/levels.dat, TDEditor
compiles TDEditor/<package>/<quest>.map, and the results go where the client
(.tdm, .lua, .hsf) and the server (.tdm, .lua) load them.

Data names a pack may use instead of the game's: data/heros.dat takes
"model: <file>" after the weapon (models/heros/<file>.obj), data/monsters.dat
"model: <file>" after the mind (models/monsters/, textures/monsters/; otherwise
the name in lower case without '_'), and a line of textures/items/icons.dat may
name another item's icon after the item ("potion_of_healing hearthbalm").

Usage: python tools/install_pack.py <install folder> [--pack <folder>] [--no-compile]
"""

import argparse
import os
import re
import shutil
import subprocess
import sys

PARTS = ("TDClient", "TDServer", "TDEditor")


def copy_pack(pack, install):
    copied = 0
    for part in PARTS:
        src_root = os.path.join(pack, part)
        if not os.path.isdir(src_root):
            continue
        for folder, _, files in os.walk(src_root):
            rel = os.path.relpath(folder, src_root)
            dst_folder = os.path.normpath(os.path.join(install, part, rel))
            os.makedirs(dst_folder, exist_ok=True)
            for name in files:
                shutil.copy2(os.path.join(folder, name), os.path.join(dst_folder, name))
                copied += 1
    print(f"copied {copied} files of {pack}")


def read_pack_ini(pack):
    settings = {}
    path = os.path.join(pack, "pack.ini")
    if os.path.exists(path):
        for line in open(path, encoding="utf-8"):
            line = line.split("#", 1)[0].strip()
            if "=" in line:
                key, value = (part.strip() for part in line.split("=", 1))
                settings[key] = value.strip('"')
    return settings


def set_app_name(install, name):
    path = os.path.join(install, "TDClient", "engine.ini")
    with open(path, newline="") as f:
        text = f.read()
    nl = "\r\n" if "\r\n" in text else "\n"
    line = f'appName = "{name}"'
    if re.search(r"^appName\s*=.*$", text, flags=re.M):
        text = re.sub(r"^appName\s*=.*$", line, text, flags=re.M)
    else:
        text = line + nl + text
    with open(path, "w", newline="") as f:
        f.write(text)
    print(f"the heroes' saves go into the folder {name}")


def compile_quests(install):
    editor_dir = os.path.abspath(os.path.join(install, "TDEditor"))
    exe = os.path.join(editor_dir, "TDEditor.exe" if os.name == "nt" else "TDEditor")
    levels_root = os.path.join(install, "TDClient", "levels")
    failed = []
    for package in sorted(os.listdir(levels_root)):
        listing = os.path.join(levels_root, package, "levels.dat")
        if not os.path.exists(listing):
            continue
        names = open(listing).read().split()[1:]  # the count, then the quests
        server_dir = os.path.join(install, "TDServer", "levels", package)
        os.makedirs(server_dir, exist_ok=True)
        for name in names:
            source = os.path.join(editor_dir, package, name)
            if not os.path.exists(source + ".map"):
                failed.append(f"{package}/{name}: no .map")
                continue
            if os.path.exists(source + ".tdm"):
                os.remove(source + ".tdm")
            subprocess.run([exe, f"{package}/{name}.map"], cwd=editor_dir,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            if not os.path.exists(source + ".tdm"):
                failed.append(f"{package}/{name}: not compiled")
                continue
            for ext, dirs in ((".tdm", (levels_root, server_dir)), (".lua", (levels_root, server_dir)), (".hsf", (levels_root,))):
                for d in dirs:
                    target = os.path.join(d, package) if d == levels_root else d
                    shutil.copy2(source + ext, target)
            print(f"compiled {package}/{name}")
    for f in failed:
        print("FAILED " + f)
    return not failed


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("install", help="the install folder (holds TDClient, TDServer, TDEditor)")
    parser.add_argument("--pack", help="the content pack's folder")
    parser.add_argument("--no-compile", action="store_true", help="don't compile the quests")
    args = parser.parse_args()
    for part in PARTS:
        if not os.path.isdir(os.path.join(args.install, part)):
            sys.exit(f"{args.install} has no {part}: install the game there first")
    if args.pack:
        copy_pack(args.pack, args.install)
        settings = read_pack_ini(args.pack)
        if "appName" in settings:
            set_app_name(args.install, settings["appName"])
    if not args.no_compile and not compile_quests(args.install):
        sys.exit(1)


if __name__ == "__main__":
    main()
