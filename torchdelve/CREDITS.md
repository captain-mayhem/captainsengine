# Credits for Torchdelve's assets

Every asset made by others is listed here: what it is, the source, the author, the license, and whether the game must credit it.

## Fonts
- **Cinzel Decorative Bold** (`art/fonts/CinzelDecorative-Bold.ttf`) by Natanael Gama, from the Google Fonts repository (github.com/google/fonts, `ofl/cinzeldecorative`). License: SIL Open Font License 1.1 (`art/fonts/OFL-CinzelDecorative.txt`); Reserved Font Name "Cinzel". Used for the logo and the subtitle on the start screen. The font itself isn't shipped with the game, only the rendered picture. A credit in the game isn't required.

## Made for Torchdelve
- **The start screen** (`TDClient/textures/other/start.jpg`), drawn as `art/start.svg` and rendered by `tools/make_start_screen.py` (2026-11-25).

- **The dungeon's textures** (`TDClient/textures/world/`: `floorWall.jpg`, `ceil2.jpg`, `floor.jpg`, `room1..22.jpg`, `door.jpg`), painted by `tools/make_textures.py` (2026-11-25).
- **The heroes, the monsters and the start field** (`TDClient/models/heros/*`, `TDClient/models/monsters/*`, `TDClient/models/world/startpos.*`: OBJ, MTL and `_skin.png`), built by `tools/make_models.py` and `tools/figures.py` (2026-11-25).
- **The furniture, the floor overlays, the triggered traps and the script marker** (`TDClient/models/furniture/*`, `models/overlays/*`, `models/world/pit.*`, `models/world/rocktrap.*`, `models/other/script.*`), built by `tools/make_models.py` and `tools/furniture.py` (2026-11-25).

- **The 2D map's tiles and tokens** (`TDClient/textures/map/*`, `textures/heros/*.jpg`, `textures/monsters/*.jpg`, `textures/furniture/*.jpg`, `textures/overlays/*.jpg`, `textures/other/items.jpg`, `textures/other/script.jpg`, `textures/world/startpos1.jpg`), painted and rendered by `tools/make_map_art.py` (2026-11-25).

- **The interface's art** (`TDClient/textures/other/inventory.jpg`, `shop.jpg`, `textures/items/*`), painted and rendered by `tools/make_ui_art.py` (2026-11-25); its titles use Cinzel Decorative (above).

## The world's geometry
- The walls, door frames, doors and floor tile (`TDClient/models/world/wall*.obj`, `door*.obj`, `ground.obj`) are the game's own simple geometry from 2006; they carry the textures above.
