 ==================================================================
|                         Hero Quest                               |
|------------------------------------------------------------------|
|                 The revival of the board game                    |
|                                                                  |
| Copyright 2006 by Marco Staginski                                |
| http://www.captain-online.de                                     |
|------------------------------------------------------------------|
| File: README                                                     |
 ==================================================================

See LICENSE.TXT for details

===================================== MANUAL ===================================

Calling the programs:

The editor:
HQEdit-Linux <filename>.map   or   editor.exe <filename>.map
The server:
HQServer-Linux <port>   or   server.exe <port>
The client:
HeroQuest-Linux   or   heroquest.exe


What you should know when using HeroQuest:

HeroQuest is a pure multiplayer game, so there must be a server running
somewhere in order to do anything with HeroQuest.
1. Running server / connecting to server
 If no server is running, start one, for example in a shell on the same computer. You
 must specify a port (If you don't know which one, use 28405. That is the default for HeroQuest).
 Now, that you have a running server, open up another shell and start HeroQuest.
 A screen appears where you have to enter server location and port. If you started the server
 as described above, the standard settings are just fine (localhost, 28405). If not, enter
 appropriate values. ATTENTION: The first player that connects to the server, gains admin status
 and is the only one who can create a game later on.
2. Logging in on the server
 Everyone who connected to the server has to log in now. Login-name / password pairs are stored
 in the players.dat of the server. You can add / remove / change them to your liking by writing
 this file directly with any text-editor. Currently, there are two accounts:
 name: captain       password: Enterprise
 name: tester        password: Test
 You can use one of them or create your own account (by editing players.dat when the server is not running) 
 when you want to login. captain/Enterprise is the default value when starting up. 
 If you don't want to be captain, type in another name and password. Click on Login and now you should be logged in.
3. Creating the game
 This is pretty easy: You must have admin status (see 1.) and then you can choose the level by
 clicking on the level button (the second from above). If you like, what you have chosen, 
 simply click on Load and every connected client shows then the map of the level.
4. Creating heros
 In order to play, you need heros. For the captain account, there already exist following heros: "Aragorn",
 "Gimli", "Legolas" and "Gandalf"; for the tester account there exists "Gimli". So if you want to use one
 of them (or you created your own heros earlier), you can skip this step. Otherwise, do the
 following. Open up the game console by pressing the ~ button on your keyboard. Type "create hero"
 and press Return. Then the different character classes are listed. Just type the number of the
 class you want and press again Return. After that, you have to enter the name that you want to give
 to your new hero (no spaces in the name allowed). Press again Return and the hero should now exist.
 Create as many heros as you want, but when being finished, press Escape to close the console.
5. Placing heros
 On the game map, you see those funny red crosses. These are hero starting positions. When you want
 to play a hero, click on a field with the cross on it. An input field and a button appear. Click on the
 input field and enter the name of the hero you want to play. The name you type in must be that of
 an existing hero (see 4.). Be aware of the fact that the name is case-sensitive. Having typed the
 name, press the play button. If everything was done correctly, the cross disappears and you
 see a picture of your hero on the chosen field. Place as many heros as you want this way (Of course, only
 as long as free starting positions are available).
6. Starting the game, Views
 After everyone who wants to play one or more heros has positioned their characters, someone has to
 press the start button on the lower right side of the screen. ATTETION: After someone had pressed
 this button, it is no longer possible for anyone to position other heros on free starting positions.
 The start button disappears, three other buttons appear and the game is now running. You are now able
 to switch between the 2D map view (in which you are in the beginning) and the 3D view of the world by
 pressing the M or the Q key. In the 3D view, there are two modes. One, in which you change the view
 of the camera by simply moving the mouse, and one in which you have a mouse pointer for clicking on buttons.
 You can switch between those two modes with the F key.
7. Within the game
 After the game has started (see 6.), a message appears for 10 seconds in the upper left corner whose turn it is.
 If it is not your turn, you can't do anything but watching. But as soon as the turn comes to you, you can do
 many things. When you want to move, click the move button (either in 2D map view or in 3D view with mouse
 pointer present). You will roll two dice and you can walk so many fields as you have diced points. The walking
 is done (be sure to be in 3D view now!) with the W, S, A, D keys as it is in many 3D games. The number in the
 upper right corner shows how many fields you can walk. Every time you cross a field border, the number of
 moves left decreases and if you reach zero, you can't cross the border to the next field. You can only roll
 the dice one time per turn. However, within the field you are currently on, it is always possible to move, but
 you gain of cource nothing by doing so.
 To open a door, you must stand on a field directly adjacent to the door and your eyes have to look straight
 to the door. Then simply click on the open door button and the door will disappear. You can open as many doors
 as you want per turn (If you are able to reach them).
 You can always in your turn click on the end turn button. When doing so, unused moves expire, you can't
 do anything more and the next character gets its turn.
8. Summary of console commands
 The game console is opened up with the TAB key and closed with Escape.
 
-v0.1 added commands:
 "help"                        gets you a situation dependent help of availible commands
 "connect <server> <port>"     connects HeroQuest to a server
 "login <loginname>"           logs in on the server. You are required to type the login password after this command.
 "list players"                lists all players, that are currently on the server
 ": <message>"                 chat <message> to all players that are on the server
 ":<player> <message>"         chat <message> to the player whose name is <player>.
 "create hero"                 see 4. of the usage instructions
 "create game <pack> <level>"  loads <level> from level-package <pack>.
 "play <character>"            use <character> for playing. You are asked to position him on a field.
      You can also type in "play zargon", that is the evil sorcerer. Then you can see the whole map from beginning.
 "start"                       starts the game
 "move"                        rolls the dice
 "w"                           move up
 "s"                           move down
 "a"                           move left
 "d"                           move right
      You should not use these console movement commands. They are for DEBUG/DEVELOPMENT purposes ONLY.
      They bypass collision detection and have not many security checks. They are there to be able to
      reach a desired position quickly against every rule. For normal movement, move as described in 7.
      i.e. in 3D view, press W S A D keys with the console being closed.
 "end turn"                    ends your turn
 "quit"                        quits HeroQuest

-v0.2 added commands:
 "open <direction>"            opens a door of the current field in <direction>. For <Direction> 
      you can use w, s, a or d with the same meaning as in the movement console commands.
 "whois <x> <y>"               displays infos about the gameobject at <x><y>
 "attack <x> <y>"              attacks creature at <x><y>
 "defend"                      creature tries to defend an attack

-v0.3 added commands:
 "secretdoor"                  searches for secret doors in the current room/floor
 "spell <x> <y> <id>"          casts spell with the spellid <id> to location <x><y>

-v0.4 added commands:
 "list inventory"              lists the content of the inventory of the current player
 "use <item> <x> <y>"          uses an item from the inventory

-v0.5 added commands:
 "treasure"                    searches for treasure in a room
 "whatis <item>"               displays information about an item
 "takeoff <item>"              takes off a wearable item that is currently worn
 "spawn <item>"                spawn an item into the inventory

9. Final remarks:
 When testing the game, you should at least run two HeroQuest clients on two different computers and
 connect them both to the same server to be able to see and test all features of the game.
 If you don't like the green debug output on the upper left side of the screen, press the F1 key to
 disable it.


What you should know when modifying HeroQuest:

1. The .map file format for the editor
 The first line has to look as follows:  Mapsize: <width> x <height>
 Then, the map is drawn in ascii-art. An empty 3 x 2 map would for example look like this:
 . . . . . . . .
 .    .    .    .
 .    .    .    .
 . . . . . . . .
 .    .    .    .
 .    .    .    .
 . . . . . . . .
 Horizontal walls are denoted by |----| or ------, depending if the wall touches vertical walls or not.
 Vertical walls are always drawn like this: |
                                            |
                                            |
                                            |
 Horizontal doors are written as DDDD for normal and SSSS for secret doors on the line where also walls would be.
 Vertical doors are written as D or S similar to horizontal doors.
                               D    S
 In the fields is enough space for 8 characters:
 . . .
 .TAaS.
 .OoRr.
 . . .
 T = type of object that is standing on the field, e.g. M (monster); F (furniture)
 Aa = specific object id of the object on the field, e.g. zo (zombie); th (throne)
 S = special action that is associated with the field, e.g. X (hero starting position)
 Oo = Object on the field, that is not blocking the field, e.g. st (stair). Traps are also encoded here.
 Rr = RoomId, e.g. 04. If no roomId is specified, the field belongs to the corridor.
 
 Every valid map has to have walls all around the outer border and needs at least one hero
 starting position. So the simplest valid map file would look the following:
 Mapsize: 1 x 1
 |----|
 |   X|
 |    |
 |----|
 ATTENTION: The editor reacts very sensitive to withespaces on the end of a line, it compiles
   more or less everything, even if it is not a valid map. The client and server will then probably react
   with segmentation fault. This is not very nice, but as the proposal says, the editor will not
   be very user friendly. The person, who creates maps has to make absolutely sure that everything on the map 
   is correct.
   In the future, when the editor will understand more commands and the maps can therefore become more complex, 
   a parser will of course be added, but that is currently not urgently neccessary.
2. Usage of the editor
 If you have written a valid map file, just type HQ-Edit <file> and the editor will create a .hqm file
 out of it. To play the map, copy the .hqm file in the levels/basic/ directory of the client and
 the server. The structure of the .hqm file is explained in the section data structures of this README file.
3. startup and texture files
 In the data/ directory of HeroQuest lies a file called heroquest.ini. There you can change the
 default settings, with which server name, port, player name and passwort the game will start up. There
 is a heros.dat which lists all hero character classes with its attributes. All spells are listed in the spells.dat and 
 so on. In this directory are also binary stored the created heros(.sav files).
 In the levels/basic directory, you can find a levels.dat. All levels, that should be choosable through
 the GUI must be entered here. The number at the beginning of the file has to match the number of entries
 in this file.
 In the scripts/ directory are several lua-Scripts located. They control many aspects of the
 game behaviour, i.e. treasure searching, armory types and usage, traps, ...
 In the /Textures directory are different sub-directories, e.g. World, Heros, ... There are all the
 textures that are needed for startup. Which textures will be loaded is written in the .dat files in
 each directory. You need as many texture ids in the World/ground.dat as you have different room ids in the
 created level files. In the wall.dat the first id is the wall, the second the door and the third the
 ceiling texture. In the Heros/hero.dat the texture ids correspond to the hero ids written in the data/heros.dat.
 In the Other directory, the texture is just the background startup image. ATTENTION: All textures need width
 and height that is a power of 2. Do not use other file formats than there already are in the
 corresponding .dat file!

Now, you should be able to play HeroQuest, create new levels and change textures to your liking. So have fun!


================================= IMPLEMENTATION ===============================

Hardware/Software dependencies:
The main program (Client):
This program needs a hardware-accelerated graphics card (GeForce4 or better recommended)
because rendering the level with textures, heros, walls, etc. is a lot of work for the GPU. 
The CPU however is not critical: 1 GHz or better should be enough.
HeroQuest is built upon the platform-independent SDL library, so currently it can run on Linux or UNIX platforms 
and on windows 2000/XP. Moreover, the game uses the jpeg-library for loading jpgs. If neccessary 
(e.g. when porting HeroQuest to other platforms), it can be compiled again from the jpeg.tar.gz and copied back 
to the heroquest directory.
The server:
It makes no use of graphics, so the graphics card in the system is irrelevant for the server. But
it uses sockets ans SDL-threads heavily. So it should run on almost every Linux/UNIX/Windows-system.
However, if many players join the server, RAM and CPU have to be chosen sufficiently high.
The editor:
It has no dependencies and should run on every system that has a C++ compiler. However, the editor
generates binary files, so the byte-order and alignment of this machine has to be the same as on the 
system where the client and the server will run.

Explanations to important functions and data structures:
1. the structure of the world
 the world is a two-dimensional array of Fields. A Field contains informations about where doors and walls
 are, stored as Bitsets, and a Field has two pointers to GameObjects. Apart from the doorbits, there exists
 also a doorindex which leads quickly to the Door object, if you need further information about the door. 
 The Field has also a tempVis Bitset that is used for visibility calculations. Of couse, the vertices and 
 texture coordinates of the 3D geometry are also stored in the Fields as simple arrays.
 The world is also organized as rooms, i.e. there is a vector of roomIds and each of them contains a vector of
 Field pointers, which point to certain Fields in the big world array. With that structure, you can quickly
 find all Fields that belong to a specific room.
2. Map visibility
 The HeroQuest rules say, that in the same room, everything is visible and on the corridor what you can see
 using a straight line that does not intersect walls or closed doors. What one hero has uncovered is visible
 for all heros forever. The algorithm works now as follows. If the current position is in a room that is
 not yet visible, make all fields active (i.e. visible), that are in that room using the room structure
 of the world. On a corridor, do a eight-connectivity seedfill that stops, if the field was already found 
 or no straight line is possible from here to the stating point of the algorithm or if it reaches a room.
 To determine, if a straight line is possible, the line is rasterized to the field grid using bresenham.
 It returns false if walls come in between from one step to the next.
3. Generating 3D geometry
 It's easy to explain, what's going on, but difficult to understand how exactly it is done. This process
 takes place in the editor. It gererates a quad for the ground of each field. If a wall is present, it
 gererates a wall quad with a predefined height. This wall quad is one unit nearer to the center of 
 the ground quad as the border of the ground quad. This gives the wall thickness. For doors, there are
 also appropriate quads generated. However, the devil lies in the details. The neighbourhood must be taken
 into consideration to determine, how long the wall may be exactly. There are many different cases for
 different lengthes and positions that must be chosen correctly. Due to that, the code is almost impossible 
 to read. The only chance to see, what is exactly going on is to take a piece of paper and follow the algorithm 
 by drawing the situation. Many sheets of paper also lost their lives as I wrote that code. ;-)
