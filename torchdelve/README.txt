 ==================================================================
|                         Torchdelve                               |
|------------------------------------------------------------------|
|                   A turn-based dungeon crawler                   |
|                                                                  |
| Copyright 2006-2026 by Marco Staginski                           |
| http://www.captain-online.de                                     |
|------------------------------------------------------------------|
| File: README                                                     |
 ==================================================================

See LICENSE.txt for the license and CREDITS.md for the fonts and other
works by others.


==================================== THE GAME ==================================

The town of Cindermere sits over old salt mines. Deep below, the Lantern
Wardens keep the Hearthflame burning, and its light holds the shadow of
Umbrath down. Someone has stolen the flame's ember-heart. Up to four heroes
go down after it, through eight quests, from the Wardens' training cellars to
the Hearthflame Deep.

Torchdelve is played over a network: a server runs the game, and every player
joins it with a client. The heroes are played by the players; one player may
play Umbrath, who moves the monsters and sees the whole board.


1. Starting
 Build and install the game as README.md in the repository describes. The
 programs are in the install folder: TDClient (the game), TDServer (the
 server) and TDEditor (compiles the quests).

 To play on one computer, start TDClient and click "Play local": it starts a
 server next to it and connects. To play over a network, start the server on
 one computer:
   TDServer <port>          (28405 if you have no other wish)
 and connect each client to it with the server's address and the port.
 The first player who connects is the game's admin: only they can load a quest.

 Then log in. The accounts are in the server's players.dat, a name and a
 password per line; edit it with any text editor while the server isn't
 running. The client fills in the account "captain" (password "Enterprise").

2. Choosing a quest
 The admin chooses the quest in the "Level" list and clicks Load. Every client
 shows the quest's introduction and then the board.

3. Heroes
 Click "Create hero", choose a class, give the hero a name (no spaces) and,
 for a spell caster, the spell classes. The heroes are saved on your computer
 (Windows: %APPDATA%\torchdelve, Linux: ~/.local/share/torchdelve) and can
 play any quest; they keep their gold and their equipment.

   Class     Attack  Defence  Health  Will  Spell classes  Starts with
   Warrior      3       3        7      2         -        Falchion
   Dwarf        2       3        8      2         -        Hand Axe
   Ranger       2       2        6      4         1        Sidesword
   Mage         1       1        5      6         3        Oak Staff

 Attack and Defence are the number of combat dice. Health is how much damage
 a hero can take; at 0 the hero dies. Will resists the spells that bind the
 mind (sleep, dread). The Dwarf can disarm traps without tools.

 The red crosses on the board are the heroes' starting fields. Click one,
 enter the name of one of your heroes and click Play. A player may place
 several heroes. To play Umbrath, click "Play Umbrath".

 Before the start, "Shop" sells equipment for the heroes' gold.

 When everyone is placed, someone clicks Start. Nobody can join after that.

4. A turn
 The game tells everyone whose turn it is. On your turn:
 - Move: rolls the movement dice (two for every hero); you may walk that many
   fields. The number at the top right shows the fields left. Walk with
   W, S, A, D, in the 3D view or on the map.
 - Open Door: opens the door you stand in front of and look at.
 - Attack: attacks the creature in front of you. With a weapon that reaches
   further (Dirk, Oak Staff, Halberd, Arbalest), click the target on the map.
 - Search: for secret doors, for treasure (once per room, with no monster in
   sight) or for traps.
 - Trap: search for traps, disarm a found trap (the Dwarf, or anyone with Trap
   Tools) or jump over it.
 - Inventory (or I): use an item or cast a spell (click the target on the
   map), take off or drop equipment, read what an item is.
 - Other: who is on a field; pick up dropped items.
 - End turn.
 A hero may move once and take one action (an attack, a spell, a search) per
 turn, in either order; opening doors is free.

 Combat: the attacker rolls their attack dice, the defender their defence dice
 ("Defend"). The combat die has three Blades, two Torches and one Shadow. Every
 Blade the attacker rolls is a hit; a hero blocks a hit with every Torch, a
 monster with every Shadow. Each hit that isn't blocked costs 1 health.

 Spells: a Ranger knows the spells of one class, a Mage those of three: ember
 (fire), stone, gale (wind) and tide (water). Each spell can be cast once per
 quest. The shadow spells in the data are meant for Umbrath, but the game
 doesn't give them to him yet. "What is" in the inventory tells what a spell
 or an item does.

 Each quest has its own goal; its introduction tells it. Usually the heroes
 must reach an exit, which opens only when the goal is met.

