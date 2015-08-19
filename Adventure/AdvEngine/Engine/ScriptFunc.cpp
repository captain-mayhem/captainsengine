#include "ScriptFunc.h"

#include <fstream>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "Engine.h"
#include "Script.h"
#include "SaveStateProvider.h"
#include "Inventory.h"
#include "Sound.h"
#include "SoundPlayer.h"
#include "Screenchange.h"
#include "Particles.h"
#include "Textout.h"
#include "PostProcessing.h"
#include "ItemObject.h"
#include <system/allocation.h>

using namespace adv;

TR_CHANNEL(ADV_ScriptFunc);

void ScriptFunctions::registerFunctions(PcdkScript* interpreter){
  interpreter->registerFunction("loadroom", loadRoom);
  interpreter->registerFunction("setfocus", setFocus);
  interpreter->registerFunction("showinfo", showInfo);
  interpreter->registerFunction("walkto", walkTo);
  interpreter->registerRelVar("walkto", 2, "charx:_");
  interpreter->registerRelVar("walkto", 3, "chary:_");
  interpreter->registerFunction("speech", speech);
  interpreter->registerFunction("pickup", pickup);
  interpreter->registerFunction("playsound", playSound);
  interpreter->registerRelVar("playsound", 2, "_volume");
  interpreter->registerFunction("if_bool", isBoolEqual);
  interpreter->registerFunction("if_obj", isObjectInState);
  interpreter->registerFunction("setlight", setLight);
  interpreter->registerFunction("setbool", setBool);
  interpreter->registerFunction("if_command", isCommandSet);
  interpreter->registerFunction("setobj", setObj);
  interpreter->registerRelVar("setobj", 2, "obj:");
  interpreter->registerFunction("beamto", beamTo);
  interpreter->registerFunction("additem", addItem);
  interpreter->registerFunction("cutscene", cutScene);
  interpreter->registerFunction("if_link", isLinkedObject);
  interpreter->registerFunction("taskbar", taskbar);
  interpreter->registerFunction("follow", follow);
  interpreter->registerFunction("lookto", lookTo);
  interpreter->registerFunction("textscene", textScene);
  interpreter->registerFunction("if_givelink", isGiveLinkedObject);
  interpreter->registerFunction("delitem", delItem);
  interpreter->registerFunction("loopsound", loopSound);
  interpreter->registerFunction("loopstop", loopStop);
  interpreter->registerFunction("stopsound", loopStop);
  interpreter->registerFunction("playmusic", playMusic);
  interpreter->registerFunction("stopmusic", stopMusic);
  interpreter->registerFunction("wait", wait);
  interpreter->registerFunction("subroom", subRoom);
  interpreter->registerFunction("return", subRoomReturn);
  interpreter->registerFunction("realreturn", subRoomReturnImmediate);
  interpreter->registerFunction("link", link);
  interpreter->registerFunction("givelink", giveLink);
  interpreter->registerFunction("if_num", isNumEqual);
  interpreter->registerFunction("setnum", setNum);
  interpreter->registerRelVar("setnum", 2, "");
  interpreter->registerFunction("offspeech", offSpeech);
  interpreter->registerFunction("unloadroom", unloadRoom);
  interpreter->registerFunction("restart", restart);
  interpreter->registerFunction("gotolevel", gotoLevel);
  interpreter->registerFunction("deactivate", deactivate);
  interpreter->registerFunction("endscene", endScene);
  interpreter->registerFunction("instobj", instObj);
  interpreter->registerFunction("command", command);
  interpreter->registerFunction("inv_down", invDown);
  interpreter->registerFunction("inv_up", invUp);
  interpreter->registerFunction("if_focus", isCharFocussed);
  interpreter->registerFunction("setfont", setFont);
  interpreter->registerFunction("setscreenchange", setScreenchange);
  interpreter->registerFunction("activate", activate);
  interpreter->registerFunction("savegame", saveGame);
  interpreter->registerFunction("loadgame", loadGame);
  interpreter->registerFunction("jiggle", jiggle);
  interpreter->registerFunction("randomnum", randomNum);
  interpreter->registerFunction("setchar", setChar);
  for (int i = 2; i < 20; ++i){
    interpreter->registerRelVar("setchar", i, "_charstate");
  }
  interpreter->registerFunction("setstring", setString);
  interpreter->registerFunction("loadnum", loadNum);
  interpreter->registerFunction("savenum", saveNum);
  interpreter->registerFunction("textenabled", textEnabled);
  interpreter->registerFunction("realtime", realTime);
  interpreter->registerFunction("setcharlight", setCharLight);
  interpreter->registerFunction("group", group);
  interpreter->registerFunction("stopskip", stopSkip);
  interpreter->registerFunction("playswf", playSwf);
  interpreter->registerFunction("stopswf", stopSwf);
  interpreter->registerFunction("stopvideo", stopSwf);
  interpreter->registerFunction("stopavi", stopSwf);
  interpreter->registerFunction("if_ischar", isCharTriggering);
  interpreter->registerFunction("if_charin", isCharInRoom);
  interpreter->registerFunction("if_hasitem", isCharPossessingItem);
  interpreter->registerFunction("setwalkmap", setWalkmap);
  interpreter->registerFunction("if_keydown", isKeyDownEqual);
  interpreter->registerFunction("if_keypressed", isKeyPressedEqual);
  interpreter->registerFunction("if_string", isStringEqual);
  interpreter->registerFunction("stepto", stepTo);
  interpreter->registerFunction("moveobj", moveObj);
  interpreter->registerRelVar("moveobj", 2, "tgtobjx:");
  interpreter->registerRelVar("moveobj", 3, "tgtobjy:");
  interpreter->registerFunction("quit", quit);
  interpreter->registerFunction("musicvolume", musicVolume);
  interpreter->registerFunction("setparticles", setParticles);
  interpreter->registerFunction("startparticles", startParticles);
  interpreter->registerFunction("stopparticles", stopParticles);
  interpreter->registerFunction("function", function);
  interpreter->registerFunction("stopfunction", stopFunction);
  interpreter->registerFunction("speechvolume", speechVolume);
  interpreter->registerFunction("setlanguage", setLanguage);
  interpreter->registerFunction("if_room", isCurrentRoom);
  interpreter->registerFunction("entertext", enterText);
  interpreter->getArgumentAsExecutionContext("entertext", 1);
  interpreter->registerFunction("if_mousewheel", isMouseWheelEqual);
  interpreter->registerFunction("fadespeed", fadeSpeed);
  interpreter->registerFunction("seteax", setEAX);
  interpreter->registerFunction("bindtext", bindText);
  interpreter->registerFunction("textout", textOut);
  interpreter->getArgumentAsExecutionContext("textout", 2);
  for (int i = 2; i < 6; ++i){
    interpreter->registerRelVar("textout", i, "_charstate");
  }
  interpreter->registerFunction("textspeed", textSpeed);
  interpreter->registerFunction("setpos", setPos);
  interpreter->registerFunction("minicut", miniCut);
  interpreter->registerFunction("minicutend", miniCutEnd);
  interpreter->registerFunction("break", breakExec);
  interpreter->registerFunction("particleview", particleView);
  interpreter->registerFunction("texthide", textHide);
  interpreter->registerFunction("playavi", playVideo);
  interpreter->registerFunction("playvideo", playVideo);
  interpreter->registerFunction("moviewait", wait);
  interpreter->registerFunction("stopavi", stopSwf);
  interpreter->registerFunction("stopeffect", stopEffect);
  interpreter->registerFunction("starteffect", startEffect);
  interpreter->registerFunction("if_xobj", isObjXPosEqual);
  interpreter->registerFunction("linkchar", linkChar);
  interpreter->registerFunction("stopzooming", stopZooming);
  interpreter->registerFunction("unlinkchar", unlinkChar);
  interpreter->registerFunction("savestring", saveString);
  interpreter->registerFunction("showmouse", showMouse);
  interpreter->registerFunction("loadstring", loadString);
  interpreter->registerFunction("charzoom", charZoom);
  interpreter->registerFunction("setwalksound", setWalkSound);
  interpreter->registerFunction("if_yobj", isObjYPosEqual);
  interpreter->registerFunction("hidealltext", hideAllTexts);
  interpreter->registerFunction("enablemouse", enableMouse);
  interpreter->registerFunction("set_rect_walkmap", setRectWalkmap);
  interpreter->registerFunction("exchange", exchange);
  interpreter->registerFunction("enablemenu", enableMenu);
  interpreter->registerFunction("settransparency", setTransparency);
  interpreter->registerFunction("showalltext", showAllText);
  interpreter->registerFunction("instmouse", instMouse);
  interpreter->registerFunction("showinventory", showInventory);
  interpreter->registerFunction("if_item", isItemInState);
  interpreter->registerFunction("setobjlight", setObjLight);
  interpreter->registerFunction("textalign", textAlign);
  interpreter->registerFunction("runspeed", runSpeed);
  interpreter->registerFunction("runto", runTo);
  interpreter->registerRelVar("runto", 2, "charx:_");
  interpreter->registerRelVar("runto", 3, "chary:_");
  interpreter->registerFunction("enablefxshape", enableFXShape);
  interpreter->registerFunction("scrollspeed", scrollSpeed);
  interpreter->registerFunction("loadchar", loadChar);
  interpreter->registerFunction("offtextcolor", offTextColor);
  interpreter->registerFunction("setitem", setItem);
  interpreter->registerFunction("sqrt", sqrt);
  interpreter->registerFunction("switchchar", switchCharacter);
  interpreter->registerFunction("movetext", moveText);
  interpreter->registerRelVar("movetext", 2, "_txtoutx:");
  interpreter->registerRelVar("movetext", 3, "_txtouty:");
  srand((unsigned)time(NULL));
}

#define ARG_MAX 20

#define NUM_ARGS(argmin, argmax) \
lua_pushthread(L);\
lua_gettable(L, LUA_REGISTRYINDEX);\
ExecutionContext& ctx = *(ExecutionContext*)lua_touserdata(L, -1);\
unsigned numArgs = ctx.stack().numArgs();\
lua_pop(L, 1);\
TR_USE(ADV_ScriptFunc);\
if (numArgs < argmin || numArgs > argmax){\
TR_BREAK("Unexpected number of arguments (%i)", numArgs);\
}

int ScriptFunctions::loadRoom(lua_State* L){
  NUM_ARGS(1, 2);
  std::string room = ctx.stack().get(1).getString();
  ScreenChange change = Engine::instance()->getScreenChange();
  if (numArgs == 2){
    std::string changename = ctx.stack().get(2).getString();
    change = getScreenChange(changename);
  }
  Engine::instance()->loadMainRoom(room, &ctx, change);
  return 0;
}

int ScriptFunctions::setFocus(lua_State* L){
  NUM_ARGS(1, 1);
  std::string character = ctx.stack().get(1).getString();
  Engine::instance()->setFocus(character, &ctx);
  return 0;
}

int ScriptFunctions::showInfo(lua_State* L){
  NUM_ARGS(2, 2);
  std::string text = ctx.stack().get(1).getString();
  bool show = ctx.stack().get(2).getBool();
  if (show){
    Engine::instance()->setObjectTooltipString(text);
  }
  else
    Engine::instance()->setObjectTooltipString("");
  Engine::instance()->setObjectString(text);
  ctx.setObjectInfo(text); //update object info in case it is changing text
  return 0;
}

int ScriptFunctions::walkTo(lua_State* L){
  return moveTo(L, 1.0f);
}

int ScriptFunctions::moveTo(lua_State* L, float speedFactor){
  NUM_ARGS(3, 5);
  std::string character = ctx.stack().get(1).getString();
  Vec2i pos;
  pos.x = ctx.stack().get(2).getInt()-1;
  pos.y = ctx.stack().get(3).getInt()-1;
  LookDir dir = UNSPECIFIED;
  bool hold = true;
  if (numArgs >= 4){
    StackData sd = ctx.stack().get(4);
    if (sd.isString()){
      if (sd.getString() == "dontwait")
        hold = false;
    }
    else
      dir = (LookDir)(sd.getInt()-1);
  }
  if (numArgs >= 5){
    std::string dw = ctx.stack().get(5).getString();
    if (dw == "dontwait")
      hold = false;
  }
  CharacterObject* chr = ctx.getCharacter(character);
  if (chr){
    pos = pos * chr->getWalkGridSize() + 
      Vec2f(chr->getWalkGridSize()/2, chr->getWalkGridSize()/2);
    if (!ctx.isSkipping() && hold){
      chr->setSuspensionScript(&ctx);
      ctx.mSuspended = true;
    }
    Engine::instance()->walkTo(chr, pos, dir, speedFactor);
  }
  return 0;
}