4. Rendering the 3D world
 The first step that the algorithm makes is to determine, which Fields are currently visible. The core idea
 behind it is to rasterize a 2D version of the view frustum on the world grid. If there are no walls or
 closed doors present, that's exactly what it does. It starts on the current field, goes two fields sideways in
 each direction and goes one forward. With each forward step, it may go two more steps sideways. This leads
 to the following pattern:    ooxoo
                            ooooooooo
			  ooooooooooooo
			      ...
 It stops, when it is gone ten steps forward. What is forward is determined by the camera. However, if
 there are occluding walls or doors, the algorithm stops there. It may only go three steps sideways
 without being gone foreward. That is to prevent the algorithm to go wide around distant corners.
 After the potetial visible set (PVS) is approximated this way, it is just rendered. All object that are
 on Fields in the PVS are also rendered. In the end, the PVS is cleared and the algorithm starts from
 the beginning.
5. Collision detection
 There is not one big collision detection method, but many function that contribute a part to it. In
 updateCollisionVertices from the world class, the geometry of the current field, the neighbouring fields 
 and the doors is taken, decomposed into triangles and put into an array.
 The Math class provides sphere-polygon-collision and a method to get collision offset.
 The first tests, if there are intersection points between a sphere and a plane and if this intersection is
 within the polygon. The second gets a offset that corrects the sphere's position according to the polygon normal.
 Equipped with these methods, it is easy for the camera, that has a bounding sphere, to check for collision
 with the collision triangles and to correct its position when collision took place.
