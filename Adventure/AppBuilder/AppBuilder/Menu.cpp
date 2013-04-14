#include "Menu.h"
#include "Engine.h"

using namespace adv;

#define MENU_WIDTH 246
#define MENU_HEIGHT 240
#define DEPTH_MENU 10000

Menu::Menu() : RoomObject(1, 
      Vec2i(Engine::instance()->getSettings()->resolution.x/2-MENU_WIDTH/2, Engine::instance()->getSettings()->resolution.y/2-MENU_HEIGHT/2), 
      Vec2i(MENU_WIDTH, MENU_HEIGHT), "#menu", Vec2i()) {
  setBackground("#menu_bg", DEPTH_MENU+DEPTH_BACKGROUND);
}

CGE::Image* Menu::getBackground(){
  CGE::Image* img = new CGE::Image(3, MENU_WIDTH, MENU_HEIGHT);
  img->allocateData();
  Color bg(Engine::instance()->getSettings()->backgroundcolor);
  Color line(Engine::instance()->getSettings()->bordercolor);
  Color black(0);
  Color* toSet = &bg;
  for (int y = 0; y < MENU_HEIGHT; ++y){
    for (int x = 0; x < MENU_WIDTH; ++x){
      if (x == 0 || x == MENU_WIDTH-1 || y == 0 || y == MENU_HEIGHT-1 ||
        ((x == 3 || x == MENU_WIDTH-1-3) && (y >= 3 && y <= MENU_HEIGHT-1-3)) || 
        ((y == 3 || y == MENU_HEIGHT-1-3) && (x >= 3 && x <= MENU_WIDTH-1-3)))
        toSet = &line;
      else if (x == 1 || x == 2 || x == MENU_WIDTH-2 || x == MENU_WIDTH-3 ||
        y == 1 || y == 2 || y == MENU_HEIGHT-2 || y == MENU_HEIGHT-3)
        toSet = &black;
      else
        toSet = &bg;
      img->getData()[y*MENU_WIDTH*3+x*3] = toSet->r;
      img->getData()[y*MENU_WIDTH*3+x*3+1] = toSet->g;
      img->getData()[y*MENU_WIDTH*3+x*3+2] = toSet->b;
    }
  }
  return img;
}