int ScriptFunctions::speech(lua_State* L){
  NUM_ARGS(2, 4);
  std::string character = ctx.stack().get(1).getString();
  std::string text = ctx.stack().get(2).getString();
  std::string sound = "";
  bool hold = Engine::instance()->getInterpreter()->isBlockingScriptRunning() || ctx.isLoop1() || ctx.isIdle();
  if (numArgs >= 3){ //TODO SOUND
    sound = ctx.stack().get(3).getString();
    if (sound == "dontwait"){
      hold = false;
      sound = "";
    }
  }
  if (numArgs >= 4){
    std::string dw = ctx.stack().get(4).getString();
    if (dw == "dontwait")
      hold = false;
  }
  if (ctx.mSkip)
    return 0;
  CharacterObject* chr = ctx.getCharacter(character);
  FontRenderer::String* str = NULL;
  SoundPlayer* plyr = NULL;
  if (chr){
    //stop the current sound
    SoundEngine::instance()->removeSpeaker(chr);
    //create new one
    if (sound != ""){
      plyr = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_CREATE_ALWAYS);
      if (plyr){
        plyr->setVolume(SoundEngine::instance()->getSpeechVolume());
        plyr->play(false);
        plyr->setSpeaker(chr);
      }
    }
    std::vector<Vec2i> breakinfo;
    Vec2i pos = chr->getOverheadPos();
    if (!Engine::instance()->isTextEnabled())
      text = "";
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, chr->getFontID(), breakinfo);
    str = Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2,pos.y-ext.y, text, 
      DEPTH_GAME_FONT, chr->getFontID(), breakinfo, chr->getTextColor(), plyr ? 100000 : Engine::instance()->getInterpreter()->getTextSpeed()*(unsigned)text.length());
    //stop speaking
    Engine::instance()->getFontRenderer()->removeText(chr, false);
    if (str)
      str->setSpeaker(chr);
    chr->setTalking(true);

    if (hold){
      if (plyr){
        plyr->setSuspensionScript(&ctx);
      }
      else if (str){
        str->setSuspensionScript(&ctx);
      }
      if (!ctx.isIdle())
        Engine::instance()->setBlockingSpeaker(chr);
      ctx.mSuspended = true;
    }
  }
  else{
    TR_WARN("Character %s not found", character.c_str());
  }
  return 0;
}

int ScriptFunctions::pickup(lua_State* L){
  NUM_ARGS(1, 1);
  std::string character = ctx.stack().get(1).getString();
  if (ctx.mSkip)
    return 0;
  CharacterObject* chr = ctx.getCharacter(character);
  if (chr){
    LookDir dir = chr->getLookDir();
    if (dir == LEFT || dir == RIGHT)
      chr->setState(16);
    else if (dir == FRONT)
      chr->setState(15);
    chr->getAnimation()->registerAnimationEndHandler(chr);
    chr->addNextState(0);
  }
  return 0;
}

int ScriptFunctions::playSound(lua_State* L){
  NUM_ARGS(1, 2);
  std::string sound = ctx.stack().get(1).getString();
  unsigned volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().get(2).getInt();
  if (ctx.mSkip)
    return 0;
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_CREATE_ALWAYS);
  if (sp){
    sp->setVolume(volume/100.0f);
    sp->play(false);
  }
  return 0;
}

int ScriptFunctions::setLight(lua_State* L){
  NUM_ARGS(4, 5);
  std::string room = ctx.stack().get(1).getString();
  Color c;
  c.r = (unsigned char)ctx.stack().get(2).getInt();
  c.g = (unsigned char)ctx.stack().get(3).getInt();
  c.b = (unsigned char)ctx.stack().get(4).getInt();
  bool fade = false;
  if (numArgs >= 5)
    fade = ctx.stack().get(5).getString() == "fade";
  RoomObject* roomobj = Engine::instance()->getRoom(room);
  if (roomobj){
    if (fade && !ctx.mSkip){
      Engine::instance()->getAnimator()->add(roomobj, c);
    }
    else
      roomobj->setLightingColor(c);
  }
  else{
    SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(room);
    if (!sr)
      TR_ERROR("Room %s does not exist", room.c_str());
    else
      sr->overlaylighting = c;
  }
  return 0;
}

int ScriptFunctions::setBool(lua_State* L){
  NUM_ARGS(2, 2);
  String boolname = ctx.stack().get(1).getString();
  bool val = ctx.stack().get(2).getBool();
  lua_getglobal(L, "bool");
  lua_pushboolean(L, val);
  lua_setfield(L, -2, boolname.c_str());
  lua_pop(L, 1);
  return 0;
}

void ScriptFunctions::setObjInternal(std::vector<std::string> objects, std::vector<int> states, bool skip){
  TR_USE(ADV_ScriptFunc);
  for (unsigned objiter = 0; objiter < objects.size(); ++objiter){
    Object2D* obj = Engine::instance()->getObject(objects[objiter], false);
    SaveStateProvider::SaveObject* so = NULL;
    if (obj){
      int oldstate = obj->getState();
      obj->setState(states[0]);
      //re-evaluate on(mouse) when state changes
      /*Object2D* cursorobj = Engine::instance()->getObjectAt(Engine::instance()->getCursorPos());
      if (cursorobj && cursorobj->getScript() != NULL && cursorobj == obj && states[0] != oldstate){
        obj->getScript()->setEvent(EVT_MOUSE);
      }*/
    }
    else{
      std::string room;
      so = Engine::instance()->getSaver()->findObject(objects[objiter], room);
      if (so)
        so->state = states[0];
      else
        TR_ERROR("Object %s does not exist", objects[objiter].c_str());
    }
    for (unsigned i = 1; i < states.size(); ++i){
      int state = states[i];
      if (obj){
        if (skip)
          obj->setState(state);
        else{
          obj->addNextState(state);
          obj->getAnimation(states[i-1])->registerAnimationEndHandler(obj);
        }
      }
      else{
        so->state = state;
      }
    }
  }
}

int ScriptFunctions::setObj(lua_State* L){
  NUM_ARGS(2, ARG_MAX);
  std::string objname = ctx.stack().get(1).getString();
  //remove whitespaces in object names
  for(int size = (int)objname.size()-1; size >= 0; --size){
    if (objname[size] == ' ')
      objname.erase(size, 1);
  }
  //TR_DEBUG("obj: %s", objname.c_str());
  int state = ctx.stack().get(2).getInt();
  ObjectGroup* grp = Engine::instance()->getInterpreter()->getGroup(objname);
  std::vector<std::string> objects;
  if (grp)
    objects = grp->getObjects();
  else
    objects.push_back(objname);
  std::vector<int> states;
  states.push_back(state);
  for (unsigned i = 2; i < numArgs; ++i){
    state = ctx.stack().get(i+1).getInt();
    states.push_back(state);
  }
  setObjInternal(objects, states, ctx.isSkipping());
  return 0;
}

int ScriptFunctions::beamTo(lua_State* L){
  NUM_ARGS(3, 5);
  String charname = ctx.stack().get(1).getString();
  StackData arg = ctx.stack().top();
  std::string roomname;
  int getidx = 1;
  if (!arg.isString()){
    ++numArgs;
  }
  else
    roomname = ctx.stack().get(++getidx).getString();
  Vec2i pos;
  pos.x = ctx.stack().get(++getidx).getInt()-1;
  pos.y = ctx.stack().get(++getidx).getInt()-1;
  LookDir dir = UNSPECIFIED;
  if (numArgs >= 5)
    dir = (LookDir)(ctx.stack().get(++getidx).getInt()-1);
  bool isFocussedChar = false;
  CharacterObject* focussedChar = Engine::instance()->getCharacter("self");
  if (focussedChar){
    String realcharname = ctx.resolveCharName(charname);
    if(_stricmp(realcharname.c_str(), focussedChar->getName().c_str()) == 0)
      isFocussedChar = true;
  }
  if (isFocussedChar){
    //focussed char, therefore change room
    if (!roomname.empty())
      Engine::instance()->loadMainRoom(roomname, &ctx, Engine::instance()->getScreenChange());
    CharacterObject* obj = Engine::instance()->getCharacter(charname);
    if (obj){
      obj->abortClick();
      obj->setTalking(false);
      SoundEngine::instance()->removeSpeaker(obj);
      Engine::instance()->getFontRenderer()->removeText(obj, true);
      obj->setWalking(false);
      if (dir != UNSPECIFIED)
        obj->setLookDir(dir);
      if (roomname.empty())
        roomname = obj->getRoom();
      RoomObject* ro = Engine::instance()->getRoom(roomname);
      std::string realname;
      Vec2i scrolloffset;
      Vec2i roomoffset;
      float walkgridsize;
      if (ro == NULL){
        SaveStateProvider::SaveRoom* sro = Engine::instance()->getSaver()->getRoom(roomname);
        realname = sro->base.name;
        scrolloffset = sro->scrolloffset;
        roomoffset = sro->base.position;
        walkgridsize = sro->getWalkGridSize();
      }
      else{
        realname = ro->getName();
        scrolloffset = ro->getScrollOffset();
        roomoffset = ro->getPosition();
        walkgridsize = ro->getWalkGridSize();
      }
      obj->setRoom(realname);
      obj->setPosition((pos*walkgridsize)
        +Vec2f(walkgridsize/2, walkgridsize/2)
        +roomoffset);
      obj->setScrollOffset(scrolloffset);
      obj->setDepth((int)(obj->getPosition().y/Engine::instance()->getWalkGridSize(true)));
      //obj->setScale(ro->getDepthScale(obj->getPosition()));
    }
  }
  else{
    //resolve the self
    String realcharname = ctx.resolveCharName(charname);
    CharacterObject* obj = Engine::instance()->extractCharacter(realcharname);
    if (!obj){
      Engine::instance()->getResLoader()->beamCharacter(realcharname, &ctx, roomname, pos, dir);
    }
    else
      Engine::instance()->insertCharacter(obj, roomname, pos, dir);
  }
  return 0;
}

int ScriptFunctions::addItem(lua_State* L){
  NUM_ARGS(2, 3);
  std::string charname = ctx.stack().get(1).getString();
  std::string itemname = ctx.stack().get(2).getString();
  if (itemname == "givelink")
    itemname = ctx.getGiveObjectName();
  int inventory = 1;
  if (numArgs >= 3)
    inventory = ctx.stack().get(3).getInt();
  CharacterObject* chr = ctx.getCharacter(charname);
  ItemObject* item = Engine::instance()->createItem(itemname, 1);
  if (item == NULL)
    return 0;
  item->realize();
  if (chr){
    if (Engine::instance()->getSettings()->group_items){
      //increment only item count
      ItemObject* io = chr->getInventory()->getItem(itemname, inventory);
      if (io != NULL){
        io->setCount(io->getCount()+1);
        delete item;
        return 0;
      }
    }
    chr->getInventory()->addItem(item, inventory);
  }
  else{
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname);
    if (!cso){
      TR_BREAK("Character %s not found", charname.c_str());
    }
    if (Engine::instance()->getSettings()->group_items){
      for (unsigned i = 0; i < cso->inventory.items[inventory].size(); ++i){
        if (_stricmp(cso->inventory.items[inventory][i].name.c_str(), itemname.c_str()) == 0){
          ++cso->inventory.items[inventory][i].count;
          return 0;
        }
      }
    }
    SaveStateProvider::SaveItem si;
    si.name = item->getName();
    si.count = item->getCount();
    cso->inventory.items[inventory].push_back(si);
    delete item;
  }
  return 0;
}

int ScriptFunctions::cutScene(lua_State* L){
  NUM_ARGS(1, 2);
  std::string scriptname = ctx.stack().get(1).getString();
  bool hideUI = true;
  if (numArgs > 1){
    String donthide = ctx.stack().get(2).getString();
    if (donthide != "donthide")
      TR_BREAK("%s unexpected", donthide.c_str());
    hideUI = false;
  }
  Engine::instance()->getInterpreter()->cutsceneMode(hideUI);
  if (!ctx.getEvents().empty() && ctx.getEvents().front() == EVT_CLICK){ //the cutscene should stop the current click
    ctx.getEvents().pop_front(); //stop click
    ctx.getEvents().pop_front(); //stop user event
  }
  ExecutionContext* context = Engine::instance()->loadScript(Script::CUTSCENE, scriptname);
  Engine::instance()->getInterpreter()->executeCutscene(context, false);
  return 0;
}

int ScriptFunctions::taskbar(lua_State* L){
  NUM_ARGS(1, 1);
  bool state = ctx.stack().get(1).getBool();
  Engine::instance()->showTaskbar(state);
  return 0;
}

int ScriptFunctions::follow(lua_State* L){
  NUM_ARGS(2, 3);
  std::string char1 = ctx.stack().get(1).getString();
  std::string char2 = ctx.stack().get(2).getString();
  bool hold = true;
  if (numArgs >= 3){
    if (ctx.stack().get(3).getString() == "dontwait"){
      hold = false;
    }
  }
  CharacterObject* chr1 = ctx.getCharacter(char1);
  CharacterObject* chr2 = ctx.getCharacter(char2);
  if (chr1 && chr2){
    if (ctx.mSkip){
      chr1->setPosition(chr2->getPosition());
      //RoomObject* ro = Engine::instance()->getContainingRoom(chr1);
      //if (ro)
      //  chr1->setScale(ro->getDepthScale(chr2->getPosition()));

    }
    else{
      if (hold){
        chr1->setSuspensionScript(&ctx);
        ctx.mSuspended = true;
      }
      Engine::instance()->walkTo(chr1, chr2->getPosition(), UNSPECIFIED, 1.0f);
    }
  }
  return 0;
}

int ScriptFunctions::lookTo(lua_State* L){
  NUM_ARGS(2, 3);
  std::string character = ctx.stack().get(1).getString();
  StackData d = ctx.stack().get(2);
  if (numArgs >= 3){
    String noturn = ctx.stack().get(3).getString();
    if (noturn != "noturn")
      TR_BREAK("noturn expected");
  }
  LookDir dir = UNSPECIFIED;
  CharacterObject* chr1 = ctx.getCharacter(character);
  if (d.getInt() != 0){
    dir = (LookDir)(d.getInt()-1);
    if (chr1)
      chr1->setLookDir(dir);
    else{
      SaveStateProvider::CharSaveObject* chs = Engine::instance()->getSaver()->findCharacter(character);
      if (!chs){
        TR_USE(ADV_ScriptFunc);
        TR_ERROR("Character %s not found", character.c_str());
      }
      //chs->base.state = CharacterObject::calculateState(chs->base.state, false, false);
    }
  }
  else{
    std::string char2 = d.getString();
    CharacterObject* chr2 = ctx.getCharacter(char2);
    if (chr1 == NULL || chr2 == NULL) //look at another character not possible, they are in different rooms
      return 0;
    Vec2i dir = chr2->getPosition()-chr1->getPosition();
    chr1->setLookDir(dir);
  }
  return 0;
}

