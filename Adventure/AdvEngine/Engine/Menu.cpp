#include "Menu.h"
#include "Engine.h"

using namespace adv;

//#define MENU_WIDTH 246
//#define MENU_HEIGHT 240
#define MENU_WIDTH (4*4/*borders*/+12*mLineHeight)
#define MENU_HEIGHT (2*4/*borders*/+1/*line*/+12*mLineHeight/*inputs*/+1)

unsigned Menu::mLineHeight = 0;

Menu::Menu(int font) : RoomObject(1, Vec2i(), Vec2i(), "#menu", Vec2i(), false) {
  mLineHeight = Engine::instance()->getFontRenderer()->getFont(font)->getLineHeight();
  mSize = Vec2i(MENU_WIDTH, MENU_HEIGHT);
  mPos = Vec2i(Engine::instance()->getSettings()->resolution.x / 2 - MENU_WIDTH / 2, Engine::instance()->getSettings()->resolution.y / 2 - MENU_HEIGHT / 2);
  Color bgorig(Engine::instance()->getSettings()->backgroundcolor);
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
    "  setstring(!button%iLabel; [empty])\n"
    "  entertext([!saveLabel] ; %i; %i; %i; %i; %i; %i; %i; [!saveName%i])\n"
    "  setstring(!button%iLabel; [!saveLabel])\n"
    "  setstring(!saveName%i; [!saveLabel])\n"
    "}\n";
  Vec2i slotpos(4, 4);
  int slotlength = (int)(170.0f / 20 * mLineHeight);
  for (int i = 1; i <= 10; ++i){
    sprintf(tmp, "save%i", i);
    ButtonObject* slot = new ButtonObject(slotpos, Vec2i(slotlength,mLineHeight), tmp, i);
    slot->BlitObject::setDepth(DEPTH_MENU);
    slot->setColors(bg, line, line, text);
    slot->setFont(font, true);
    sprintf(tmp, slotscript, i, i, i,
      Engine::instance()->getSettings()->resolution.x/2-MENU_WIDTH/2+slotpos.x,
      Engine::instance()->getSettings()->resolution.y/2-MENU_HEIGHT/2+slotpos.y,
      font,
      18,
      line.r, line.g, line.b,
      i, i, i);
    ExecutionContext* slotscr = Engine::instance()->getInterpreter()->parseProgram(tmp);
    slot->getScript()->unref();
    slot->setScript(slotscr);
    addObject(slot);
    Engine::instance()->getInterpreter()->execute(slot->getScript(), false);
    slotpos.y += mLineHeight;
  }

  std::vector<Vec2i> bi;
  unsigned butlen = Engine::instance()->getFontRenderer()->getTextExtent("LOAD", font, bi, 10000).x+4;
  bi.clear();
  int butpos = slotlength + (MENU_WIDTH - slotlength - butlen) / 2;
  ButtonObject* loadBut = new ButtonObject(Vec2i(butpos, mLineHeight + 4), Vec2i(butlen, mLineHeight), "LOAD", 11);
  loadBut->setColors(bg, line, line, text);
  loadBut->setFont(font, true);
  addObject(loadBut);
  Engine::instance()->getInterpreter()->execute(loadBut->getScript(), false);

  ButtonObject* saveBut = new ButtonObject(Vec2i(butpos, (int)(mLineHeight*2.5f) + 4), Vec2i(butlen, mLineHeight), "SAVE", 12);
  saveBut->setColors(bg, line, line, text);
  saveBut->setFont(font, true);
  addObject(saveBut);
  Engine::instance()->getInterpreter()->execute(saveBut->getScript(), false);

  ButtonObject* newBut = new ButtonObject(Vec2i(butpos, 5 * mLineHeight + 4), Vec2i(butlen, mLineHeight), "NEW", 13);
  newBut->setColors(bg, line, line, text);
  newBut->setFont(font, true);
  addObject(newBut);
  Engine::instance()->getInterpreter()->execute(newBut->getScript(), false);

  ButtonObject* quit = new ButtonObject(Vec2i(butpos, (int)(mLineHeight*6.5f) + 4), Vec2i(butlen, mLineHeight), "QUIT", 14);
  quit->setColors(bg, line, line, text);
  quit->setFont(font, true);
  addObject(quit);
  Engine::instance()->getInterpreter()->execute(quit->getScript(), false);

  unsigned textlen = Engine::instance()->getFontRenderer()->getTextExtent("Textspeed: Normal", font, bi, 10000).x + 4;
  ButtonObject* speed = new ButtonObject(Vec2i((MENU_WIDTH-textlen)/2, 10*mLineHeight+4+1), Vec2i(textlen, mLineHeight), "Textspeed: Normal", 15);
  speed->setColors(bgorig, bgorig, line, text);
  speed->setFont(font, true);
  addObject(speed);
  Engine::instance()->getInterpreter()->execute(speed->getScript(), false);

  const char* script = 
    "on(enter){\n"
    "  setnum(!button; 0)\n"
    "  loadstring(!saveName1)\n"
    "  loadstring(!saveName2)\n"
    "  loadstring(!saveName3)\n"
    "  loadstring(!saveName4)\n"
    "  loadstring(!saveName5)\n"
    "  loadstring(!saveName6)\n"
    "  loadstring(!saveName7)\n"
    "  loadstring(!saveName8)\n"
    "  loadstring(!saveName9)\n"
    "  loadstring(!saveName10)\n"
    "  if_num(!saveSlot; 0){\n"
    "    setnum(!saveSlot; 1)\n"
    "    setstring(!textSpeed; Normal)\n"
    "  }\n"
    "  if_string(!saveName1; none){\n"
    "    setstring(!saveName1; save1)\n"
    "    setstring(!saveName2; save2)\n"
    "    setstring(!saveName3; save3)\n"
    "    setstring(!saveName4; save4)\n"
    "    setstring(!saveName5; save5)\n"
    "    setstring(!saveName6; save6)\n"
    "    setstring(!saveName7; save7)\n"
    "    setstring(!saveName8; save8)\n"
    "    setstring(!saveName9; save9)\n"
    "    setstring(!saveName10; save10)\n"
    "  }\n"
    "  setobj(!button[!saveSlot]; 2)\n"
    "  setstring(!button1Label; [!saveName1])\n"
    "  setstring(!button2Label; [!saveName2])\n"
    "  setstring(!button3Label; [!saveName3])\n"
    "  setstring(!button4Label; [!saveName4])\n"
    "  setstring(!button5Label; [!saveName5])\n"
    "  setstring(!button6Label; [!saveName6])\n"
    "  setstring(!button7Label; [!saveName7])\n"
    "  setstring(!button8Label; [!saveName8])\n"
    "  setstring(!button9Label; [!saveName9])\n"
    "  setstring(!button10Label; [!saveName10])\n"
    "  setstring(!button15Label; Textspeed: [!textSpeed])\n"
    "}\n"
    "\n"
    "on(loop1){\n"
    "  if_num(!button; 11){\n"
    "    setnum(!button; 0)\n"
    "    loadgame([!saveSlot])\n"
    "  }\n"
    "  if_num(!button; 12){\n"
    "    setnum(!button; 0)\n"
    "    savestring(!saveName1)\n"
    "    savestring(!saveName2)\n"
    "    savestring(!saveName3)\n"
    "    savestring(!saveName4)\n"
    "    savestring(!saveName5)\n"
    "    savestring(!saveName6)\n"
    "    savestring(!saveName7)\n"
    "    savestring(!saveName8)\n"
    "    savestring(!saveName9)\n"
    "    savestring(!saveName10)\n"
    "    savegame([!saveSlot])\n"
    "  }\n"
    "  if_num(!button; 13)\n"
    "    restart()\n"
    "  if_num(!button; 14)\n"
    "    quit()\n"
    "  if_num(!button; 15){\n"
    "    setnum(!button; 0)\n"
    "    if_string(!textSpeed; Normal)\n"
    "      setstring(!tmp; Fast)\n"
    "    if_string(!textSpeed; Fast)\n"
    "      setstring(!tmp; Slow)\n"
    "    if_string(!textSpeed; Slow)\n"
    "      setstring(!tmp; Normal)\n"
    "    setstring(!textSpeed; [!tmp])\n"
    "    setstring(!button15Label; Textspeed: [!textSpeed])\n"
    "    textspeed([!textSpeed])\n"
    "  }\n"
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
  for (unsigned y = 0; y < MENU_HEIGHT; ++y){
    for (unsigned x = 0; x < MENU_WIDTH; ++x){
      if (x == 0 || x == MENU_WIDTH-1 || y == 0 || y == MENU_HEIGHT-1 ||
        ((x == 3 || x == MENU_WIDTH-1-3) && (y >= 3 && y <= MENU_HEIGHT-1-3)) || 
        ((y == 3 || y == MENU_HEIGHT-1-3) && (x >= 3 && x <= MENU_WIDTH-1-3)))
        toSet = &line;
      else if (x == 1 || x == 2 || x == MENU_WIDTH-2 || x == MENU_WIDTH-3 ||
        y == 1 || y == 2 || y == MENU_HEIGHT-2 || y == MENU_HEIGHT-3)
        toSet = &black;
      else if (y == MENU_HEIGHT-4-mLineHeight)
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
