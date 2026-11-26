"""Builds Torchdelve's quest maps from compact layouts.

A quest is three files in TDEditor/basic:
  <quest>.lvl   the layout (written by hand, read here)
  <quest>.map   the ASCII map TDEditor compiles (written here; 5 x 3 characters per field)
  <quest>.lua   the quest's script (written by hand; checked here)

The .lvl format (lines starting with # are comments):
  title: The Quest's Name
  rooms: a=3 b=12 ...      the floor colour (room1..22) of each room letter; letters that
                           aren't listed get the free ids in order
  cells:                   then H lines of W characters, one per field ('#' rock, '.' corridor,
                           a letter a room), up to "end"; the doors are given by door lines
  door x y <RIGHT|BOTTOM|LEFT|TOP> [S]   a door (S: a secret door) on that side of (x,y)
  size: W H                or, instead of cells: the board starts as rock, and
  rect <c> x y w h         lines paint fields (c: '.', '#' or a room letter) in their order,
                           so later rects cut pillars or add L-shapes
  grid:                    or, instead of cells: 2H-1 lines for a board of W x H fields:
    field rows (even lines): 2W-1 characters, the fields at the even columns:
      '#' solid rock, '.' corridor, a letter a room (one letter per room)
      and between two fields (odd columns): ' ' automatic, 'D' door, 'S' secret door,
      '|' a wall even where the automatic rule has none
    wall rows (odd lines): under each field (even columns): ' ' automatic, 'D', 'S',
      '-' a wall; the odd columns are ignored (write '+' or ' ')
    Automatic walls: between fields of different kinds (rock, corridor, one room and
    another), and around the board. Rock next to rock has none.
  end
  start x y                a hero's start field
  monster <code> x y [S]   go sk zo or bb mu ak ga dc; S an optional script letter
  furniture <code> x y <top|right|bottom|left> [S]
                           ab bc ch cb fp ra st ta th to wr, its back against that wall;
                           x y is its first field: the corner at the back wall's left end
                           as seen from the room (top: top-left, right: top-right,
                           bottom: bottom-right, left: bottom-left)
  overlay <code> x y [S]   st (stairs, 2 x 2 from x y), pt rt sp (traps)
  script S x y [x y ...]   a script letter on fields (A-Z, a-z: a-d are the traps of
                           scripts/misc.lua, e-m are reserved for them, n-z and A-Z the quest's)

The checks: every open field reachable from the starts (doors and secret doors open),
every room with a door, furniture inside one room against its wall, nothing on top of
something else, every script letter handled by the .lua's eventScheduler and defined there,
and every openDoor(x, y, "DIR") of the .lua naming a door of the map.

Usage: python tools/make_levels.py [quest ...] [--preview <folder>]
  without quests, all .lvl files in TDEditor/basic.
"""

import argparse
import glob
import os
import re
import sys
from collections import deque

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
BASIC = os.path.join(ROOT, "TDEditor", "basic")
MAPTILES = os.path.join(ROOT, "TDClient", "textures", "map")

MONSTERS = {"go", "sk", "zo", "or", "bb", "mu", "ak", "ga", "dc"}
# code -> (width along the wall, depth into the room)
FURNITURE = {"ab": (3, 2), "bc": (3, 1), "ch": (1, 1), "cb": (3, 1), "fp": (3, 1), "ra": (3, 2),
             "st": (3, 2), "ta": (3, 2), "th": (1, 1), "to": (3, 2), "wr": (3, 1)}
OVERLAYS = {"st": 2, "pt": 1, "rt": 1, "sp": 1}
SIDES = ("top", "right", "bottom", "left")
# the letters' case gives the wall (see Compiler::calcTrafo)
CASE = {"top": (str.lower, str.lower), "right": (str.lower, str.upper),
        "bottom": (str.upper, str.lower), "left": (str.upper, str.upper)}