int ScriptFunctions::textScene(lua_State* L){
  NUM_ARGS(1, 4);
  std::string scenename = ctx.stack().get(1).getString();
  Vec2i pos(0,Engine::instance()->getResolution().y);
  int width = Engine::instance()->getResolution().x;
  if (numArgs > 1){
    pos.x = ctx.stack().get(2).getInt();
    if (numArgs > 2)
      pos.y = ctx.stack().get(3).getInt();
    if (numArgs > 3)
      width = ctx.stack().get(4).getInt();
    Engine::instance()->getInterpreter()->mTSTopToBottom = true;
  }
  else{
    Engine::instance()->getInterpreter()->mTSTopToBottom = false;
  }
  Engine::instance()->getInterpreter()->mTSName = scenename;
  Engine::instance()->getInterpreter()->mTSLevel = 1;
  Engine::instance()->getInterpreter()->mTSRow = 0;
  Engine::instance()->getInterpreter()->mTSPosOrig = pos;
  Engine::instance()->getInterpreter()->mTSWidth = width;
  Engine::instance()->getInterpreter()->mNextTSLevel = 0;
  Engine::instance()->enableTextScene(true);
  ExecutionContext* context = Engine::instance()->loadScript(Script::CUTSCENE, scenename);
  Engine::instance()->getInterpreter()->cutsceneMode(true);
  Engine::instance()->getInterpreter()->executeCutscene(context, true);
  return 0;
}

int ScriptFunctions::delItem(lua_State* L){
  NUM_ARGS(2, 3);
  std::string charname = ctx.stack().get(1).getString();
  std::string itemname = ctx.stack().get(2).getString();
  if (itemname == "givelink"){
    itemname = ctx.getGiveObjectName();
  }
  int inventory = 1;
  if (numArgs >= 3)
    inventory = ctx.stack().get(3).getInt();
  CharacterObject* chr = ctx.getCharacter(charname);
  if (chr){
    ItemObject* io = chr->getInventory()->getItem(itemname, inventory);
    if (io != NULL && io->getCount() > 1)
      io->setCount(io->getCount()-1);
    else
      chr->getInventory()->removeItem(itemname, inventory, &ctx);
  }
  else{
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname);
    if (!cso){
      TR_USE(ADV_ScriptFunc);
      TR_BREAK("Character %s not found", charname.c_str());
    }
    bool deleted = false;
    for (std::vector<SaveStateProvider::SaveItem>::iterator iter = cso->inventory.items[inventory].begin(); iter != cso->inventory.items[inventory].end(); ++iter){
      if (_stricmp(iter->name.c_str(), itemname.c_str()) == 0){
        if (iter->count > 1)
          --iter->count;
        else
          cso->inventory.items[inventory].erase(iter);
        deleted = true;
        break;
      }
    }
    if (!deleted){
      TR_USE(ADV_ScriptFunc);
      TR_BREAK("Unable to delete item %s from %s", itemname.c_str(), charname.c_str());
    }
  }
  return 0;
}

int ScriptFunctions::loopSound(lua_State* L){
  NUM_ARGS(1, 2);
  std::string sound = ctx.stack().get(1).getString();
  int volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().get(2).getInt();
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_DEFAULT);
  if (sp){
    sp->setVolume(volume/100.0f);
    sp->play(true);
  }
  return 0;
}

int ScriptFunctions::loopStop(lua_State* L){
  NUM_ARGS(1, 1);
  std::string sound = ctx.stack().get(1).getString();
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_DEFAULT);
  if (sp)
    sp->stop();
  return 0;
}

int ScriptFunctions::playMusic(lua_State* L){
  NUM_ARGS(1, 3);
  std::string music = ctx.stack().get(1).getString();
  std::string pattern;
  if (numArgs >= 2){
    pattern = ctx.stack().get(2).getString();
    if (!pattern.empty()){
      TR_USE(ADV_ScriptFunc);
      TR_BREAK("patterns in playMusic need to be implemented %s", pattern.c_str());
    }
  }
  if (numArgs >= 3){
    std::string dummy = ctx.stack().get(3).getString();
    TR_INFO("%s unexpected", dummy.c_str());
  }
  SoundPlayer* sp = SoundEngine::instance()->getMusic(music);
  if (sp && !sp->isPlaying()){
    sp->play(true);
  }
  return 0;
}

int ScriptFunctions::stopMusic(lua_State* L){
  NUM_ARGS(0, 0);
  SoundPlayer* sp = SoundEngine::instance()->getMusic("");
  if (sp){
    sp->fadeVolume(SoundEngine::instance()->getMusicVolume(), 0.0f, SoundEngine::instance()->getFadingTime());
  }
  return 0;
}

int ScriptFunctions::wait(lua_State* L){
  NUM_ARGS(1, 1);
  float seconds = ctx.stack().get(1).getFloat();
  if (ctx.mSkip)
    return 0;
  ctx.mSleepTime = (int)(seconds*1000);
  ctx.mSuspended = true;
  return 0;
}

int ScriptFunctions::subRoom(lua_State* L){
  NUM_ARGS(1, 2);
  std::string roomname = ctx.stack().get(1).getString();
  int fading_time = 0;
  if (numArgs >= 2){
    fading_time = ctx.stack().get(2).getInt();
  }
  if (Engine::instance()->isSubRoomLoaded())
    Engine::instance()->unloadRoom(NULL, false, false, &ctx);
  Engine::instance()->loadSubRoom(roomname, &ctx, fading_time);
  return 0;
}

int ScriptFunctions::subRoomReturn(lua_State* L){
  NUM_ARGS(0, 0);
  Engine::instance()->unloadRoom(NULL, false, false, &ctx);
  return 0;
}

int ScriptFunctions::subRoomReturnImmediate(lua_State* L){
  NUM_ARGS(0, 0);
  Engine::instance()->unloadRoom(NULL, false, true, &ctx);
  return 0;
}

int ScriptFunctions::link(lua_State* L){
  NUM_ARGS(1, 1);
  std::string objectname = ctx.stack().get(1).getString();
  Engine::instance()->setUseObject(objectname, ctx.mObjectInfo);
  PcdkScript::mRemoveLinkObject = false;
  return 0;
}

int ScriptFunctions::giveLink(lua_State* L){
  NUM_ARGS(0, 1);
  Object2D* obj = NULL;
  if (numArgs >= 1){
    std::string objectname = ctx.stack().get(1).getString();
    obj = Engine::instance()->getObject(objectname, true);
  }
  else{
    obj = ctx.mOwner;
  }
  Engine::instance()->setGiveObject(obj->getName(), ctx.mObjectInfo);
  PcdkScript::mRemoveLinkObject = false;
  return 0;
}

int ScriptFunctions::setNum(lua_State* L){
  NUM_ARGS(2, 2);
  String varname = ctx.stack().get(1).getString();
  float val = ctx.stack().get(2).getFloat();
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(val));
  return 0;
}

int ScriptFunctions::offSpeech(lua_State* L){
  NUM_ARGS(3, 5);
  Vec2i pos;
  float walkgridsize;
  RoomObject* room = Engine::instance()->getRoom("");
  if (room){
    walkgridsize = room->getWalkGridSize();
  }
  else
    walkgridsize = Engine::instance()->getWalkGridSize(false);
  pos.x = (int)(ctx.stack().get(1).getInt()*walkgridsize);
  pos.y = (int)(ctx.stack().get(2).getInt()*walkgridsize);
  std::string text = ctx.stack().get(3).getString();
  std::string sound = "";
  bool hold = Engine::instance()->getInterpreter()->isBlockingScriptRunning() || ctx.isLoop1() || ctx.isIdle();
  if (numArgs >= 4){
    sound = ctx.stack().get(4).getString();
    if (sound == "dontwait"){
      hold = false;
      sound = "";
    }
  }
  if (numArgs >= 5){
    std::string dw = ctx.stack().get(5).getString();
    if (dw == "dontwait")
      hold = false;
  }
  if (ctx.mSkip)
    return 0;
  FontRenderer::String* str = NULL;
  SoundPlayer* plyr = NULL;
  if (sound != ""){
    plyr = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_CREATE_ALWAYS);
    if (plyr){
      plyr->setVolume(SoundEngine::instance()->getSpeechVolume());
      plyr->play(false);
    }
  }
  //correct the offspeech position
  if (room){
    pos = pos+room->getScrollOffset();
  }
  std::vector<Vec2i> breakinfo;
  int fontid = Engine::instance()->getFontID();
  if (!Engine::instance()->isTextEnabled())
      text = "";
  Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, fontid, breakinfo);
  str = Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2,pos.y-ext.y, text, 
    DEPTH_GAME_FONT, fontid, breakinfo, Engine::instance()->getInterpreter()->getOfftextColor(), plyr ? 100000 : Engine::instance()->getInterpreter()->getTextSpeed()*(unsigned)text.length());
  if (str && plyr)
    plyr->setSpokenString(str);
  if (hold){
    if (plyr){
      plyr->setSuspensionScript(&ctx);
    }
    else if (str)
      str->setSuspensionScript(&ctx);
    ctx.mSuspended = true;
  }
  return 0;
}

int ScriptFunctions::unloadRoom(lua_State* L){
  NUM_ARGS(0, 0);
  bool animate = !ctx.isSkipping();
  Engine::instance()->unloadRoom(NULL, true, false, &ctx);
  //make unloads smoother. triggerScreenchange, add finished callback to screenchanges, call ForceNot... in callback
  if (animate){
    //Engine::instance()->getRoom("")->setFadeout(1);
    Engine::instance()->triggerScreenchange(NULL, Engine::instance()->getScreenChange(), true);
  }
  else
    Engine::instance()->forceNotToRenderUnloadingRoom();
  return 0;
}

int ScriptFunctions::restart(lua_State* L){
  NUM_ARGS(0, 0);
  Engine::instance()->reset();
  return 0;
}

int ScriptFunctions::gotoLevel(lua_State* L){
  NUM_ARGS(1, 1);
  int level = ctx.stack().get(1).getInt();
  Engine::instance()->getInterpreter()->mNextTSLevel = level;
  return 0;
}

int ScriptFunctions::deactivate(lua_State* L){
  NUM_ARGS(3, 3);
  std::string scene = ctx.stack().get(1).getString();
  int level = ctx.stack().get(2).getInt();
  int row = ctx.stack().get(3).getInt();
  if (scene == "self")
    scene = Engine::instance()->getInterpreter()->mTSName;
  Engine::instance()->getInterpreter()->mTSActive[scene][level][row] = false;
  return 0;
}

int ScriptFunctions::endScene(lua_State* L){
  NUM_ARGS(0, 0);
  ctx.mExecuteOnce = true;
  Engine::instance()->enableTextScene(false);
  Engine::instance()->clearGui();
  return 0;
}

int ScriptFunctions::instObj(lua_State* L){
  NUM_ARGS(2, ARG_MAX);
  std::string objname = ctx.stack().get(1).getString();
  int state = ctx.stack().get(2).getInt();
  for (unsigned i = 2; i < numArgs; ++i){
    TR_USE(ADV_ScriptFunc);
    TR_BREAK("state lists - implement me"); //TODO state lists
    state = ctx.stack().get(i+1).getInt();
  }
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj){
    Object2D* owner = obj;
    if (ctx.mOwner != NULL)
      owner = ctx.mOwner;
    Engine::instance()->getInterpreter()->setPrevState(owner, obj);
    obj->setState(state);
  }
  return 0;
}

int ScriptFunctions::command(lua_State* L){
  NUM_ARGS(0, 1);
  std::string cmd;
  if (numArgs == 1)
    cmd = ctx.stack().get(1).getString();
  else
    cmd = "none";
  //change also event in current script
  EngineEvent evt = ctx.getCommandEvent();
  ctx.resetEvent(evt);
  evt = Engine::instance()->getInterpreter()->getEngineEvent(cmd);
  ctx.setEvent(evt);
  Engine::instance()->setCommand(cmd, true);
  return 0;
}

int ScriptFunctions::invDown(lua_State* L){
  NUM_ARGS(1, 1);
  int move = ctx.stack().get(1).getInt();
  RoomObject* room = Engine::instance()->getContainingRoom(ctx.mOwner);
  int maxit = 1000;
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr)
    maxit = chr->getInventory()->getNumItemSlots();
  room->getInventory()->addScrollOffset(move, maxit);
  return 0;
}

int ScriptFunctions::invUp(lua_State* L){
  NUM_ARGS(1, 1);
  int move = ctx.stack().get(1).getInt();
  RoomObject* room = Engine::instance()->getContainingRoom(ctx.mOwner);
  int maxit = 1000;
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr)
    maxit = chr->getInventory()->getNumItemSlots();
  room->getInventory()->addScrollOffset(-move, maxit);
  return 0;
}

int ScriptFunctions::setFont(lua_State* L){
  NUM_ARGS(1, 2);
  int fontid = ctx.stack().get(1).getInt();
  if (!Engine::instance()->getFontRenderer()->loadFont(fontid))
    return 0;
  if (numArgs >= 2){
    CharacterObject* chr = ctx.getCharacter(ctx.stack().get(2).getString());
    if (!chr){
      TR_USE(ADV_ScriptFunc);
      TR_BREAK("Character not found");
    }
    chr->setFontID(fontid);
  }
  else{
    Engine::instance()->setFontID(fontid);
  }
  return 0;
}

