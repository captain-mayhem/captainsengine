#include "Menu.h"
#include "Engine.h"

using namespace adv;

#define MENU_WIDTH 246
#define MENU_HEIGHT 240
#define DEPTH_MENU 10000

Menu::Menu() : RoomObject(1, 
      Vec2i(Engine::instance()->getSettings()->resolution.x/2-MENU_WIDTH/2, Engine::instance()->getSettings()->resolution.y/2-MENU_HEIGHT/2), 
      Vec2i(MENU_WIDTH, MENU_HEIGHT), "#menu", Vec2i()) {
  Color bg(Engine::instance()->getSettings()->backgroundcolor);
  bg += 16;
  Color line(Engine::instance()->getSettings()->bordercolor);
  Color text(Engine::instance()->getSettings()->textcolor);
  setBackground("#menu_bg", DEPTH_MENU+DEPTH_BACKGROUND);

  ButtonObject* loadBut = new ButtonObject(Vec2i(183,20), Vec2i(51,20), "LOAD", 11, true);
  loadBut->setColors(bg, line, line, text);
  loadBut->setFont(0, true);
  addObject(loadBut);
  Engine::instance()->getInterpreter()->execute(loadBut->getScript(), false);

  ButtonObject* saveBut = new ButtonObject(Vec2i(183,50), Vec2i(51,20), "SAVE", 12, true);
  saveBut->setColors(bg, line, line, text);
  saveBut->setFont(0, true);
  addObject(saveBut);
  Engine::instance()->getInterpreter()->execute(saveBut->getScript(), false);

  ButtonObject* newBut = new ButtonObject(Vec2i(183,100), Vec2i(51,20), "NEW", 13, true);
  newBut->setColors(bg, line, line, text);
  newBut->setFont(0, true);
  addObject(newBut);
  Engine::instance()->getInterpreter()->execute(newBut->getScript(), false);

  ButtonObject* quit = new ButtonObject(Vec2i(183,130), Vec2i(51,20), "QUIT", 14, true);
  quit->setColors(bg, line, line, text);
  quit->setFont(0, true);
  addObject(quit);
  Engine::instance()->getInterpreter()->execute(quit->getScript(), false);

  const char* script = 
    "on(enter){\n"
    "  setnum(!button; 0)\n"
    "}\n"
    "\n"
    "on(loop1){\n"
    "  if_num(!button; 11){\n"
    "    loadgame(1)\n"
    "  }\n"
    "  if_num(!button; 11){\n"
    "    savegame(1)\n"
    "  }\n"
    "  if_num(!button; 13)\n"
    "    restart()\n"
    "  if_num(!button; 14)\n"
    "    quit()\n"
    "}\n";
  ExecutionContext* scr = Engine::instance()->getInterpreter()->parseProgram(script);
  setScript(scr);
  Engine::instance()->getInterpreter()->execute(scr, false);
  scr->setEvent(EVT_ENTER);
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
      else if (y == 210)
         toSet = &line;
      else
        toSet = &bg;
      img->getData()[y*MENU_WIDTH*3+x*3] = toSet->r;
      img->getData()[y*MENU_WIDTH*3+x*3+1] = toSet->g;
      img->getData()[y*MENU_WIDTH*3+x*3+2] = toSet->b;
    }
  }
  return img;
}

void Menu::addButton(const char* text, int id){

}