DIRS = {"TOP": (0, -1), "RIGHT": (1, 0), "BOTTOM": (0, 1), "LEFT": (-1, 0)}


class LevelError(Exception):
  pass


def footprint(code, x, y, side):
  """The fields a furniture piece covers, and for each field on its back row the wall side."""
  w, d = FURNITURE[code]
  if side == "top":
    return [(x + i, y + j) for i in range(w) for j in range(d)], [((x + i, y), "TOP") for i in range(w)]
  if side == "right":
    return [(x - j, y + i) for i in range(w) for j in range(d)], [((x, y + i), "RIGHT") for i in range(w)]
  if side == "bottom":
    return [(x - i, y - j) for i in range(w) for j in range(d)], [((x - i, y), "BOTTOM") for i in range(w)]
  return [(x + j, y - i) for i in range(w) for j in range(d)], [((x, y - i), "LEFT") for i in range(w)]


class Level:
  def __init__(self, path):
    self.name = os.path.splitext(os.path.basename(path))[0]
    self.path = path
    self.title = self.name
    self.roomids = {}
    self.starts = []
    self.monsters = []   # (code, x, y)
    self.furniture = []  # (code, x, y, side)
    self.overlays = []   # (code, x, y)
    self.scripts = {}    # (x, y) -> letter
    self.doors = []      # (x, y, dir, secret) from door lines
    self.size = None
    self.rects = []
    self.parse()

  def fail(self, msg):
    raise LevelError("%s: %s" % (self.name, msg))

  def parse(self):
    with open(self.path, encoding="utf-8") as f:
      lines = [l.rstrip("\n").rstrip("\r") for l in f]
    grid = None
    i = 0
    while i < len(lines):
      line = lines[i]
      i += 1
      if line.startswith("#") or not line.strip():
        continue
      if line.startswith("title:"):
        self.title = line[6:].strip()
      elif line.startswith("rooms:"):
        for item in line[6:].split():
          k, v = item.split("=")
          self.roomids[k] = int(v)
      elif line.startswith("size:"):
        self.size = tuple(int(v) for v in line[5:].split())
      elif line.strip() == "cells:":
        rows = []
        while lines[i].strip() != "end":
          rows.append(lines[i].strip())
          i += 1
        i += 1
        grid = []
        for k, r in enumerate(rows):
          grid.append(" ".join(r))
          if k < len(rows) - 1:
            grid.append(" " * (2 * len(r) - 1))
      elif line.strip() == "grid:":
        grid = []
        while lines[i].strip() != "end":
          grid.append(lines[i].rstrip())
          i += 1
        i += 1
      else:
        self.entity(line.split())
    if grid is None and self.size:
      W, H = self.size
      cells = [["#"] * W for _ in range(H)]
      for c, x, y, w, h in self.rects:
        for j in range(y, y + h):
          for i in range(x, x + w):
            if not (0 <= i < W and 0 <= j < H):
              self.fail("rect %s %d %d %d %d leaves the board" % (c, x, y, w, h))
            cells[j][i] = c
      grid = []
      for k, r in enumerate(cells):
        grid.append(" ".join(r))
        if k < H - 1:
          grid.append(" " * (2 * W - 1))
    if grid is None:
      self.fail("no grid")
    self.read_grid(grid)
    for x, y, d, secret in self.doors:
      dx, dy = DIRS[d]
      nx, ny = x + dx, y + dy
      ch = "S" if secret else "D"
      if not (0 <= nx < self.w and 0 <= ny < self.h):
        self.fail("door at (%d,%d) %s leads off the board" % (x, y, d))
      if dx:
        self.vsep[y][min(x, nx)] = ch
      else:
        self.hsep[min(y, ny)][x] = ch

  def entity(self, t):
    kind = t[0]
    def opt_script(n, x, y):
      if len(t) > n:
        self.add_script(t[n], x, y)
    if kind == "start":
      self.starts.append((int(t[1]), int(t[2])))
    elif kind == "monster":
      if t[1] not in MONSTERS:
        self.fail("unknown monster " + t[1])
      x, y = int(t[2]), int(t[3])
      self.monsters.append((t[1], x, y))
      opt_script(4, x, y)
    elif kind == "furniture":
      if t[1] not in FURNITURE or t[4] not in SIDES:
        self.fail("bad furniture line " + " ".join(t))
      x, y = int(t[2]), int(t[3])
      self.furniture.append((t[1], x, y, t[4]))
      opt_script(5, x, y)
    elif kind == "overlay":
      if t[1] not in OVERLAYS:
        self.fail("unknown overlay " + t[1])
      x, y = int(t[2]), int(t[3])
      self.overlays.append((t[1], x, y))
      opt_script(4, x, y)
    elif kind == "rect":
      self.rects.append((t[1], int(t[2]), int(t[3]), int(t[4]), int(t[5])))
    elif kind == "door":
      if t[3] not in DIRS:
        self.fail("bad door line " + " ".join(t))
      self.doors.append((int(t[1]), int(t[2]), t[3], len(t) > 4 and t[4] == "S"))
    elif kind == "script":
      coords = [int(v) for v in t[2:]]
      for k in range(0, len(coords), 2):
        self.add_script(t[1], coords[k], coords[k + 1])
    else:
      self.fail("unknown line " + " ".join(t))

  def add_script(self, letter, x, y):
    if len(letter) != 1 or not letter.isalpha() or letter in "X":
      self.fail("bad script letter %r" % letter)
    if (x, y) in self.scripts:
      self.fail("two scripts on (%d,%d)" % (x, y))
    self.scripts[(x, y)] = letter

  def read_grid(self, g):
    if len(g) % 2 == 0:
      self.fail("the grid needs an odd number of lines")
    self.h = (len(g) + 1) // 2
    self.w = (max(len(l) for l in g) + 1) // 2
    g = [l.ljust(2 * self.w - 1) for l in g]
    self.cells = [[g[2 * y][2 * x] for x in range(self.w)] for y in range(self.h)]
    for y in range(self.h):
      for x in range(self.w):
        c = self.cells[y][x]
        if not (c in "#." or c.isalpha()):
          self.fail("bad field %r at (%d,%d)" % (c, x, y))
    # vertical separators right of (x,y); horizontal ones below (x,y)
    self.vsep = [[g[2 * y][2 * x + 1] for x in range(self.w - 1)] for y in range(self.h)]
    self.hsep = [[g[2 * y + 1][2 * x] for x in range(self.w)] for y in range(self.h - 1)]
    letters = sorted({c for row in self.cells for c in row if c.isalpha()})
    used = set(self.roomids.values())
    free = [n for n in range(1, 23) if n not in used]
    for l in letters:
      if l not in self.roomids:
        if not free:
          self.fail("more than 22 rooms")
        self.roomids[l] = free.pop(0)
    for l, n in self.roomids.items():
      if not 1 <= n <= 22:
        self.fail("room id %d out of 1..22" % n)
    if len(set(self.roomids[l] for l in letters)) != len(letters):
      self.fail("two rooms share an id")

  # --- the edges between fields ---
  def cell(self, x, y):
    if 0 <= x < self.w and 0 <= y < self.h:
      return self.cells[y][x]
    return None

  def edge(self, x, y, d):
    """'wall', 'door', 'secret' or 'open' on side d (TOP/RIGHT/BOTTOM/LEFT) of (x,y)."""
    dx, dy = DIRS[d]
    nx, ny = x + dx, y + dy
    a, b = self.cell(x, y), self.cell(nx, ny)
    if b is None:
      return "wall"
    if dx:
      s = self.vsep[y][min(x, nx)]
    else:
      s = self.hsep[min(y, ny)][x]
    if s == "D":
      return "door"
    if s == "S":
      return "secret"
    if s in "|-":
      return "wall"
    if s != " " and s != "+":
      self.fail("bad separator %r next to (%d,%d)" % (s, x, y))
    if a == b:
      return "open"
    return "wall"

  # --- checks ---
  def check(self, lua):
    errors = []
    def err(m):
      errors.append(m)
    def open_field(x, y):
      c = self.cell(x, y)
      return c is not None and c != "#"
    # doors between open fields
    for y in range(self.h):
      for x in range(self.w):
        for d in ("RIGHT", "BOTTOM"):
          e = self.edge(x, y, d)
          if e in ("door", "secret"):
            dx, dy = DIRS[d]
            if not (open_field(x, y) and open_field(x + dx, y + dy)):
              err("door at (%d,%d) %s leads into rock or off the board" % (x, y, d))
    if not 1 <= len(self.starts) <= 8:
      err("needs start fields")
    # reachable
    seen = set(self.starts)
    q = deque(self.starts)
    while q:
      x, y = q.popleft()
      for d, (dx, dy) in DIRS.items():
        if self.edge(x, y, d) != "wall":
          n = (x + dx, y + dy)
          if n not in seen and open_field(*n):
            seen.add(n)
            q.append(n)
    for y in range(self.h):
      for x in range(self.w):
        if open_field(x, y) and (x, y) not in seen:
          err("(%d,%d) can't be reached" % (x, y))
          break
    # rooms have doors
    for l in self.roomids:
      fields = [(x, y) for y in range(self.h) for x in range(self.w) if self.cells[y][x] == l]
      if not fields:
        continue
      if not any(self.edge(x, y, d) in ("door", "secret") for x, y in fields for d in DIRS):
        err("room %s has no door" % l)
    # occupancy
    occupied = {}
    def occupy(pos, what):
      if not open_field(*pos):
        err("%s at %s is not on an open field" % (what, pos))
      elif pos in occupied:
        err("%s at %s overlaps %s" % (what, pos, occupied[pos]))
      else:
        occupied[pos] = what
    for p in self.starts:
      occupy(p, "start")
      if p in self.scripts:
        err("a start field can't hold a script: %s" % (p,))
    for code, x, y in self.monsters:
      occupy((x, y), "monster " + code)
    for code, x, y, side in self.furniture:
      fields, back = footprint(code, x, y, side)
      room = self.cell(x, y)
      if room is None or not room.isalpha():
        err("furniture %s at (%d,%d) not in a room" % (code, x, y))
        continue
      for p in fields:
        occupy(p, "furniture %s" % code)
        if self.cell(*p) != room:
          err("furniture %s at (%d,%d) leaves its room at %s" % (code, x, y, p))
      for p, d in back:
        if self.edge(p[0], p[1], d) != "wall":
          err("furniture %s at (%d,%d) %s: no wall behind %s" % (code, x, y, side, p))
    for code, x, y in self.overlays:
      n = OVERLAYS[code]
      for i in range(n):
        for j in range(n):
          if not open_field(x + i, y + j):
            err("overlay %s at (%d,%d) is not on open fields" % (code, x, y))
    # scripts and the lua
    handled = set(re.findall(r'script\s*==\s*"(\w)"', lua))
    defined = set(re.findall(r'^function\s+(\w)\s*\(', lua, re.M))
    for (x, y), s in sorted(self.scripts.items()):
      if s in "abcd":
        continue
      if "e" <= s <= "m":
        err("script %s at (%d,%d): e-m are reserved for traps" % (s, x, y))
      if s not in handled:
        err("script %s at (%d,%d) isn't handled by eventScheduler" % (s, x, y))
      if s not in defined:
        err("script %s isn't defined in the .lua" % s)
    for (x, y), s in self.scripts.items():
      if s in "abc" and not any(o[1] == x and o[2] == y for o in self.overlays):
        err("trap script %s at (%d,%d) has no trap overlay" % (s, x, y))
    for x, y, d in re.findall(r'openDoor\(\s*(\d+)\s*,\s*(\d+)\s*,\s*"(\w+)"', lua):
      if d not in DIRS or self.edge(int(x), int(y), d) not in ("door", "secret"):
        err("openDoor(%s, %s, %s) names no door" % (x, y, d))
    for fn in ("intro", "levelInit", "eventScheduler", "wandering_monster"):
      if not re.search(r'^function\s+%s\s*\(' % fn, lua, re.M):
        err("the .lua has no %s()" % fn)
    if errors:
      raise LevelError("%s:\n  %s" % (self.name, "\n  ".join(errors)))

  # --- the .map ---
  def write_map(self):
    W, H = self.w, self.h
    out = [[" "] * (5 * W + 1) for _ in range(3 * H + 1)]
    def wallchar(e, horizontal):
      return {"wall": "-" if horizontal else "|", "door": "D", "secret": "S", "open": " "}[e]
    for y in range(H):
      for x in range(W):
        top, bot, mid, low = 3 * y, 3 * y + 3, 3 * y + 1, 3 * y + 2
        c = self.cells[y][x]
        # horizontal walls (the whole run of 4 characters, plus corners)
        for row, d in ((top, "TOP"), (bot, "BOTTOM")):
          ch = wallchar(self.edge(x, y, d), True)
          if ch == " ":
            out[row][5 * x + 1] = "."
            out[row][5 * x + 3] = "."
          else:
            for k in range(1, 5):
              out[row][5 * x + k] = ch
        for col, d in ((5 * x, "LEFT"), (5 * x + 5, "RIGHT")):
          ch = wallchar(self.edge(x, y, d), False)
          out[mid][col] = ch if ch != " " else "."
          out[low][col] = ch if ch != " " else "."
        if c.isalpha():
          rid = "%02d" % self.roomids[c]
          out[low][5 * x + 3], out[low][5 * x + 4] = rid[0], rid[1]
    # corners: '|' where a vertical wall meets, '-' on a horizontal run
    for row in range(0, 3 * H + 1, 3):
      for col in range(0, 5 * W + 1, 5):
        up = row > 0 and out[row - 1][col] in "|DS"
        down = row < 3 * H and out[row + 1][col] in "|DS"
        left = col > 0 and out[row][col - 1] in "-DS"
        right = col < 5 * W and out[row][col + 1] in "-DS"
        out[row][col] = "|" if (up or down) else ("-" if (left or right) else " ")
    for y in range(H):
      for x in range(W):
        if self.cells[y][x] == "#":
          # rock: nothing inside; keep it visibly empty
          pass
    def put(x, y, line, off, s):
      r = 3 * y + line
      for k, ch in enumerate(s):
        out[r][5 * x + off + k] = ch
    for code, x, y in self.monsters:
      put(x, y, 1, 1, "M" + code)
    for code, x, y, side in self.furniture:
      fields, _ = footprint(code, x, y, side)
      lo, up = CASE[side]
      put(x, y, 1, 1, "F" + lo(code[0]) + up(code[1]))
      for p in fields[1:]:
        put(p[0], p[1], 1, 1, "*")
    for code, x, y in self.overlays:
      put(x, y, 2, 1, code)
    for (x, y), s in self.scripts.items():
      put(x, y, 1, 4, s)
    for x, y in self.starts:
      put(x, y, 1, 4, "X")
    text = "Mapsize: %d x %d\n" % (W, H) + "\n".join("".join(r) for r in out) + "\n"
    path = os.path.join(BASIC, self.name + ".map")
    with open(path, "w", newline="\r\n") as f:
      f.write(text)
    return path

  # --- a picture to look at ---
  def preview(self, folder):
    from PIL import Image, ImageDraw
    S = 32
    img = Image.new("RGB", (self.w * S + 2, self.h * S + 2), (10, 10, 12))
    tiles = {}
    def tile(name):
      if name not in tiles:
        tiles[name] = Image.open(os.path.join(MAPTILES, name)).convert("RGB").resize((S, S))
      return tiles[name]
    for y in range(self.h):
      for x in range(self.w):
        c = self.cells[y][x]
        if c == "#":
          continue
        t = tile("floor.png") if c == "." else tile("room%d.png" % self.roomids[c])
        img.paste(t, (x * S + 1, y * S + 1))
    dr = ImageDraw.Draw(img)
    for y in range(self.h):
      for x in range(self.w):
        if self.cells[y][x] == "#":
          continue
        for d in DIRS:
          e = self.edge(x, y, d)
          x0, y0 = x * S + 1, y * S + 1
          seg = {"TOP": (x0, y0, x0 + S, y0), "BOTTOM": (x0, y0 + S, x0 + S, y0 + S),
                 "LEFT": (x0, y0, x0, y0 + S), "RIGHT": (x0 + S, y0, x0 + S, y0 + S)}[d]
          if e == "wall":
            dr.line(seg, fill=(235, 225, 200), width=3)
          elif e == "door":
            dr.line(seg, fill=(200, 120, 40), width=6)
          elif e == "secret":
            dr.line(seg, fill=(170, 60, 200), width=6)
    for code, x, y, side in self.furniture:
      fields, _ = footprint(code, x, y, side)
      xs = [p[0] for p in fields]
      ys = [p[1] for p in fields]
      dr.rectangle((min(xs) * S + 5, min(ys) * S + 5, (max(xs) + 1) * S - 3, (max(ys) + 1) * S - 3),
                   fill=(90, 60, 30), outline=(240, 200, 120))
      dr.text((x * S + 8, y * S + 10), code, fill=(255, 255, 255))
    for code, x, y in self.overlays:
      n = OVERLAYS[code]
      dr.rectangle((x * S + 3, y * S + 3, (x + n) * S - 1, (y + n) * S - 1), outline=(120, 200, 255), width=2)
      dr.text((x * S + 6, y * S + 18), code, fill=(120, 200, 255))
    for code, x, y in self.monsters:
      dr.ellipse((x * S + 5, y * S + 5, x * S + S - 3, y * S + S - 3), fill=(150, 20, 20))
      dr.text((x * S + 10, y * S + 10), code, fill=(255, 255, 255))
    for x, y in self.starts:
      dr.ellipse((x * S + 7, y * S + 7, x * S + S - 5, y * S + S - 5), outline=(255, 220, 60), width=3)
    for (x, y), s in self.scripts.items():
      dr.text((x * S + S - 9, y * S + 2), s, fill=(120, 255, 120))
    out = os.path.join(folder, self.name + ".png")
    img.save(out)
    return out


def main():
  ap = argparse.ArgumentParser()
  ap.add_argument("quests", nargs="*")
  ap.add_argument("--preview")
  a = ap.parse_args()
  names = a.quests or sorted(os.path.splitext(os.path.basename(p))[0]
                             for p in glob.glob(os.path.join(BASIC, "*.lvl")))
  bad = False
  for n in names:
    try:
      lv = Level(os.path.join(BASIC, n + ".lvl"))
      luapath = os.path.join(BASIC, n + ".lua")
      lua = open(luapath, encoding="utf-8").read() if os.path.exists(luapath) else ""
      if a.preview:
        os.makedirs(a.preview, exist_ok=True)
        lv.preview(a.preview)
      lv.check(lua)
      path = lv.write_map()
      rooms = len({c for row in lv.cells for c in row if c.isalpha()})
      print("%-20s %2d x %2d, %2d rooms, %2d monsters -> %s" % (n, lv.w, lv.h, rooms, len(lv.monsters),
                                                           os.path.relpath(path, ROOT)))
    except LevelError as e:
      bad = True
      print(e, file=sys.stderr)
  sys.exit(1 if bad else 0)


if __name__ == "__main__":
  main()