ScreenChange ScriptFunctions::getScreenChange(const std::string& name){
  ScreenChange screenchange = SC_DIRECT;
  if (name == "rectangle")
    screenchange = SC_RECTANGLE;
  else if (name == "circle")
    screenchange = SC_CIRCLE;
  else if (name == "shutters")
    screenchange = SC_SHUTTERS;
  else if (name == "clock")
    screenchange = SC_CLOCK;
  else if (name == "blend")
    screenchange = SC_BLEND;
  else if (name == "blendslow")
    screenchange = SC_BLEND_SLOW;
  else if (name == "direct")
    screenchange = SC_DIRECT;
  else if (name == "fadeblack")
    screenchange = SC_FADEOUT;
  else{
    TR_USE(ADV_ScriptFunc);
    TR_BREAK("Unknown screenchange %s", name.c_str());
  }
  return screenchange;
}

int ScriptFunctions::setScreenchange(lua_State* L){
  NUM_ARGS(1, 1);
  StackData data = ctx.stack().get(1);
  ScreenChange screenchange = SC_DIRECT;
  if (data.getString().size() > 1){
    std::string name = data.getString();
    screenchange = getScreenChange(name);
  }
  else{
    screenchange = (ScreenChange)data.getInt();
  }
  Engine::instance()->setScreenChange(screenchange);
  return 0;
}

int ScriptFunctions::activate(lua_State* L){
  NUM_ARGS(3, 3);
  std::string scene = ctx.stack().get(1).getString();
  int level = ctx.stack().get(2).getInt();
  int row = ctx.stack().get(3).getInt();
  if (scene == "self")
    scene = Engine::instance()->getInterpreter()->mTSName;
  Engine::instance()->getInterpreter()->mTSActive[scene][level][row] = true;
  return 0;
}

int ScriptFunctions::saveGame(lua_State* L){
  NUM_ARGS(1, 1);
  int slot = ctx.stack().get(1).getInt();
  Engine::instance()->getSaver()->save(SaveStateProvider::saveSlotToPath(slot));
  return 0;
}

int ScriptFunctions::loadGame(lua_State* L){
  NUM_ARGS(1, 1);
  int slot = ctx.stack().get(1).getInt();
  Engine::instance()->getSaver()->load(SaveStateProvider::saveSlotToPath(slot));
  return 0;
}

int ScriptFunctions::jiggle(lua_State* L){
  NUM_ARGS(1, 2);
  float seconds = ctx.stack().get(1).getFloat();
  int power = 10;
  if (numArgs >= 2)
    power = ctx.stack().get(2).getInt();
  if (ctx.mSkip)
    return 0;
  RoomObject* room = Engine::instance()->getRoom("");
  if (room){
    std::list<Vec2i> path;
    path.push_back(Vec2i());
    int timecount = 0;
    while(timecount < seconds*1000){
      int x = (rand()%(2*power+1))-power;
      int y = (rand()%(2*power+1))-power;
      Vec2i next(x,y);
      float length = (next-path.back()).length();
      path.push_back(next);
      float time = length*20.0f/power;
      timecount += (int)time;
    }
    path.push_back(Vec2i());
    Engine::instance()->getAnimator()->add(room, path, (float)power);
  }
  return 0;
}

int ScriptFunctions::randomNum(lua_State* L){
  NUM_ARGS(2, 2);
  String name = ctx.stack().get(1).getString();
  int limit = ctx.stack().get(2).getInt();
  int rnd = rand()%limit;
  Engine::instance()->getInterpreter()->setVariable(name, StackData(rnd+1));
  return 0;
}

int ScriptFunctions::getRequestedState(Character* cclass, const StackData& data){
  TR_USE(ADV_ScriptFunc);
  int state = 0;
  if (data.isNumber()){
    state = data.getInt()+16;
    if (state > 36)
      state = 0;
  }
  else{
    std::string statename = data.getString();
    bool found = false;
    for (unsigned i = 0; i < cclass->extrastatenames.size(); ++i){
      if (_stricmp(cclass->extrastatenames[i].c_str(), statename.c_str()) == 0){
        found = true;
        state = i + 17;
        break;
      }
    }
    if (!found)
      TR_ERROR("setChar: state %s not found", statename.c_str());
  }
  return state;
}

int ScriptFunctions::setChar(lua_State* L){
  NUM_ARGS(2, ARG_MAX);
  std::string chrname = ctx.stack().get(1).getString();
  StackData data = ctx.stack().get(2);
  if (ctx.mSkip){
    for (unsigned i = 2; i < numArgs; ++i){
      ctx.stack().get(i+1);
    }
    return 0;
  }
  int state = 0;
  CharacterObject* obj = ctx.getCharacter(chrname);
  if (obj == NULL){
    TR_WARN("setChar: Character %s not found", chrname.c_str());
  }
  else{
    state = getRequestedState(obj->getClass(), data);
  }
  if (obj && state != 0){
    obj->clearNextStates();
    TR_DEBUG("setting new state %i for char %s", state, chrname.c_str());
    obj->setState(state);
    obj->getAnimation()->start(); //restart from beginning
  }
  for (unsigned i = 2; i < numArgs; ++i){
    data = ctx.stack().get(i+1);
    if (obj){
      state = getRequestedState(obj->getClass(), data);
      obj->addNextState(state);
      TR_DEBUG("adding intermediate state %i", state);
    }
  }
  if (obj){
    obj->addNextState(0);
    obj->getAnimation()->registerAnimationEndHandler(obj);
  }
  return 0;
}

int ScriptFunctions::setString(lua_State* L){
  NUM_ARGS(2, 2);
  String varname = ctx.stack().get(1).getString();
  String val = ctx.stack().get(2).getString();
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(val));
  return 0;
}

int ScriptFunctions::loadNum(lua_State* L){
  NUM_ARGS(1, 1);
  String varname = ctx.stack().get(1).getString();
  int val = 0;
  std::string file = Engine::instance()->getSettings()->savedir+"/num.sav";
  std::ifstream in(file.c_str());
  while(in){
    std::string name;
    int value;
    in >> name >> value;
    if (varname == name){
      val = value;
      break;
    }
  }
  in.close();
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(val));
  return 0;
}

int ScriptFunctions::saveNum(lua_State* L){
  NUM_ARGS(1, 1);
  String varname = ctx.stack().get(1).getString();
  int val = Engine::instance()->getInterpreter()->getVariable(varname).getInt();
  std::string file = Engine::instance()->getSettings()->savedir+"/num.sav";
  //load old content
  std::ifstream in(file.c_str());
  std::map<std::string, int> data;
  while (in){
    std::string name;
    int value;
    in >> name >> value;
    if (!in)
      break;
    data[name] = value;
  }
  in.close();
  if (data[varname] == val) //no change
    return 0;
  //insert new content
  data[varname] = val;
  //save content
  std::ofstream out(file.c_str());
  for (std::map<std::string,int>::iterator iter = data.begin(); iter != data.end(); ++iter){
    out << iter->first << " " << iter->second << std::endl;
  }
  out.close();
  Engine::instance()->getData()->fileChanged(file);
  return 0;
}

int ScriptFunctions::textEnabled(lua_State* L){
  NUM_ARGS(1, 1);
  bool enabled = ctx.stack().get(1).getBool();
  Engine::instance()->setTextEnabled(enabled);
  return 0;
}

int ScriptFunctions::realTime(lua_State* L){
  NUM_ARGS(1, 1);
  bool enabled = ctx.stack().get(1).getBool();
  //this is intentionally left blank, consider implementing if engine too slow otherwise
  return 0;
}

int ScriptFunctions::setCharLight(lua_State* L){
  NUM_ARGS(4, 5);
  std::string charname = ctx.stack().get(1).getString();
  Color c;
  c.r = (unsigned char)ctx.stack().get(2).getInt();
  c.g = (unsigned char)ctx.stack().get(3).getInt();
  c.b = (unsigned char)ctx.stack().get(4).getInt();
  bool fade = false;
  if (numArgs >= 5){
    std::string fading = ctx.stack().get(5).getString();
    if (fading == "fade")
      fade = true;
  }
  CharacterObject* chr = ctx.getCharacter(charname);
  if (chr){
    if (fade && !ctx.mSkip){
      //do not block right now.
      //be aware that if blocking is done, problems occur
      //example: Aluehn, Floor, opening lorengs door, walkto sets suspension script, walkmap overrides it with charlight, walk is not unblocked
      //multiple suspend reasons for one object needed?
      //chr->setSuspensionScript(&ctx);
      //ctx.mSuspended = true;
      Engine::instance()->getAnimator()->add(chr, c);
    }
    else{
      chr->setLightingColor(c);
    }
  }
  else{
    std::string room;
    std::string name;
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(ctx.resolveCharName(charname), room, name);
    cso->base.lighting = c;
  }
  return 0;
}

int ScriptFunctions::group(lua_State* L){
  NUM_ARGS(1, ARG_MAX);
  std::string groupname = ctx.stack().get(1).getString();
  ObjectGroup* grp = new ObjectGroup(groupname);
  for (unsigned i = 1; i < numArgs; ++i){
    std::string object = ctx.stack().get(i+1).getString();
    grp->add(object);
  }
  Engine::instance()->getInterpreter()->mGroups.push_back(grp);
  return 0;
}

int ScriptFunctions::stopSkip(lua_State* L){
  NUM_ARGS(0, 0);
  ctx.mSkip = false;
  return 0;
}

int ScriptFunctions::playSwf(lua_State* L){
  NUM_ARGS(1, 5);
  std::string moviename = ctx.stack().get(1).getString();
  int x = 0;
  int y = 0;
  int width = Engine::instance()->getSettings()->resolution.x;
  int height = Engine::instance()->getSettings()->resolution.y;
  if (numArgs >= 2){
    x = ctx.stack().get(2).getInt();
  }
  if (numArgs >= 3){
    y = ctx.stack().get(3).getInt();
  }
  if (numArgs >= 4){
    width = ctx.stack().get(4).getInt();
  }
  if (numArgs >= 5){
    height = ctx.stack().get(5).getInt();
  }
  VideoPlayer* vp = SoundEngine::instance()->getMovie(moviename, true);
  if (vp){
    vp->initLayer(x, y, width, height);
    vp->play(false);
  }
  return 0;
}

int ScriptFunctions::stopSwf(lua_State* L){
  NUM_ARGS(0, 0);
  VideoPlayer* vp = SoundEngine::instance()->getMovie("", true);
  if (vp){
    vp->stop();
  }
  return 0;
}

class VideoSuspender : Suspender{
public:
  VideoSuspender(VideoPlayer* player) : mPlayer(player){

  }
  virtual void forceResume(){
    mPlayer->stop();
  }
protected:
  VideoPlayer* mPlayer;
};

int ScriptFunctions::playVideo(lua_State* L){
  NUM_ARGS(2, 6);
  std::string moviename = ctx.stack().get(1).getString();
  bool suspend = ctx.stack().get(2).getBool();
  int x = 0;
  int y = 0;
  int width = Engine::instance()->getSettings()->resolution.x;
  int height = Engine::instance()->getSettings()->resolution.y;
  if (numArgs >= 3){
    x = ctx.stack().get(3).getInt();
  }
  if (numArgs >= 4){
    y = ctx.stack().get(4).getInt();
  }
  if (numArgs >= 5){
    width = ctx.stack().get(5).getInt();
  }
  if (numArgs >= 6){
    height = ctx.stack().get(6).getInt();
  }
  VideoPlayer* vp = SoundEngine::instance()->getMovie(moviename, false);
  if (!ctx.isSkipping() && vp){
    vp->initLayer(x, y, width, height);
    if (suspend){
      vp->setSuspensionScript(&ctx);
      ctx.suspend(0, NULL);
    }
    vp->play(false);
  }
  return 0;
}

int ScriptFunctions::setWalkmap(lua_State* L){
  NUM_ARGS(4, 4);
  std::string room = ctx.stack().get(1).getString();
  Vec2i pos;
  pos.x = ctx.stack().get(2).getInt();
  pos.y = ctx.stack().get(3).getInt();
  bool walkable = ctx.stack().get(4).getBool();
  RoomObject* rm = Engine::instance()->getRoom(room);
  if (rm)
    rm->modifyWalkmap(pos, walkable);
  else{
    SaveStateProvider::SaveRoom* srm = Engine::instance()->getSaver()->getRoom(room);
    srm->walkmap[pos] = walkable;
  }
  return 0;
}

int ScriptFunctions::stepTo(lua_State* L){
  NUM_ARGS(2, 2);
  std::string name = ctx.stack().get(1).getString();
  std::string dirname = ctx.stack().get(2).getString();
  LookDir dir = UNSPECIFIED;
  if (_stricmp(dirname.c_str(), "UP") == 0)
    dir = BACK;
  else if (_stricmp(dirname.c_str(), "DOWN") == 0)
    dir = FRONT;
  else if (_stricmp(dirname.c_str(), "LEFT") == 0)
    dir = LEFT;
  else if (_stricmp(dirname.c_str(), "RIGHT") == 0)
    dir = RIGHT;
  else{
    TR_BREAK("Unknown direction %s", dirname.c_str());
  }
  CharacterObject* chr = ctx.getCharacter(name);
  if (chr){
    int step = 3;
    Vec2i pos = chr->getPosition();
    switch(dir){
      case LEFT:
        pos.x -= step;
        break;
      case RIGHT:
        pos.x += step;
        break;
      case FRONT:
        pos.y += step;
        break;
      case BACK:
        pos.y -= step;
        break;
      default:
        break;
    }
    //Engine::instance()->walkTo(chr, pos, dir);
    chr->setPosition(pos);
    chr->setLookDir(dir);
    chr->setDepth((int)(pos.y/Engine::instance()->getWalkGridSize(true)));
  }
  else{
    TR_BREAK("Character %s not found", name.c_str());
  }
  return 0;
}

