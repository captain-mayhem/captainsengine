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

  char tmp[512];
  const char* slotscript = 
    "on(click){\n"
    "  setobj(!button[!saveSlot]; 1)\n"
    "  setnum(!saveSlot; %i)\n"
    "  setobj(!button[!saveSlot]; 2)\n"
    "}\n"
    "\n"
    "on(doubleclick){\n"
    "  setobj(!button[!saveSlot]; 1)\n"
    "  setnum(!saveSlot; %i)\n"
    "  setobj(!button[!saveSlot]; 2)\n"
    "  entertext([!saveLabel] ; %i; %i; 0; 18; %i; %i; %i; save%i)\n"
    "}\n";
  Vec2i slotpos(3, 4);
  for (int i = 1; i <= 10; ++i){
    sprintf(tmp, "save%i", i);
    ButtonObject* slot = new ButtonObject(slotpos, Vec2i(170,20), tmp, i);
    slot->BlitObject::setDepth(DEPTH_MENU);
    slot->setColors(bg, line, line, text);
    slot->setFont(0, true);
    sprintf(tmp, slotscript, i, i,
      Engine::instance()->getSettings()->resolution.x/2-MENU_WIDTH/2+slotpos.x,
      Engine::instance()->getSettings()->resolution.y/2-MENU_HEIGHT/2+slotpos.y,
      line.r, line.g, line.b,
      i);
    ExecutionContext* slotscr = Engine::instance()->getInterpreter()->parseProgram(tmp);
    slot->getScript()->unref();
    slot->setScript(slotscr);
    addObject(slot);
    Engine::instance()->getInterpreter()->execute(slot->getScript(), false);
    slotpos.y += 18;
  }

  ButtonObject* loadBut = new ButtonObject(Vec2i(183,20), Vec2i(51,20), "LOAD", 11);
  loadBut->setColors(bg, line, line, text);
  loadBut->setFont(0, true);
  addObject(loadBut);
  Engine::instance()->getInterpreter()->execute(loadBut->getScript(), false);

  ButtonObject* saveBut = new ButtonObject(Vec2i(183,50), Vec2i(51,20), "SAVE", 12);
  saveBut->setColors(bg, line, line, text);
  saveBut->setFont(0, true);
  addObject(saveBut);
  Engine::instance()->getInterpreter()->execute(saveBut->getScript(), false);

  ButtonObject* newBut = new ButtonObject(Vec2i(183,100), Vec2i(51,20), "NEW", 13);
  newBut->setColors(bg, line, line, text);
  newBut->setFont(0, true);
  addObject(newBut);
  Engine::instance()->getInterpreter()->execute(newBut->getScript(), false);

  ButtonObject* quit = new ButtonObject(Vec2i(183,130), Vec2i(51,20), "QUIT", 14);
  quit->setColors(bg, line, line, text);
  quit->setFont(0, true);
  addObject(quit);
  Engine::instance()->getInterpreter()->execute(quit->getScript(), false);

  const char* script = 
    "on(enter){\n"
    "  setnum(!button; 0)\n"
    "  setnum(!saveSlot; 1)\n"
    "  setobj(!button[!saveSlot]; 2)\n"
    "}\n"
    "\n"
    "on(loop1){\n"
    "  if_num(!button; 11){\n"
    "    setnum(!button; 0)\n"
    "    loadgame([!saveSlot])\n"
    "  }\n"
    "  if_num(!button; 12){\n"
    "    setnum(!button; 0)\n"
    "    savegame([!saveSlot])\n"
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