5. Views and keys
   M or Q      switch between the map and the 3D view
   F           in the 3D view: free the mouse pointer, or let it turn the view
   W S A D     walk
   I           the inventory
   C           Umbrath: the next monster
   ~           the console (Escape closes it)
   Escape      quit (asks first)

6. The console
 Everything can also be done by typing commands. "help" lists those that can
 be used at the moment. The most useful:
   : <message>                   a message to all players
   :<player> <message>           a message to one player
   list players | inventory | shop
   create hero
   create game <package> <level>   load a quest (the admin)
   play <hero>  |  play umbrath
   start, move, end turn
   open <direction>              w, s, a or d: up, down, left, right on the map
   attack <x> <y>, defend
   spell <x> <y> <id>, use <item> [<x> <y>]
   secretdoor, treasure, trap, disarm <direction>, jump <direction>
   whois <x> <y>, whatis <item>, takeoff <item>, drop <item>, pickup
   buy <item>, sell <item>
   quit


============================= MAKING QUESTS AND CONTENT =========================

1. Quests
 A quest is a layout (TDEditor/basic/<quest>.lvl), a script
 (TDEditor/basic/<quest>.lua) and a line in the level lists.
 - The layout is a grid with a character per field (# rock, . corridor, a
   letter per room) plus lines for doors, starting fields, monsters,
   furniture, traps and script fields. The docstring of tools/make_levels.py
   describes it, and TDEditor/basic/template.lvl is a small example.
   python tools/make_levels.py writes the .map files the editor compiles and
   checks every quest (every field reachable, every room with a door, every
   script letter defined, ...); --preview <folder> draws the boards.
 - The script tells the introduction, sets the quest up and reacts to events:
   a hero walking onto a field, searching a room, a monster's death. The
   quests in TDEditor/basic show how; template.lua is the smallest.
 - TDClient/levels/basic/levels.dat lists the quests the client offers (the
   number of quests first).
 - python tools/install_pack.py <install folder> compiles all quests of the
   lists with TDEditor and copies them where the client and the server load
   them.

2. Monsters, heroes, equipment, spells
 The game's content is data and Lua scripts in TDClient:
   data/heros.dat         the classes
   data/monsters.dat      the monsters (their code in the layouts is "short")
   data/armory.dat        equipment and its price (-1: not sold)
   data/items.dat         potions and other items
   data/spells.dat        the spells and their classes (data/spellClasses.dat)
   data/trade0.dat        what the shop sells
   scripts/*.lua          what equipment, items, spells, treasure and traps do
 The server loads the same scripts from TDServer/scripts. The scripts run in
 Lua 5.2.

3. Art
 The models, textures and icons are generated by the tools in tools/
 (make_models.py, make_textures.py, make_map_art.py, make_ui_art.py) from
 simple shapes and the SVG sources in art/. A new item needs an icon model in
 make_ui_art.py; a model is an OBJ with one texture.

4. Content packs
 A content pack replaces parts of the game's content without touching it: a
 folder laid out like the game (TDClient/, TDServer/, TDEditor/) with only the
 files it changes, and an optional pack.ini (appName = <folder of the saves>).
 Install it into an install folder of its own:
   cmake --install build/windows --config Release --prefix build/install/windows-<pack>
   python tools/install_pack.py build/install/windows-<pack> --pack <pack folder>
 A pack's data may name the game's art: "model: <file>" in heros.dat (after
 the weapon) and monsters.dat (after the mind), and a second word in
 textures/items/icons.dat names the icon to use. The docstring of
 tools/install_pack.py has the details.


================================ HOW IT WORKS ==================================

The world is a grid of fields; a field knows its walls and doors and holds up
to two objects (a creature passing another one stands on its field for a
moment). Rooms are lists of fields.

Visibility: inside a room everything is visible; in a corridor, what a
straight line reaches without crossing a wall or a closed door. What one hero
has seen stays visible for all.

The editor turns a level into 3D geometry: a floor tile per field, walls set
in from the field's border (their thickness), door frames, and the furniture's
and monsters' models.

The 3D view draws the fields in a cone in front of the camera that stops at
walls and closed doors. The camera collides with the walls of its field and
the fields next to it.

The server decides everything (dice, damage, scripts' results) and sends it to
the clients; the clients run the same scripts to show the results.