class PositionSuspender : public Suspender{
public:
  PositionSuspender(Object2D* obj, const Vec2i& pos) : mObject(obj), mPosition(pos) {
  }
  virtual void forceResume(){
    Engine::instance()->getAnimator()->remove(mObject);
    mObject->setPosition(mPosition);
  }
private:
  Object2D* mObject;
  Vec2i mPosition;
};

int ScriptFunctions::moveObj(lua_State* L){
  NUM_ARGS(4, 5);
  std::string name = ctx.stack().get(1).getString();
  //remove whitespaces in object names
  for(int size = (int)name.size()-1; size >= 0; --size){
    if (name[size] == ' ')
      name.erase(size, 1);
  }
  Vec2i newpos;
  newpos.x = ctx.stack().get(2).getInt();
  newpos.y = ctx.stack().get(3).getInt();
  int speed = ctx.stack().get(4).getInt();
  bool hold = false;
  if (numArgs >= 5){
    std::string wait = ctx.stack().get(5).getString();
    if (wait == "wait")
      hold = true;
  }

  ObjectGroup* grp = Engine::instance()->getInterpreter()->getGroup(name);
  std::vector<std::string> objects;
  if (grp)
    objects = grp->getObjects();
  else
    objects.push_back(name);

  Vec2i baseoffset;
  for (unsigned i = 0; i < objects.size(); ++i){
    std::string objname = objects[i];
    Object2D* obj = Engine::instance()->getObject(objname, false);
    if (obj == NULL){
      std::string room;
      SaveStateProvider::SaveObject* obj = Engine::instance()->getSaver()->findObject(objname, room);
      if (obj){
        if (i == 0)
          baseoffset = obj->position;
        obj->position = newpos+obj->position-baseoffset;
      }
      else
        TR_ERROR("moveObj: Object %s does not exist", objname.c_str());
      continue;
    }
    if (i == 0)
      baseoffset = obj->getPosition();
    if (speed == 0 || ctx.mSkip){
      TR_DETAIL("object %s positioned to %i %i", objname.c_str(), newpos.x, newpos.y);
      Engine::instance()->getAnimator()->remove(obj);
      obj->setPosition(newpos+obj->getPosition()-baseoffset);
      continue;
    }
    std::list<Vec2i> path;
    //path.push_back(obj->getPosition());
    path.push_back(newpos+obj->getPosition()-baseoffset);
    float factor;
    if (speed <= 10)
      factor = (float)(11-speed);
    else{
      //we have ms => calculate speedfactor
      factor = (newpos+obj->getPosition()-baseoffset-obj->getPosition()).length()*20.0f/speed;
    }
    if (hold){
      ctx.suspend(0, NULL/*new PositionSuspender(obj, newpos)*/);
      obj->setSuspensionScript(&ctx);
    }
    Engine::instance()->getAnimator()->add(obj, path, factor);
  }
  return 0;
}

int ScriptFunctions::quit(lua_State* L){
  NUM_ARGS(0, 0);
  Engine::instance()->quit();
  return 0;
}

int ScriptFunctions::musicVolume(lua_State* L){
  NUM_ARGS(1, 1);
  int volume = ctx.stack().get(1).getInt();
  SoundEngine::instance()->setMusicVolume(volume/100.0f);
  return 0;
}

int ScriptFunctions::setParticles(lua_State* L){
  NUM_ARGS(6, 6);
  std::string object = ctx.stack().get(1).getString();
  int speed = ctx.stack().get(2).getInt();
  int amount = ctx.stack().get(3).getInt();
  int direction = ctx.stack().get(4).getInt();
  int rotation = ctx.stack().get(5).getInt();
  int variation = ctx.stack().get(6).getInt();
  float angle = (float)((90.0f-direction)/180.0f*M_PI);
  Vec2f dir;
  dir.x = speed*cosf(angle);
  dir.y = speed*sinf(angle);
  Engine::instance()->getParticleEngine()->setDirection(dir);
  Engine::instance()->getParticleEngine()->setMaxParticles(amount);
  float anglespeed = (float)(rotation/180.0f*M_PI);
  Engine::instance()->getParticleEngine()->setRotation(anglespeed);
  Engine::instance()->getParticleEngine()->setSpeedVariation(variation/100.0f);
  Engine::instance()->getParticleEngine()->setParticleObject(object, float(angle-M_PI/2.0f));
  return 0;
}

int ScriptFunctions::startParticles(lua_State* L){
  NUM_ARGS(0, 1);
  bool fast = false;
  if (numArgs >= 1){
    std::string arg = ctx.stack().get(1).getString();
    fast = arg == "fast";
  }
  Engine::instance()->getParticleEngine()->activate(true, fast);
  return 0;
}

int ScriptFunctions::stopParticles(lua_State* L){
  NUM_ARGS(0, 1);
  bool fast = false;
  if (numArgs >= 1){
    std::string arg = ctx.stack().get(1).getString();
    fast = arg == "fast";
  }
  Engine::instance()->getParticleEngine()->activate(false, fast);
  return 0;
}

int ScriptFunctions::function(lua_State* L){
  NUM_ARGS(1, 2);
  std::string scriptname = ctx.stack().get(1).getString();
  TR_DEBUG("Function %s started", scriptname.c_str());
  ExecutionContext* func = Engine::instance()->getInterpreter()->getScript(scriptname);
  int numExecutions = 1;
  if (numArgs > 1){
    StackData d = ctx.stack().get(2);
    if (d.getInt() == 0){
      std::string txt = d.getString();
      bool loop = false;
      CSTATE* state = (CSTATE*)func->getCode()->get(0);
      if (txt == "inf" || txt == "infinitly" || txt == "*"){
        loop = true;
        state->setState(CSTATE::NORMAL);
      }
      else if (txt == "loop2"){
        loop = true;
        state->setState(CSTATE::IDLE);
      }
      else
        TR_BREAK("Unhandled repeat %s", txt.c_str());
      Engine::instance()->getInterpreter()->execute(func, !loop);
    }
    else{
      numExecutions = atoi(d.getString().c_str());
      TR_BREAK("Check this");
    }
  }
  else
    if (!Engine::instance()->getInterpreter()->executeImmediately(func)){
      //makes it inaccessible for stopfunction, but prevents function to find the already running/destroyed function
      Engine::instance()->getInterpreter()->removeScript(scriptname, true);
      //blocking stuff prevents immediate execution, so enqueue the script
      Engine::instance()->getInterpreter()->execute(func, true);
    }
    else{
      Engine::instance()->getInterpreter()->removeScript(scriptname, false); //script ran through, so remove it again
    }
  return 0;
}

int ScriptFunctions::stopFunction(lua_State* L){
  NUM_ARGS(1, 2);
  std::string scriptname = ctx.stack().get(1).getString();
  if (numArgs >= 2){
    String dummy = ctx.stack().get(2).getString();
    if (dummy != "inf")
      TR_BREAK("inf expected");
  }
  TR_DEBUG("Function %s stopped", scriptname.c_str());
  ExecutionContext* stopped = Engine::instance()->getInterpreter()->removeScript(scriptname, false);
  if (stopped == &ctx){
    //script removes itself, skip remaining instructions
    ctx.mPC = 1000000;
  }
  return 0;
}

int ScriptFunctions::speechVolume(lua_State* L){
  NUM_ARGS(1, 1);
  int volume = ctx.stack().get(1).getInt();
  SoundEngine::instance()->setSpeechVolume(volume/100.0f);
  return 0;
}

int ScriptFunctions::setLanguage(lua_State* L){
  NUM_ARGS(1, 1);
  std::string language = ctx.stack().get(1).getString();
  Engine::instance()->getInterpreter()->setLanguage(language);
  return 0;
}

int ScriptFunctions::enterText(lua_State* L){
  NUM_ARGS(0, 9);
  int textnum = -1;
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setEnabled(true);
  String varname;
  int maxchars = 100;
  String initalContent;
  if (numArgs >= 1){
    ExecutionContext* text = ctx.stack().get(1).getEC();
    //get and init the variable
    CCode* code = text->getCode()->get(text->getCode()->numInstructions()-2);
    if (code == NULL) //no string prepended to variable
      code = text->getCode()->get(text->getCode()->numInstructions()-1);
    if (code->getType() != CCode::LOAD){
      TR_USE(ADV_ScriptFunc);
      TR_BREAK("Something's wrong");
    }
    else{
      CLOAD* load = (CLOAD*)code;
      varname = load->getVarname();
    }

    Engine::instance()->getInterpreter()->executeImmediately(text, false);
    StackData result = text->stack().pop();
    if (!result.isNumber() || result.getInt() != -1){
      txtout->setText(text);
    }
  }
  if (numArgs >= 2){
    int x = ctx.stack().get(2).getInt();
    if (x != -1)
      txtout->getPos().x = x;
  }
  if (numArgs >= 3){
    int y = ctx.stack().get(3).getInt();
    if (y != -1)
      txtout->getPos().y = y;
  }
  if (numArgs >= 4){
    int font = ctx.stack().get(4).getInt();
    if (font != -1)
      txtout->setFont(font);
  }
  if (numArgs >= 5){
    maxchars = ctx.stack().get(5).getInt();
  }
  if (numArgs >= 6){
    int col = ctx.stack().get(6).getInt();
    if (col != -1)
      txtout->getColor().r = col;
  }
  if (numArgs >= 7){
    int col = ctx.stack().get(7).getInt();
    if (col != -1)
      txtout->getColor().g = col;
  }
  if (numArgs >= 8){
    int col = ctx.stack().get(8).getInt();
    if (col != -1)
      txtout->getColor().b = col;
  }
  if (numArgs >= 9){
    initalContent = ctx.stack().get(9).getString();
  }
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(initalContent));
  Engine::instance()->getInterpreter()->cutsceneMode(false);
  Engine::instance()->enterText(varname, maxchars, &ctx);
  ctx.mSuspended = true;
  return 0;
}

int ScriptFunctions::fadeSpeed(lua_State* L){
  NUM_ARGS(1, 1);
  int speed = ctx.stack().get(1).getInt();
  if (speed <= 15){
    speed = 1500-(speed-1)*100;
  }
  SoundEngine::instance()->setFadingTime(speed);
  return 0;
}

int ScriptFunctions::setEAX(lua_State* L){
  NUM_ARGS(1, 1);
  std::string effect = ctx.stack().get(1).getString();
  SoundEngine::instance()->setEAXEffect(toLower(effect));
  return 0;
}

int ScriptFunctions::bindText(lua_State* L){
  NUM_ARGS(2, 2);
  int textnum = ctx.stack().get(1).getInt();
  String room = ctx.stack().get(2).getString();
  if (room == "any"){
    room = Engine::instance()->getData()->getProjectSettings()->anywhere_room.c_str();
  }
  else if (room == "taskbar"){
    room = Engine::instance()->getData()->getProjectSettings()->taskroom.c_str();
  }
  else if (room == "menu"){
    if (Engine::instance()->getData()->getProjectSettings()->has_menuroom)
      room = Engine::instance()->getData()->getProjectSettings()->menuroom.c_str();
    else
      TR_BREAK("Implement me");
  }
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setRoom(room);
  return 0;
}

int ScriptFunctions::textOut(lua_State* L){
  NUM_ARGS(1, 8);
  int textnum = ctx.stack().get(1).getInt();
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setEnabled(true);
  if (numArgs >= 2){
    ExecutionContext* text = ctx.stack().get(2).getEC();
    Engine::instance()->getInterpreter()->executeImmediately(text, false);
    StackData result = text->stack().pop();
    if (!result.isNumber() || result.getInt() != -1){
      txtout->setText(text);
    }
  }
  if (numArgs >= 3){
    int x = ctx.stack().get(3).getInt();
    if (x != -1){
      Engine::instance()->getAnimator()->remove(txtout);
      txtout->getPos().x = x;
    }
  }
  if (numArgs >= 4){
    int y = ctx.stack().get(4).getInt();
    if (y != -1){
      Engine::instance()->getAnimator()->remove(txtout);
      txtout->getPos().y = y;
    }
  }
  if (numArgs >= 5){
    int font = ctx.stack().get(5).getInt();
    if (font != -1)
      txtout->setFont(font);
  }
  if (numArgs >= 6){
    int col = ctx.stack().get(6).getInt();
    if (col != -1)
      txtout->getColor().r = col;
  }
  if (numArgs >= 7){
    int col = ctx.stack().get(7).getInt();
    if (col != -1)
      txtout->getColor().g = col;
  }
  if (numArgs >= 8){
    int col = ctx.stack().get(8).getInt();
    if (col != -1)
      txtout->getColor().b = col;
  }
  return 0;
}

int ScriptFunctions::textSpeed(lua_State* L){
  NUM_ARGS(1, 1);
  std::string speed = ctx.stack().get(1).getString();
  toLower(speed);
  int numspeed = 100;
  if (speed == "slow")
    numspeed = 110;
  else if (speed == "normal")
    numspeed = 100;
  else if (speed == "fast")
    numspeed = 90;
  else
    TR_BREAK("Unhandled speed %s", speed.c_str());
  Engine::instance()->getInterpreter()->setTextSpeed(numspeed);
  return 0;
}

