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
  BACK=0,
  FRONT=1,
  LEFT=2,
  RIGHT=3,
};

enum WindowIds{
  ID_AdvTreeCtrl = 1000,
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
