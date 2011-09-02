#ifndef IDS_H
#define IDS_H

enum ResourceID{
  SCRIPT=0,
  ITEM=1,
  MUSIC=2,
  OBJECT=3,
  IMAGE=4,
  ROOM=5,
  SOUND=6,
  FOLDER=11,
  CHARACTER=15,
};

enum LookDir{
  FRONT=0,
  BACK=1,
  RIGHT=2,
  LEFT=3,
  UNSPECIFIED=4,
};

enum WindowIds{
  ID_AdvTreeCtrl = 1000,
};

#define DEPTH_PARALLAX_BACKGROUND -4
#define DEPTH_PARTICLES_BACK -3
#define DEPTH_BACKGROUND -2
#define DEPTH_BACK_OBJECTS -1
#define DEPTH_PARTICLES_MIDDLE 0
#define DEPTH_PARTICLES_FRONT 989
#define DEPTH_FRONT_OBJECTS 990
#define DEPTH_ITEM 993
#define DEPTH_BOUND_FONT 994
#define DEPTH_LIGHTING 995
#define DEPTH_PARTICLES_TOP 18500
#define DEPTH_GAME_FONT 19500
#define DEPTH_BUTTON 19750
#define DEPTH_UI_FONT 19875
#define DEPTH_SCREENCHANGE 19900
#define DEPTH_CURSOR 20000

enum TaskPopup{
  TB_STATIC = 0,
  TB_DIRECT,
  TB_SCROLLING
};

enum TsStyle{
  TS_SOLID = 0,
  TS_TRANSPARENT,
  TS_ADDITIVE,
  TS_NONE
};

enum ScreenChange{
  SC_DIRECT = 0,
  SC_FADEOUT,
  SC_RECTANGLE,
  SC_CIRCLE,
  SC_SHUTTERS,
  SC_CLOCK,
  SC_BLEND,
  SC_BLEND_SLOW,
};

/* parallax scolling
//speed
var speed0:Number = 130;//higher = slower
var speed1:Number = 120;


//main
function scroller(target:MovieClip, speed:Number):Void
{
var xDistance:Number = _xmouse - Stage.width / 2;
var tDistance:Number = target._x;

tDistance += xDistance / speed;


if(tDistance >= 400)//was zero but now half the width of the stage (800)
{
tDistance = 400;
}

if(tDistance <= Stage.width - target._width)
{
tDistance = Stage.width - target._width;
}


setProperty(target, _x, tDistance);
updateAfterEvent();
}


//run scrolling
land0_mc.onEnterFrame = function()
{
scroller(this, speed0);
}

land1_mc.onEnterFrame = function()
{
scroller(this, speed1);
}
*/

#endif