int ScriptFunctions::setPos(lua_State* L){
  NUM_ARGS(3, 5);
  std::string roomname = ctx.stack().get(1).getString();
  Vec2i pos;
  pos.x = ctx.stack().get(2).getInt();
  pos.y = ctx.stack().get(3).getInt();
  pos = pos * -Engine::instance()->getWalkGridSize(false);
  bool dontscroll = false;
  if (numArgs > 3)
    dontscroll = ctx.stack().get(4).getBool();
  if (numArgs > 4){
    LookDir blenddir = UNSPECIFIED;
    string dir = ctx.stack().get(5).getString();
    if (dir == "right")
      blenddir = RIGHT;
    else if (dir == "left")
      blenddir = LEFT;
    else if (dir == "up")
      blenddir = BACK;
    else if (dir == "down")
      blenddir = FRONT;
    else
      TR_BREAK("Unknown direction %s", dir.c_str());
    Engine::instance()->getAnimator()->moveCameraViewport(blenddir);
  }
  RoomObject* room = Engine::instance()->getRoom(roomname);
  if (room){
    if (dontscroll || ctx.isSkipping())
      room->setScrollOffset(pos);
    else
      Engine::instance()->getAnimator()->add(room, pos, Engine::instance()->getScrollSpeed());
  }
  else{
    SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(roomname);
    if (sr == NULL){
      TR_ERROR("setPos called on room %s that does not exist", roomname.c_str());
      return 0;
    }
    sr->scrolloffset = pos;
  }
  return 0;
}

int ScriptFunctions::miniCut(lua_State* L){
  NUM_ARGS(0, 1);
  bool hide = true;
  if (numArgs >= 1){
    String hstr = ctx.stack().get(1).getString();
    if (hstr != "donthide")
      TR_WARN("donthide expected");
    hide = false;
  }
  Engine::instance()->getInterpreter()->cutsceneMode(hide);
  return 0;
}

int ScriptFunctions::miniCutEnd(lua_State* L){
  NUM_ARGS(0, 0);
  if (Engine::instance()->getInterpreter()->getCutscene() == NULL){ //no cutscene started from minicut
    Engine::instance()->getInterpreter()->mGlobalSuspend = false;
    Engine::instance()->getInterpreter()->mHideUI = false;
  }
  return 0;
}

int ScriptFunctions::breakExec(lua_State* L){
  NUM_ARGS(0, 0);
  //don't know anymore why this was there, but it is bad => see elevator (room 8) functiondemo
  //ctx.resetEvents(true);
  //it was there because of this: pickup keycard and use machine (simple test adventure)
  return 0;
}

int ScriptFunctions::particleView(lua_State* L){
  NUM_ARGS(1, 1);
  StackData view = ctx.stack().get(1);
  int viewnum = 0;
  if (view.isString()){
    std::string val = view.getString();
    if (val == "behindfront")
      viewnum = 2;
    else if (val == "back")
      viewnum = 4;
    else
      TR_BREAK("Unhandled view %s", val.c_str());
  }
  else
    viewnum = view.getInt();
  int depth = 0;
  switch(viewnum){
    case 1:
      depth = DEPTH_PARTICLES_TOP;
      break;
    case 2:
      depth = DEPTH_PARTICLES_FRONT;
      break;
    case 3:
      depth = DEPTH_PARTICLES_MIDDLE;
      break;
    case 4:
      depth = DEPTH_PARTICLES_BACK;
      break;
    default:
      TR_BREAK("Unhandled depth %i", depth);
      break;
  }
  Engine::instance()->getParticleEngine()->setDepth(depth);
  return 0;
}

int ScriptFunctions::textHide(lua_State* L){
  NUM_ARGS(1, 1);
  int textnum = ctx.stack().get(1).getInt();
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setEnabled(false);
  return 0;
}

class TimeAnimation : public DynamicAnimation{
public:
  TimeAnimation(const float src, const float target) : mDuration(2000), 
    mCurrentTime(0), mSrc(src), mTarget(target){
      mDuration = (int)(abs(mSrc-mTarget)*mDuration);
  }
  virtual bool update(unsigned interval){
    float t = (mDuration-mCurrentTime)/(float)mDuration;
    if (t < 0)
      t = 0;
    if (t > 1.0f)
      t = 1.0f;
    float factor = t*mSrc + (1-t)*mTarget;
    Engine::instance()->setTimeFactor(factor, true);
    SoundEngine::instance()->setSpeedFactor(factor);
    if (mCurrentTime >= mDuration){
      return false;
    }
    mCurrentTime += interval;
    return true;
  }
  virtual void finish(){
    Engine::instance()->setTimeFactor(mTarget, true);
    SoundEngine::instance()->setSpeedFactor(mTarget);
  }
  virtual Object2D* getTarget() {return NULL;}
  virtual Type getType() {return TIME;}
private:
  int mDuration;
  int mCurrentTime;
  float mSrc;
  float mTarget;
};

int ScriptFunctions::stopEffect(lua_State* L){
  NUM_ARGS(1, 2);
  std::string effect = ctx.stack().get(1).getString();
  if (numArgs == 2 && effect != "lightning")
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  if (effect == "all"){
    Engine::instance()->getPostProcessor()->stopEffects();
  }
  if (effect == "slowmotion" || effect == "all"){
    bool isFaded;
    float currFactor = Engine::instance()->getTimeFactor(&isFaded);
    if (isFaded){
      if (currFactor != 1.0f){
        TimeAnimation* ta = new TimeAnimation(currFactor, 1.0f);
        Engine::instance()->getAnimator()->add(ta);
      }
    }
    else{
      Engine::instance()->setTimeFactor(1.0f, false);
      SoundEngine::instance()->setSpeedFactor(1.0f);
    }
  }
  else if (effect == "lightning" && numArgs == 2){
    int idx = ctx.stack().get(2).getInt();
    PostProcessor::Effect* ef = Engine::instance()->getPostProcessor()->getEffect(effect);
    if (ef)
      ef->deactivate(idx);
  }
  else{
    PostProcessor::Effect* ef = Engine::instance()->getPostProcessor()->getEffect(effect);
    if (ef)
      ef->deactivate();
  }
  return 0;
}

static void disableMainEffect(){
  Engine::instance()->getPostProcessor()->getEffect("darkbloom")->deactivate();
  Engine::instance()->getPostProcessor()->getEffect("hell")->deactivate();
  Engine::instance()->getPostProcessor()->getEffect("motionblur")->deactivate();
  Engine::instance()->getPostProcessor()->getEffect("heat")->deactivate();
  Engine::instance()->getPostProcessor()->getEffect("whoosh")->deactivate();
  Engine::instance()->getPostProcessor()->getEffect("bloom")->deactivate();
  Engine::instance()->getPostProcessor()->getEffect("underwater")->deactivate();
}

int ScriptFunctions::startEffect(lua_State* L){
  NUM_ARGS(1, ARG_MAX);
  std::string effect = ctx.stack().get(1).getString();
  if (effect == "slowmotion"){
    int speed = ctx.stack().get(2).getInt();
    bool fade = false;
    if (numArgs > 2){
      std::string fadestr = ctx.stack().get(3).getString();
      if (fadestr == "fade")
        fade = true;
    }
    float factor = speed/100.0f;
    if (fade){
      float currFactor = Engine::instance()->getTimeFactor();
      if (factor != currFactor){
        TimeAnimation* ta = new TimeAnimation(currFactor, factor);
        Engine::instance()->getAnimator()->add(ta);
      }
    }
    else{
      Engine::instance()->setTimeFactor(factor, false);
      SoundEngine::instance()->setSpeedFactor(factor);
    }
    return 0;
  }
  PostProcessor::Effect* ef = Engine::instance()->getPostProcessor()->getEffect(effect);
  if (effect == "noise" || effect == "darkbloom" || effect == "whoosh" || effect == "bloom" || effect == "fog"){
    int strength = ctx.stack().get(2).getInt();
    bool fade = false;
    if (numArgs > 2){
      std::string fadestr = ctx.stack().get(3).getString();
      if (fadestr == "fade")
        fade = true;
    }
    if (effect == "darkbloom" || effect == "whoosh" || effect == "bloom"){
      disableMainEffect();
      ef->activate(fade, strength/50.0f, fade);
    }
    else if (effect == "noise" || effect == "fog")
      ef->activate(fade, strength/99.0f);
  }
  else if (effect == "hell"){
    disableMainEffect();
    ef->activate(false, 1.0, false);
  }
  else if (effect == "motionblur"){
    int strength = ctx.stack().get(2).getInt();
    disableMainEffect();
    ef->activate(false, strength/50.0f);
  }
  else if (effect == "heat"){
    bool fade = false;
    if (numArgs > 1){
      std::string fadestr = ctx.stack().get(2).getString();
      if (fadestr == "fade")
        fade = true;
    }
    disableMainEffect();
    ef->activate(fade);
  }
  else if (effect == "underwater"){
    disableMainEffect();
    ef->activate(false);
  }
  else if (effect == "drugged"){
    int strength = ctx.stack().get(2).getInt();
    ef->activate(false, strength/500.0f);
  }
  else if (effect == "lightning"){
    int slot = ctx.stack().get(2).getInt();
    double x1 = (double)ctx.stack().get(3).getInt();
    double y1 = (double)ctx.stack().get(4).getInt();
    double x2 = (double)ctx.stack().get(5).getInt();
    double y2 = (double)ctx.stack().get(6).getInt();
    x1 /= (double)Engine::instance()->getSettings()->resolution.x;
    y1 /= (double)Engine::instance()->getSettings()->resolution.y;
    x2 /= (double)Engine::instance()->getSettings()->resolution.x;
    y2 /= (double)Engine::instance()->getSettings()->resolution.y;
    Color c;
    c.r = (unsigned char)ctx.stack().get(7).getInt();
    c.g = (unsigned char)ctx.stack().get(8).getInt();
    c.b = (unsigned char)ctx.stack().get(9).getInt();
    int spikes = ctx.stack().get(10).getInt();
    double heightfactor = Engine::instance()->getSettings()->resolution.x+Engine::instance()->getSettings()->resolution.y/2.0;
    double height = ctx.stack().get(11).getInt()/heightfactor;
    int speed = ctx.stack().get(12).getInt();
    speed = (speed+1)*25;
    ef->activate(false, slot, x1, y1, x2, y2, c.r, c.g, c.b, spikes, height, speed);
  }
  else if (effect == "zoom"){
    Vec2i pos;
    pos.x = ctx.stack().get(2).getInt();
    pos.y = ctx.stack().get(3).getInt();
    double factor = ctx.stack().get(4).getFloat()/100.0f;
    int fading = 0;
    if (numArgs > 4){
      std::string fadestr = ctx.stack().get(5).getString();
      if (fadestr == "fade")
        fading = 1000;
      else if (fadestr == "fadeslow")
        fading = 2000;
      else
        TR_BREAK("Unknown fading %s", fadestr.c_str());
    }
    ef->activate(fading != 0, pos.x, pos.y, factor, fading);
  }
  else if (effect == "whiteflash"){
    int fadein = 0;
    if (numArgs > 1)
      fadein = ctx.stack().get(2).getInt();
    int fadeout = 0;
    if (numArgs > 2)
      ctx.stack().get(3).getInt();
    ef->activate(true, fadein, fadeout);
  }
  else{
    TR_USE(ADV_ScriptFunc);
    TR_BREAK("Unknown effect %s", effect.c_str());
  }
  return 0;
}

int ScriptFunctions::linkChar(lua_State* L){
  NUM_ARGS(2, 2);
  std::string character = ctx.stack().get(1).getString();
  std::string object = ctx.stack().get(2).getString();
  CharacterObject* chr = ctx.getCharacter(character);
  if (!chr){
    //just save link information for later use
    std::string room;
    std::string name;
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(character, room, name);
    cso->linkedObject = object;
    return 0;
  }
  Object2D* obj = Engine::instance()->getObject(object, false);
  if (!obj)
    TR_BREAK("Unknown object %s", object.c_str());
  chr->setLinkObject(obj);
  return 0;
}

int ScriptFunctions::stopZooming(lua_State* L){
  NUM_ARGS(2, 2);
  std::string character = ctx.stack().get(1).getString();
  bool stopzooming = ctx.stack().get(2).getBool();
  CharacterObject* chr = ctx.getCharacter(character);
  if (!chr){
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(character);
    if (cso == NULL)
      TR_BREAK("Unknown character %s", character.c_str());
    cso->nozooming = stopzooming;
  }
  else
    chr->setNoZooming(stopzooming, stopzooming && ctx.isSkipping());
  return 0;
}

int ScriptFunctions::unlinkChar(lua_State* L){
  NUM_ARGS(1, 2);
  std::string character = ctx.stack().get(1).getString();
  String object = "";
  if (numArgs >= 2)
    object = ctx.stack().get(2).getString();
  CharacterObject* chr = ctx.getCharacter(character);
  if (!chr)
    TR_BREAK("Unknown character %s", character.c_str());
  chr->setLinkObject(NULL);
  return 0;
}

int ScriptFunctions::saveString(lua_State* L){
  NUM_ARGS(1, 1);
  String varname = ctx.stack().get(1).getString();
  StackData val = Engine::instance()->getInterpreter()->getVariable(varname);
  std::string file = Engine::instance()->getSettings()->savedir+"/string.sav";
  //load old content
  std::ifstream in(file.c_str());
  std::map<std::string, StackData> data;
  while (in){
    std::string name;
    StackData value;
    in >> name >> value;
    if (!in)
      break;
    data[name] = value;
  }
  in.close();
  if (data[varname].getString() == val.getString()) //no change
    return 0;
  //insert new content
  data[varname] = val;
  //save content
  std::ofstream out(file.c_str());
  for (std::map<std::string,StackData>::iterator iter = data.begin(); iter != data.end(); ++iter){
    out << iter->first << " " << iter->second << std::endl;
  }
  out.close();
  Engine::instance()->getData()->fileChanged(file);
  return 0;
}

