#ifndef IDS_H
#define IDS_H

#include <iostream>

namespace adv{

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

#define DEPTH_ROOM_RANGE 1000
#define DEPTH_PARALLAX_BACKGROUND -5
#define DEPTH_PARTICLES_BACK -4
#define DEPTH_BACKGROUND -3
#define DEPTH_BACK_OBJECTS -2
#define DEPTH_MIRROR_BACK -1
#define DEPTH_PARTICLES_MIDDLE 0
#define DEPTH_PARTICLES_FRONT 989
#define DEPTH_FRONT_OBJECTS 990
#define DEPTH_ITEM 991
#define DEPTH_BOUND_FONT 992
#define DEPTH_LIGHTING 993
#define DEPTH_SCREENCHANGE 995
#define DEPTH_COIN_MENU 10000
#define DEPTH_MENU 11000
#define DEPTH_PARTICLES_TOP 18500
#define DEPTH_GAME_FONT 19500
#define DEPTH_BUTTON 19750
#define DEPTH_UI_FONT 19875
#define DEPTH_VIDEO_LAYER 19900
#define DEPTH_CURSOR 20000
#define DEPTH_CONSOLE 30000

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

struct Color{
  Color(){
    r = 255; g=255; b=255; a=255;
  }
  Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a){
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }
  Color(unsigned packedcolor){
    r = (packedcolor >> 0) & 0xFF;
    g = (packedcolor >> 8) & 0xFF;
    b = (packedcolor >> 16) & 0xFF;
    a = 255;
  }
  Color& operator*=(const Color& color){
    r = (unsigned char)((r/255.0f*color.r/255.0)*255.0);
    g = (unsigned char)((g/255.0f*color.g/255.0)*255.0);
    b = (unsigned char)((b/255.0f*color.b/255.0)*255.0);
    a = (unsigned char)((a/255.0f*color.a/255.0)*255.0);
    return *this;
  }
  Color& operator+=(const int factor){
    int tmp = r+factor; if (tmp > 255) tmp = 255; r = (unsigned char)tmp;
    tmp = g+factor; if (tmp > 255) tmp = 255; g = (unsigned char)tmp;
    tmp = b+factor; if (tmp > 255) tmp = 255; b = (unsigned char)tmp;
    tmp = a+factor; if (tmp > 255) tmp = 255; a = (unsigned char)tmp;
    return *this;
  }
  Color& operator+=(const Color& color){
    int tmp = r + color.r; if (tmp > 255) tmp = 255; r = (unsigned char)tmp;
    tmp = g + color.g; if (tmp > 255) tmp = 255; g = (unsigned char)tmp;
    tmp = b + color.b; if (tmp > 255) tmp = 255; b = (unsigned char)tmp;
    tmp = a + color.a; if (tmp > 255) tmp = 255; a = (unsigned char)tmp;
    return *this;
  }
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

std::ostream& operator<<(std::ostream& strm, const Color& color);
std::istream& operator>>(std::istream& strm, Color& color);

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

}

#endif