6. Object positioning
 The HeroQuest rules say that on a field, only one object may be there at the same time. However, creatures
 of the same type (i.e. Hero - Hero or Monster - Monster) may pass, but only, if they finally can stay on a free
 field. So object moving is trivial when the field is free (just set pointers appropriate). But if on the
 field where the hero wants to go is another hero, he can go there temporarily by just using the second
 GameObject pointer. However, it could be, that he cannot go further (e.g no more moves left). Then the position
 of the hero is invalid and he has to be set back to the last valid (i.e. free) position. So if he enters an
 occupied field, a backpointer is set that points to the last valid field. It is cleared, if the hero reaches
 a new free field. If he ends his turn with the backpointer set, he is set back to that position the pointer
 tells.

 
Known bugs:
See TODO in HQClient

================================ The future of this project ======================================================
To do for next releases:
v0.5
- armory and treasures
- level init scripting (special monster names, introduction, ...)
v0.6
- level scripting (goals to reach, special monsters and level properties, ...)
- traps
- distance weapons like crossbow
v0.7
- extending the GUI (new elements, more control buttons)
- extend scripting to be ready for all basic levels
- trade system
v0.8
- all basic levels added
- campaign mode from level to level
v0.9
- artificial intelligence for the monsters
v1.0
- 3D models for heros, monsters and furniture
- Character animation
- graphical special effects (fire, spells, ...)

So have a look at the webpage www.captain-online.de/programming.php from time to time for further releases


Contribution:
If you like this project and you would like to contribute, please contact:
captain@captain-online.de
Currently, I am just one programmer, and there is always need for more.
And if you have some experience in 3D-modelling or 2D-art, your contribution would be highly appreciated.


**** END OF FILE ***************************************************************