int ScriptFunctions::loadString(lua_State* L){
  NUM_ARGS(1, 1);
  String varname = ctx.stack().get(1).getString();
  StackData val(String("none"));
  std::string file = Engine::instance()->getSettings()->savedir+"/string.sav";
  std::ifstream in(file.c_str());
  while(in){
    std::string name;
    StackData value;
    in >> name >> value;
    if (varname == name){
      val = value;
      break;
    }
  }
  in.close();
  Engine::instance()->getInterpreter()->setVariable(varname, val);
  return 0;
}

int ScriptFunctions::showMouse(lua_State* L){
  NUM_ARGS(1, 1);
  bool show = ctx.stack().get(1).getBool();
  Engine::instance()->showMouse(show);
  return 0;
}

int ScriptFunctions::charZoom(lua_State* L){
  NUM_ARGS(2, 3);
  std::string charname = ctx.stack().get(1).getString();
  int size = ctx.stack().get(2).getInt();
  bool fade = false;
  if (numArgs >= 3){
    std::string fadestr = ctx.stack().get(3).getString();
    if (fadestr != "fade")
      TR_BREAK("fade is %s", fadestr.c_str());
    fade = true;
  }
  CharacterObject* chr = ctx.getCharacter(charname);
  if (chr){
    if (fade && !ctx.isSkipping())
      Engine::instance()->getAnimator()->add(chr, chr->getUserScale(), size/100.0f, true);
    else
      chr->setUserScale(size/100.0f);
  }
  else{
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname);
    cso->scale = size/100.0f;
  }
  return 0;
}

int ScriptFunctions::setWalkSound(lua_State* L){
  NUM_ARGS(2, 2);
  std::string charname = ctx.stack().get(1).getString();
  std::string soundname = ctx.stack().get(2).getString();
  CharacterObject* chr = ctx.getCharacter(charname);
  if (chr){
    SoundPlayer* plyr = SoundEngine::instance()->getSound(soundname, SoundEngine::PLAYER_CREATE_ALWAYS | SoundEngine::PLAYER_UNMANAGED);
    chr->setWalkSound(plyr);
  }
  else{
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(ctx.resolveCharName(charname));
    if (cso == NULL)
      TR_BREAK("Character %s not found", charname.c_str());
    else{
      cso->walksound = soundname;
      CharacterObject* cached = Engine::instance()->getCachedCharacter(ctx.resolveCharName(charname));
      if (cached){
        SoundPlayer* plyr = SoundEngine::instance()->getSound(soundname, SoundEngine::PLAYER_CREATE_ALWAYS | SoundEngine::PLAYER_UNMANAGED);
        cached->setWalkSound(plyr);
      }
    }
  }
  return 0;
}

int ScriptFunctions::hideAllTexts(lua_State* L){
  NUM_ARGS(0, 0);
  Engine::instance()->getFontRenderer()->enableTextouts(false);
  return 0;
}

int ScriptFunctions::enableMouse(lua_State* L){
  NUM_ARGS(1, 1);
  bool enable = ctx.stack().get(1).getBool();
  Engine::instance()->enableMouse(enable);
  return 0;
}

int ScriptFunctions::setRectWalkmap(lua_State* L){
  NUM_ARGS(6, 6);
  std::string room = ctx.stack().get(1).getString();
  Vec2i pos1;
  Vec2i pos2;
  pos1.x = ctx.stack().get(2).getInt();
  pos1.y = ctx.stack().get(3).getInt();
  pos2.x = ctx.stack().get(4).getInt();
  pos2.y = ctx.stack().get(5).getInt();
  bool walkable = ctx.stack().get(6).getBool();
  Vec2i pos;
  RoomObject* rm = Engine::instance()->getRoom(room);
  if (rm){
    for (int j = pos1.y; j <= pos2.y; ++j){
      pos.y = j;
      for (int i = pos1.x; i <= pos2.x; ++i){
        pos.x = i;
        rm->modifyWalkmap(pos, walkable);
      }
    }
  }
  else{
    SaveStateProvider::SaveRoom* srm = Engine::instance()->getSaver()->getRoom(room);
    for (int j = pos1.y; j <= pos2.y; ++j){
      pos.y = j;
      for (int i = pos1.x; i <= pos2.x; ++i){
        pos.x = i;
        srm->walkmap[pos] = walkable;
      }
    }
  }
  return 0;
}

int ScriptFunctions::exchange(lua_State* L){
  NUM_ARGS(2, 2);
  std::string char1 = ctx.stack().get(1).getString();
  std::string char2 = ctx.stack().get(2).getString();
  CharacterObject* c1 = ctx.getCharacter(char1);
  CharacterObject* c2 = ctx.getCharacter(char2);
  if (c1 == NULL || c2 == NULL){
    std::string room, realname;
    if (c1 == NULL && c2 != NULL){
      SaveStateProvider::SaveInventory si;
      c2->getInventory()->save(si);
      SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(char1, room, realname);
      if (cso == NULL)
        TR_BREAK("Character %s is unknown", char1.c_str());
      else{
        c2->getInventory()->load(cso->inventory);
        c2->getInventory()->realize();
        cso->inventory = si;
      }
    }
    else if (c2 == NULL && c1 != NULL){
      SaveStateProvider::SaveInventory si;
      c1->getInventory()->save(si);
      SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(char2, room, realname);
      if (cso == NULL)
        TR_BREAK("Character %s is unknown", char2.c_str());
      else{
        c1->getInventory()->load(cso->inventory);
        c1->getInventory()->realize();
        cso->inventory = si;
      }
    }
    else{
      SaveStateProvider::CharSaveObject* cso1 = Engine::instance()->getSaver()->findCharacter(char1, room, realname);
      SaveStateProvider::CharSaveObject* cso2 = Engine::instance()->getSaver()->findCharacter(char2, room, realname);
      if (cso1 == NULL || cso2 == NULL)
        TR_BREAK("Character %s or %s is unknown", char1.c_str(), char2.c_str());
      SaveStateProvider::SaveInventory tmp = cso1->inventory;
      cso1->inventory = cso2->inventory;
      cso2->inventory = tmp;
    }
  }
  else{
    Inventory* inv = c1->getInventory();
    c1->setInventory(c2->getInventory());
    c2->setInventory(inv);
  }
  return 0;
}

int ScriptFunctions::enableMenu(lua_State* L){
  NUM_ARGS(1, 1);
  bool enable = ctx.stack().get(1).getBool();
  Engine::instance()->enableMenu(enable);
  return 0;
}

int ScriptFunctions::setTransparency(lua_State* L){
  NUM_ARGS(1, 1);
  int transparency = ctx.stack().get(1).getInt();
  int opacity = 255-transparency*255/100;
  std::string roomname = Engine::instance()->getData()->getProjectSettings()->anywhere_room;
  if (!roomname.empty()){
    RoomObject* room = Engine::instance()->getRoom(roomname);
    if (room)
      room->setOpacity(opacity);
  }
  roomname = Engine::instance()->getData()->getProjectSettings()->taskroom;
  if (!roomname.empty()){
    RoomObject* room = Engine::instance()->getRoom(roomname);
    if (room)
      room->setOpacity(opacity);
  }
  return 0;
}

int ScriptFunctions::showAllText(lua_State* L){
  NUM_ARGS(0, 0);
  Engine::instance()->getFontRenderer()->enableTextouts(true);
  return 0;
}

int ScriptFunctions::instMouse(lua_State* L){
  NUM_ARGS(1, 1);
  int state = ctx.stack().get(1).getInt();
  CursorObject* cursor = Engine::instance()->getCursor();
  Engine::instance()->getInterpreter()->setPrevState(cursor, cursor);
  cursor->setState(state+1);
  return 0;
}

int ScriptFunctions::showInventory(lua_State* L){
  NUM_ARGS(1, 1);
  int inventory = ctx.stack().get(1).getInt();
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr == NULL){
    TR_WARN("No character focussed");
    return 0;
  }
  Inventory* inv = chr->getInventory();
  inv->setCurrent(inventory);
  return 0;
}

int ScriptFunctions::setObjLight(lua_State* L){
  NUM_ARGS(4, 5);
  std::string objname = ctx.stack().get(1).getString();
  //remove whitespaces in object names
  for(int size = (int)objname.size()-1; size >= 0; --size){
    if (objname[size] == ' ')
      objname.erase(size, 1);
  }
  Color c;
  c.r = (unsigned char)ctx.stack().get(2).getInt();
  c.g = (unsigned char)ctx.stack().get(3).getInt();
  c.b = (unsigned char)ctx.stack().get(4).getInt();
  bool fade = false;
  if (numArgs >= 5){
    std::string fading = ctx.stack().get(5).getString();
    if (fading == "fade")
      fade = true;
  }
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj){
    if (fade && !ctx.mSkip){
      //do not block right now.
      //ctx.mSuspended = true;
      Engine::instance()->getAnimator()->add(obj, c);
    }
    else{
      obj->setLightingColor(c);
    }
  }
  else{
    std::string room;
    SaveStateProvider::SaveObject* so = Engine::instance()->getSaver()->findObject(objname, room);
    so->lighting = c;
  }
  return 0;
}

int ScriptFunctions::textAlign(lua_State* L){
  NUM_ARGS(2, 2);
  int num = ctx.stack().get(1).getInt();
  std::string alignstr = ctx.stack().get(2).getString();
  Textout::Alignment align;
  if (alignstr == "left")
    align = Textout::LEFT;
  else if (alignstr == "center")
    align = Textout::CENTER;
  else if (alignstr == "right")
    align = Textout::RIGHT;
  Textout* text = Engine::instance()->getFontRenderer()->getTextout(num);
  text->setAlignment(align);
  return 0;
}

int ScriptFunctions::runSpeed(lua_State* L){
  NUM_ARGS(1, 1);
  float speed = ctx.stack().get(1).getInt()/100.0f;
  Engine::instance()->getInterpreter()->setRunSpeed(speed);
  return 0;
}

int ScriptFunctions::runTo(lua_State* L){
  return moveTo(L, Engine::instance()->getInterpreter()->getRunSpeed());
}

int ScriptFunctions::enableFXShape(lua_State* L){
  NUM_ARGS(1, 1);
  bool enable = ctx.stack().get(1).getBool();
  Engine::instance()->enableFXShapes(enable);
  return 0;
}

int ScriptFunctions::scrollSpeed(lua_State* L){
  NUM_ARGS(1, 1);
  int speed = ctx.stack().get(1).getInt();
  bool follow = true;
  if (speed > 100){
    follow = false;
    speed -= 100;
  }
  Engine::instance()->setScrollSpeed(speed*0.0375f, follow);
  return 0;
}

int ScriptFunctions::loadChar(lua_State* L){
  NUM_ARGS(1, 1);
  std::string name = ctx.stack().get(1).getString();
  std::string dummy;
  CharacterObject* ch = Engine::instance()->loadCharacter(name, Engine::instance()->getCharacterClass(name), &ctx);
  ch->realize();
  Engine::instance()->disposeCharacter(ch);
  return 0;
}

int ScriptFunctions::offTextColor(lua_State* L){
  NUM_ARGS(3, 3);
  Color col;
  col.r = ctx.stack().get(1).getInt();
  col.g = ctx.stack().get(2).getInt();
  col.b = ctx.stack().get(3).getInt();
  Engine::instance()->getInterpreter()->setOfftextColor(col);
  return 0;
}

int ScriptFunctions::setItem(lua_State* L){
  NUM_ARGS(2, ARG_MAX);
  if (numArgs > 2)
    TR_BREAK("Implement me");
  std::string itemname = ctx.stack().get(1).getString();
  int state = ctx.stack().get(2).getInt();
  Object2D* item = Engine::instance()->getObject(itemname, true);
  if (item)
    item->setState(state);
  Engine::instance()->getInterpreter()->setItemState(itemname, state);
  return 0;
}

int ScriptFunctions::sqrt(lua_State* L){
  NUM_ARGS(1, 1);
  String variable = ctx.stack().get(1).getString();
  float val = Engine::instance()->getInterpreter()->getVariable(variable).getFloat();
  val = sqrtf(val);
  Engine::instance()->getInterpreter()->setVariable(variable, val);
  return 0;
}

int ScriptFunctions::switchCharacter(lua_State* L){
  NUM_ARGS(2, 2);
  String char1 = ctx.stack().get(1).getString();
  String char2 = ctx.stack().get(2).getString();
  CharacterObject* c1 = ctx.getCharacter(char1);
  CharacterObject* c2 = ctx.getCharacter(char2);
  if (c2 != NULL && c1 == NULL){
    c1 = c2;
    c2 = NULL;
    String tmp = char1;
    char1 = char2;
    char2 = tmp;
  }
  if (c1 != NULL && c2 == NULL){
    std::string room;
    std::string realname;
    SaveStateProvider::CharSaveObject* cso2 = Engine::instance()->getSaver()->findCharacter(char2, room, realname);
    //save char1
    Engine::instance()->extractCharacter(char1);
    std::string activeRoom = c1->getRoom();
    c1->setRoom(room);
    SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(room);
    c1->save(sr);
    //exchange save state
    SaveStateProvider::CharSaveObject* cso1 = Engine::instance()->getSaver()->getCharacter(sr, c1->getName());
    String tmp = cso2->base.name;
    cso2->base.name = cso1->base.name;
    cso1->base.name = tmp;
    SaveStateProvider::CharSaveObject tmpcso = *cso2;
    *cso2 = *cso1;
    *cso1 = tmpcso;
    //load char2
    c2 = Engine::instance()->loadCharacter(char2, Engine::instance()->getCharacterClass(char2), &ctx);
    Engine::instance()->getSaver()->removeCharacter(sr, realname);
    c2->setRoom(activeRoom);
    c2->realize();
    RoomObject* ro = Engine::instance()->getRoom("");
    c2->setScale(ro->getDepthScale(c2->getPosition()));
    if (Engine::instance()->getCharacter("self") == c1){
      c2->setPosition(c2->getPosition()+ro->getScrollOffset());
      Engine::instance()->setFocus(c2);
    }
    else
      ro->addObject(c2);
    Engine::instance()->disposeCharacter(c1);
  }
  else if (c1 != NULL && c2 != NULL){
    TR_BREAK("Implement me 1");
  }
  else if (c1 == NULL && c2 == NULL){
    SaveStateProvider::CharSaveObject* cso1 = Engine::instance()->getSaver()->findCharacter(char1);
    SaveStateProvider::CharSaveObject* cso2 = Engine::instance()->getSaver()->findCharacter(char2);
    String tmp = cso2->base.name;
    cso2->base.name = cso1->base.name;
    cso1->base.name = tmp;
    SaveStateProvider::CharSaveObject tmpcso = *cso2;
    *cso2 = *cso1;
    *cso1 = tmpcso;
  }
  return 0;
}

int ScriptFunctions::moveText(lua_State* L){
  NUM_ARGS(4, 5);
  int id = ctx.stack().get(1).getInt();

  Vec2i newpos;
  newpos.x = ctx.stack().get(2).getInt();
  newpos.y = ctx.stack().get(3).getInt();
  int speed = ctx.stack().get(4).getInt();
  bool hold = false;
  if (numArgs >= 5){
    std::string wait = ctx.stack().get(5).getString();
    if (wait == "wait")
      hold = true;
  }

  Textout* text = Engine::instance()->getFontRenderer()->getTextout(id);

  if (speed == 0 || ctx.mSkip){
    TR_DETAIL("textout %i positioned to %i %i", id, newpos.x, newpos.y);
    Engine::instance()->getAnimator()->remove(text);
    text->setPosition(newpos);
  }
  else{
    std::list<Vec2i> path;
    //path.push_back(obj->getPosition());
    path.push_back(newpos);
    float factor;
    if (speed <= 10)
      factor = (float)(11-speed);
    else{
      //we have ms => calculate speedfactor
      factor = newpos.length()*20.0f/speed;
    }
    if (hold){
      ctx.suspend(0, NULL/*new PositionSuspender(obj, newpos)*/);
      text->setSuspensionScript(&ctx);
    }
    Engine::instance()->getAnimator()->add(text, path, factor);
  }
  return 0;
}

int ScriptFunctions::dummy(lua_State* L){
  NUM_ARGS(0, ARG_MAX);
  for (unsigned i = 0; i < numArgs; ++i){
    ctx.stack().get(i+1);
  }
  return 0;
}

int ScriptFunctions::isBoolEqual(lua_State* L){
  NUM_ARGS(2, 2);
  String boolname = ctx.stack().get(1).getString();
  bool test = ctx.stack().get(2).getBool();
  lua_getglobal(L, "bool");
  lua_getfield(L, -1, boolname.c_str());
  bool saved = lua_toboolean(L, -1) != 0;
  lua_pop(L, 2);
  ctx.stack().push(saved ? 1 : 0);
  ctx.stack().push(test ? 1 : 0);
  return 2;
}

int ScriptFunctions::isObjectInState(lua_State* L){
  NUM_ARGS(2, 2);
  String objname = ctx.stack().get(1).getString();
  objname = objname.removeAll(' ');
  int checkstate = ctx.stack().get(2).getInt();
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj){
    ctx.stack().push(obj->getState());
  }
  else{
    std::string room;
    SaveStateProvider::SaveObject* so = Engine::instance()->getSaver()->findObject(objname, room);
    if (!so){
      ctx.stack().push(0);
    }
    else
      ctx.stack().push(so->state);
  }
  ctx.stack().push(checkstate);
  return 2;
}

int ScriptFunctions::isCommandSet(lua_State* L){
  NUM_ARGS(0, 1);
  EngineEvent check = EVT_NONE;
  if (numArgs >= 1){
    std::string evtname = ctx.stack().get(1).getString();
    check = Engine::instance()->getInterpreter()->getEngineEvent(evtname);
  }
  EngineEvent evt = ctx.getCommandEvent();
  if (evt == EVT_NONE)
    evt = Engine::instance()->getActiveCommandAsEvent();
  ctx.stack().push(evt);
  ctx.stack().push(check);
  return 2;
}

int ScriptFunctions::isLinkedObject(lua_State* L){
  NUM_ARGS(1, 1);
  std::string objname = ctx.stack().get(1).getString();
  std::string linkname = ctx.getUseObjectName();
  ctx.stack().push(0);
  ctx.stack().push(_stricmp(linkname.c_str(), objname.c_str()));
  return 2;
}

int ScriptFunctions::isGiveLinkedObject(lua_State* L){
  NUM_ARGS(1, 1);
  std::string objname = ctx.stack().get(1).getString();
  std::string linkname = ctx.getGiveObjectName();
  ctx.stack().push(0);
  ctx.stack().push(_stricmp(linkname.c_str(), objname.c_str()));
  return 2;
}

int ScriptFunctions::isNumEqual(lua_State* L){
  NUM_ARGS(2, 2);
  String varname = ctx.stack().get(1).getString();
  int test = ctx.stack().get(2).getInt();
  int saved = Engine::instance()->getInterpreter()->getVariable(varname).getInt();
  ctx.stack().push(saved);
  ctx.stack().push(test);
  return 2;
}

int ScriptFunctions::isCharFocussed(lua_State* L){
  NUM_ARGS(1, 1);
  std::string name = ctx.stack().get(1).getString();
  ctx.stack().push(0);
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr && _stricmp(chr->getName().c_str(), name.c_str()) == 0)
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

int ScriptFunctions::isCharTriggering(lua_State* L){
  NUM_ARGS(1, 1);
  std::string name = ctx.stack().get(1).getString();
  ctx.stack().push(0);
  CharacterObject* chr = ctx.getCharacter(name);
  if (!chr){ //when the character is not found, he or she is not in the current room, so no triggering possible
    ctx.stack().push(3);
    return 2;
  }
  RoomObject* room = Engine::instance()->getRoom("");
  if (room){
    //find the position of the script
    Vec2i pos = room->getScriptPosition(&ctx);
    Vec2i charpos = chr->getPosition();
    charpos.x = (int)(charpos.x/room->getWalkGridSize());
    charpos.y = (int)(charpos.y/room->getWalkGridSize());
    if (pos == charpos)
      ctx.stack().push(0);
    else
      ctx.stack().push(1);
  }
  else{
    TR_USE(ADV_ScriptFunc);
    ctx.stack().push(2);
    TR_BREAK("Room not found");
  }
  return 2;
}

int ScriptFunctions::isCharInRoom(lua_State* L){
  NUM_ARGS(2, 2);
  std::string charname = ctx.stack().get(1).getString();
  std::string roomname = ctx.stack().get(2).getString();
  ctx.stack().push(0);
  std::string room;
  std::string name;
  CharacterObject* chr = ctx.getCharacter(charname);
  if (chr)
    room = chr->getRoom();
  else{
    SaveStateProvider::CharSaveObject* chs = Engine::instance()->getSaver()->findCharacter(charname, room, name);
  }
  if (_stricmp(room.c_str(), roomname.c_str()) == 0)
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  //RoomObject* room = Engine::instance()->getRoom(ro);
  return 2;
}

int ScriptFunctions::isCharPossessingItem(lua_State* L){
  NUM_ARGS(2, 2);
  std::string charname = ctx.stack().get(1).getString();
  std::string itemname = ctx.stack().get(2).getString();
  ctx.stack().push(0);
  CharacterObject* chr = ctx.getCharacter(charname);
  if (!chr){
    if (charname == "self"){ //no focussed char, so he cannot possess it
      ctx.stack().push(1);
      return 2;
    }
    std::string room;
    std::string realName;
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname, room, realName);
    bool found = false;
    for (std::map<int,std::vector<SaveStateProvider::SaveItem> >::iterator iter = cso->inventory.items.begin(); iter != cso->inventory.items.end(); ++iter){
      for (unsigned i = 0; i < iter->second.size(); ++i){
        if (_stricmp(itemname.c_str(), iter->second[i].name.c_str()) == 0){
          found = true;
          break;
        }
      }
      if (found)
        break;
    }
    ctx.stack().push(found ? 0 : 1);
    return 2;
  }
  if (chr->getInventory()->getItem(itemname))
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

int ScriptFunctions::isKeyDownEqual(lua_State* L){
  NUM_ARGS(1, 1);
  StackData sd = ctx.stack().get(1);
  std::string key = sd.getString();
  if (key.empty()){
    char tmp[16];
    int k = sd.getInt();
    sprintf(tmp, "%i", k);
    key = tmp;
  }
  std::map<std::string,int>::iterator iter = Engine::instance()->getInterpreter()->mKeymap.find(key);
  if (iter == Engine::instance()->getInterpreter()->mKeymap.end())
    TR_BREAK("Unknown key %s", key.c_str());
  int keycode = iter->second;
  ctx.stack().push(0);
  if (Engine::instance()->isKeyDown(keycode))
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

int ScriptFunctions::isKeyPressedEqual(lua_State* L){
  NUM_ARGS(1, 1);
  std::string key = ctx.stack().get(1).getString();
  std::map<std::string,int>::iterator iter = Engine::instance()->getInterpreter()->mKeymap.find(key);
  if (iter == Engine::instance()->getInterpreter()->mKeymap.end())
    TR_BREAK("Unknown key %s", key.c_str());
  int keycode = iter->second;
  ctx.stack().push(0);
  if (Engine::instance()->isKeyPressed(keycode))
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

int ScriptFunctions::isStringEqual(lua_State* L){
  NUM_ARGS(2, 2);
  String name = ctx.stack().get(1).getString();
  String text = ctx.stack().get(2).getString();
  String val = Engine::instance()->getInterpreter()->getVariable(name).getString();
  ctx.stack().push(0);
  ctx.stack().push(_stricmp(val.removeAll(' ').c_str(), text.removeAll(' ').c_str()));
  return 2;
}

int ScriptFunctions::isCurrentRoom(lua_State* L){
  NUM_ARGS(1, 1);
  std::string room = ctx.stack().get(1).getString();
  ctx.stack().push(0);
  RoomObject* ro = Engine::instance()->getRoom("");
  if (!ro){
    ctx.stack().push(1);
    return 2;
  }
  ctx.stack().push(_stricmp(room.c_str(), ro->getName().c_str()));
  return 2;
}

int ScriptFunctions::isMouseWheelEqual(lua_State* L){
  NUM_ARGS(1, 1);
  std::string dir = ctx.stack().get(1).getString();
  dir = toLower(dir);
  ctx.stack().push(0);
  int delta = Engine::instance()->getMouseWheelDelta();
  if (dir == "up"){
    if (delta > 0){
      Engine::instance()->setMouseWheelDelta(delta-1);
      ctx.stack().push(0);
    }
    else
      ctx.stack().push(1);
  }
  else if (dir == "down"){
    if (delta < 0){
      Engine::instance()->setMouseWheelDelta(delta+1);
      ctx.stack().push(0);
    }
    else
      ctx.stack().push(1);
  }
  else
    TR_BREAK("Unknown direction %s", dir.c_str());
  return 2;
}

int ScriptFunctions::isObjXPosEqual(lua_State* L){
  NUM_ARGS(2, 2);
  std::string objname = ctx.stack().get(1).getString();
  int xpos = ctx.stack().get(2).getInt();
  Object2D* obj = Engine::instance()->getObject(objname, false);
  TR_DETAIL("eval if: object %s is at x %i", objname.c_str(), obj->getPosition().x);
  if (obj == NULL){
    std::string dummy;
    SaveStateProvider::SaveObject* so = Engine::instance()->getSaver()->findObject(objname, dummy);
    if (so == NULL)
      TR_BREAK("Unknown object %s", objname.c_str());
    ctx.stack().push(so->position.x);
  }
  else
    ctx.stack().push(obj->getPosition().x);
  ctx.stack().push(xpos);
  return 2;
}

int ScriptFunctions::isObjYPosEqual(lua_State* L){
  NUM_ARGS(2, 2);
  std::string objname = ctx.stack().get(1).getString();
  int ypos = ctx.stack().get(2).getInt();
  Object2D* obj = Engine::instance()->getObject(objname, false);
  TR_DETAIL("eval if: object %s is at y %i", objname.c_str(), obj->getPosition().y);
  if (obj == NULL){
    std::string dummy;
    SaveStateProvider::SaveObject* so = Engine::instance()->getSaver()->findObject(objname, dummy);
    if (so == NULL)
      TR_BREAK("Unknown object %s", objname.c_str());
    ctx.stack().push(so->position.y);
  }
  else
    ctx.stack().push(obj->getPosition().y);
  ctx.stack().push(ypos);
  return 2;
}

int ScriptFunctions::isItemInState(lua_State* L){
  NUM_ARGS(2, 2);
  std::string itemname = ctx.stack().get(1).getString();
  int checkstate = ctx.stack().get(2).getInt();
  Object2D* item = Engine::instance()->getObject(itemname, true);
  if (item){
    ctx.stack().push(item->getState());
  }
  else{
    int state = Engine::instance()->getInterpreter()->getItemState(itemname);
    ctx.stack().push(state);
  }
  ctx.stack().push(checkstate);
  return 2;
}
