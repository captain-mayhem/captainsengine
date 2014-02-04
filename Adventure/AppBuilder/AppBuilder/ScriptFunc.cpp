#include "ScriptFunc.h"

#include <fstream>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>

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
  interpreter->registerFunction("enablefxshape", enableFXShape);
  interpreter->registerFunction("scrollspeed", scrollSpeed);
  interpreter->registerFunction("loadchar", loadChar);
  interpreter->registerFunction("offtextcolor", offTextColor);
  interpreter->registerFunction("setitem", setItem);
  srand((unsigned)time(NULL));
}

int ScriptFunctions::loadRoom(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string room = ctx.stack().pop().getString();
  ScreenChange change = Engine::instance()->getScreenChange();
  if (numArgs == 2){
    std::string changename = ctx.stack().pop().getString();
    change = getScreenChange(changename);
  }
  Engine::instance()->loadMainRoom(room, &ctx, change);
  return 0;
}

int ScriptFunctions::setFocus(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string character = ctx.stack().pop().getString();
  Engine::instance()->setFocus(character, &ctx);
  return 0;
}

int ScriptFunctions::showInfo(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string text = ctx.stack().pop().getString();
  bool show = ctx.stack().pop().getBool();
  if (show){
    Engine::instance()->setObjectTooltipString(text);
  }
  else
    Engine::instance()->setObjectTooltipString("");
  Engine::instance()->setObjectString(text);
  return 0;
}

int ScriptFunctions::walkTo(ExecutionContext& ctx, unsigned numArgs){
  return moveTo(ctx, numArgs, 1.0f);
}

int ScriptFunctions::moveTo(ExecutionContext& ctx, unsigned numArgs, float speedFactor){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 3 || numArgs > 5)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string character = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt()-1;
  pos.y = ctx.stack().pop().getInt()-1;
  LookDir dir = UNSPECIFIED;
  bool hold = true;
  if (numArgs >= 4){
    StackData sd = ctx.stack().pop();
    if (sd.isString()){
      if (sd.getString() == "dontwait")
        hold = false;
    }
    else
      dir = (LookDir)(sd.getInt()-1);
  }
  if (numArgs >= 5){
    std::string dw = ctx.stack().pop().getString();
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

int ScriptFunctions::speech(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2 || numArgs > 4)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string character = ctx.stack().pop().getString();
  std::string text = ctx.stack().pop().getString();
  std::string sound = "";
  bool hold = Engine::instance()->getInterpreter()->isBlockingScriptRunning() || ctx.isLoop1() || ctx.isIdle();
  if (numArgs >= 3){ //TODO SOUND
    sound = ctx.stack().pop().getString();
    if (sound == "dontwait"){
      hold = false;
      sound = "";
    }
  }
  if (numArgs >= 4){
    std::string dw = ctx.stack().pop().getString();
    if (dw == "dontwait")
      hold = false;
  }
  if (ctx.mSkip)
    return 0;
  CharacterObject* chr = ctx.getCharacter(character);
  FontRenderer::String* str = NULL;
  SoundPlayer* plyr = NULL;
  if (chr){
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
      DEPTH_GAME_FONT, chr->getFontID(), breakinfo, chr->getTextColor(), plyr ? 100000 : Engine::instance()->getInterpreter()->getTextSpeed()*text.length());
    //stop speaking
    Engine::instance()->getFontRenderer()->removeText(chr);
    if (str)
      str->setSpeaker(chr);
    chr->setTalking(true);
  }
  if (hold){
    if (plyr){
      plyr->setSuspensionScript(&ctx);
    }
    else if (str){
      str->setSuspensionScript(&ctx);
    }
    ctx.mSuspended = true;
  }
  return 0;
}

int ScriptFunctions::pickup(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string character = ctx.stack().pop().getString();
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

int ScriptFunctions::playSound(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string sound = ctx.stack().pop().getString();
  unsigned volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().pop().getInt();
  if (ctx.mSkip)
    return 0;
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_CREATE_ALWAYS);
  if (sp){
    sp->setVolume(volume/100.0f);
    sp->play(false);
  }
  return 0;
}

int ScriptFunctions::setLight(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 4 || numArgs > 5)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string room = ctx.stack().pop().getString();
  Color c;
  c.r = (unsigned char)ctx.stack().pop().getInt();
  c.g = (unsigned char)ctx.stack().pop().getInt();
  c.b = (unsigned char)ctx.stack().pop().getInt();
  bool fade = false;
  if (numArgs >= 5)
    fade = ctx.stack().pop().getString() == "fade";
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
    sr->overlaylighting = c;
  }
  return 0;
}

int ScriptFunctions::setBool(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String boolname = ctx.stack().pop().getString();
  bool val = ctx.stack().pop().getBool();
  Engine::instance()->getInterpreter()->mBooleans[boolname] = val;
  return 0;
}

void ScriptFunctions::setObjInternal(std::vector<std::string> objects, std::vector<int> states, bool skip){
  TR_USE(ADV_ScriptFunc);
  for (unsigned objiter = 0; objiter < objects.size(); ++objiter){
    Object2D* obj = Engine::instance()->getObject(objects[objiter], false);
    SaveStateProvider::SaveObject* so = NULL;
    if (obj){
      obj->setState(states[0]);
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

int ScriptFunctions::setObj(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string objname = ctx.stack().pop().getString();
  //remove whitespaces in object names
  for(int size = objname.size()-1; size >= 0; --size){
    if (objname[size] == ' ')
      objname.erase(size, 1);
  }
  //TR_DEBUG("obj: %s", objname.c_str());
  int state = ctx.stack().pop().getInt();
  ObjectGroup* grp = Engine::instance()->getInterpreter()->getGroup(objname);
  std::vector<std::string> objects;
  if (grp)
    objects = grp->getObjects();
  else
    objects.push_back(objname);
  std::vector<int> states;
  states.push_back(state);
  for (unsigned i = 2; i < numArgs; ++i){
    state = ctx.stack().pop().getInt();
    states.push_back(state);
  }
  setObjInternal(objects, states, ctx.isSkipping());
  return 0;
}

int ScriptFunctions::beamTo(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 3 || numArgs > 5)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String charname = ctx.stack().pop().getString();
  StackData arg = ctx.stack().top();
  std::string roomname;
  if (!arg.isString()){
    ++numArgs;
  }
  else
    roomname = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt()-1;
  pos.y = ctx.stack().pop().getInt()-1;
  LookDir dir = UNSPECIFIED;
  if (numArgs >= 5)
    dir = (LookDir)(ctx.stack().pop().getInt()-1);
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
      Engine::instance()->getFontRenderer()->removeText(obj);
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
      obj->setDepth((int)(obj->getPosition().y/Engine::instance()->getWalkGridSize(false)));
      //obj->setScale(ro->getDepthScale(obj->getPosition()));
    }
  }
  else{
    //resolve the self
    String realcharname = ctx.resolveCharName(charname);
    CharacterObject* obj = Engine::instance()->extractCharacter(realcharname);
    if (!obj){
      obj = Engine::instance()->loadCharacter(realcharname, Engine::instance()->getCharacterClass(realcharname), false, &ctx);
      SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(obj->getRoom());
      Engine::instance()->getSaver()->removeCharacter(sr, obj->getName());
      obj->realize();
    }
    if (obj){
      obj->abortClick();   
      obj->setTalking(false);
      obj->setWalking(false);
      if (dir != UNSPECIFIED)
        obj->setLookDir(dir);
      if (roomname.empty())
        roomname = obj->getRoom();
      RoomObject* room = Engine::instance()->getRoom(roomname);
      SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(roomname);
      if (room){
        obj->setRoom(room->getName());
        obj->setPosition(Vec2i(pos*room->getWalkGridSize()+Vec2f(room->getWalkGridSize()/2, room->getWalkGridSize()/2)));
        obj->setScale(room->getDepthScale(obj->getPosition()));
        obj->setDepth((int)(obj->getPosition().y/Engine::instance()->getWalkGridSize(false)));
        Engine::instance()->getSaver()->removeCharacter(sr, obj->getName());
        room->addObject(obj);
      }
      else{
        obj->setRoom(roomname);
        Engine::instance()->getSaver()->removeCharacter(sr, obj->getName());
        obj->setPosition(Vec2i(pos*sr->getWalkGridSize()+Vec2f(sr->getWalkGridSize()/2, sr->getWalkGridSize()/2)));
        obj->save(sr);
        delete obj;
      }
    }
  }
  return 0;
}

int ScriptFunctions::addItem(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2 || numArgs > 3)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string charname = ctx.stack().pop().getString();
  std::string itemname = ctx.stack().pop().getString();
  if (itemname == "givelink")
    itemname = Engine::instance()->getGiveObjectName();
  int inventory = 1;
  if (numArgs >= 3)
    inventory = ctx.stack().pop().getInt();
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

int ScriptFunctions::cutScene(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string scriptname = ctx.stack().pop().getString();
  Engine::instance()->getInterpreter()->cutsceneMode(true);
  if (!ctx.getEvents().empty() && ctx.getEvents().front() == EVT_CLICK){ //the cutscene should stop the current click
    ctx.getEvents().pop_front(); //stop click
    ctx.getEvents().pop_front(); //stop user event
  }
  ExecutionContext* context = Engine::instance()->loadScript(Script::CUTSCENE, scriptname);
  Engine::instance()->getInterpreter()->executeCutscene(context, false);
  return 0;
}

int ScriptFunctions::taskbar(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool state = ctx.stack().pop().getBool();
  Engine::instance()->showTaskbar(state);
  return 0;
}

int ScriptFunctions::follow(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2 || numArgs > 3)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string char1 = ctx.stack().pop().getString();
  std::string char2 = ctx.stack().pop().getString();
  bool hold = true;
  if (numArgs >= 3){
    if (ctx.stack().pop().getString() == "dontwait"){
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

int ScriptFunctions::lookTo(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs!= 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string character = ctx.stack().pop().getString();
  StackData d = ctx.stack().pop();
  LookDir dir = UNSPECIFIED;
  CharacterObject* chr1 = ctx.getCharacter(character);
  if (d.getInt() != 0){
    dir = (LookDir)(d.getInt()-1);
    if (chr1)
      chr1->setLookDir(dir);
    else{
      std::string dummy;
      SaveStateProvider::CharSaveObject* chs = Engine::instance()->getSaver()->findCharacter(character, dummy, dummy);
      if (!chs){
        TR_USE(ADV_ScriptFunc);
        TR_BREAK("Character %s not found", character.c_str());
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

int ScriptFunctions::textScene(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 4)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string scenename = ctx.stack().pop().getString();
  Vec2i pos(0,Engine::instance()->getResolution().y);
  int width = Engine::instance()->getResolution().x;
  if (numArgs > 1){
    pos.x = ctx.stack().pop().getInt();
    if (numArgs > 2)
      pos.y = ctx.stack().pop().getInt();
    if (numArgs > 3)
      width = ctx.stack().pop().getInt();
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
  ExecutionContext* context = Engine::instance()->loadScript(Script::CUTSCENE, scenename);
  Engine::instance()->getInterpreter()->executeCutscene(context, true);
  return 0;
}

int ScriptFunctions::delItem(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2 || numArgs > 3)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string charname = ctx.stack().pop().getString();
  std::string itemname = ctx.stack().pop().getString();
  if (itemname == "givelink"){
    itemname = Engine::instance()->getGiveObjectName();
  }
  int inventory = 1;
  if (numArgs >= 3)
    inventory = ctx.stack().pop().getInt();
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

int ScriptFunctions::loopSound(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string sound = ctx.stack().pop().getString();
  int volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().pop().getInt();
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_DEFAULT);
  if (sp){
    sp->setVolume(volume/100.0f);
    sp->play(true);
  }
  return 0;
}

int ScriptFunctions::loopStop(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string sound = ctx.stack().pop().getString();
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_DEFAULT);
  if (sp)
    sp->stop();
  return 0;
}

int ScriptFunctions::playMusic(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string music = ctx.stack().pop().getString();
  std::string pattern;
  if (numArgs >= 2){
    pattern = ctx.stack().pop().getString();
    if (!pattern.empty()){
      TR_USE(ADV_ScriptFunc);
      TR_BREAK("patterns in playMusic need to be implemented %s", pattern.c_str());
    }
  }
  SoundPlayer* sp = SoundEngine::instance()->getMusic(music);
  if (sp){
    sp->play(true);
  }
  return 0;
}

int ScriptFunctions::stopMusic(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  SoundPlayer* sp = SoundEngine::instance()->getMusic("");
  if (sp){
    sp->fadeVolume(SoundEngine::instance()->getMusicVolume(), 0.0f, SoundEngine::instance()->getFadingTime());
  }
  return 0;
}

int ScriptFunctions::wait(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  float seconds = ctx.stack().pop().getFloat();
  if (ctx.mSkip)
    return 0;
  ctx.mSleepTime = (int)(seconds*1000);
  ctx.mSuspended = true;
  return 0;
}

int ScriptFunctions::subRoom(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string roomname = ctx.stack().pop().getString();
  int fading_time = 0;
  if (numArgs >= 2){
    fading_time = ctx.stack().pop().getInt();
  }
  if (Engine::instance()->isSubRoomLoaded())
    Engine::instance()->unloadRoom(NULL, false, false);
  Engine::instance()->loadSubRoom(roomname, &ctx, fading_time);
  return 0;
}

int ScriptFunctions::subRoomReturn(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Engine::instance()->unloadRoom(NULL, false, false);
  return 0;
}

int ScriptFunctions::subRoomReturnImmediate(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Engine::instance()->unloadRoom(NULL, false, true);
  return 0;
}

int ScriptFunctions::link(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string objectname = ctx.stack().pop().getString();
  Engine::instance()->setUseObject(objectname, ctx.mObjectInfo);
  PcdkScript::mRemoveLinkObject = false;
  return 0;
}

int ScriptFunctions::giveLink(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs > 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Object2D* obj = NULL;
  if (numArgs >= 1){
    std::string objectname = ctx.stack().pop().getString();
    obj = Engine::instance()->getObject(objectname, true);
  }
  else{
    obj = ctx.mOwner;
  }
  Engine::instance()->setGiveObject(obj->getName(), ctx.mObjectInfo);
  PcdkScript::mRemoveLinkObject = false;
  return 0;
}

int ScriptFunctions::setNum(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String varname = ctx.stack().pop().getString();
  float val = ctx.stack().pop().getFloat();
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(val));
  return 0;
}

int ScriptFunctions::offSpeech(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 3 || numArgs > 5)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Vec2i pos;
  float walkgridsize;
  RoomObject* room = Engine::instance()->getRoom("");
  if (room){
    walkgridsize = room->getWalkGridSize();
  }
  else
    walkgridsize = Engine::instance()->getWalkGridSize(false);
  pos.x = (int)(ctx.stack().pop().getInt()*walkgridsize);
  pos.y = (int)(ctx.stack().pop().getInt()*walkgridsize);
  std::string text = ctx.stack().pop().getString();
  std::string sound = "";
  bool hold = Engine::instance()->getInterpreter()->isBlockingScriptRunning() || ctx.isLoop1() || ctx.isIdle();
  if (numArgs >= 4){
    sound = ctx.stack().pop().getString();
    if (sound == "dontwait"){
      hold = false;
      sound = "";
    }
  }
  if (numArgs >= 5){
    std::string dw = ctx.stack().pop().getString();
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
    DEPTH_GAME_FONT, fontid, breakinfo, Engine::instance()->getInterpreter()->getOfftextColor(), plyr ? 100000 : Engine::instance()->getInterpreter()->getTextSpeed()*text.length());
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

int ScriptFunctions::unloadRoom(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool animate = !ctx.isSkipping();
  Engine::instance()->unloadRoom(NULL, true, false);
  //make unloads smoother. triggerScreenchange, add finished callback to screenchanges, call ForceNot... in callback
  if (animate){
    //Engine::instance()->getRoom("")->setFadeout(1);
    Engine::instance()->triggerScreenchange(NULL, Engine::instance()->getScreenChange(), true);
  }
  else
    Engine::instance()->forceNotToRenderUnloadingRoom();
  return 0;
}

int ScriptFunctions::restart(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Engine::instance()->reset();
  return 0;
}

int ScriptFunctions::gotoLevel(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int level = ctx.stack().pop().getInt();
  Engine::instance()->getInterpreter()->mNextTSLevel = level;
  return 0;
}

int ScriptFunctions::deactivate(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 3)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string scene = ctx.stack().pop().getString();
  int level = ctx.stack().pop().getInt();
  int row = ctx.stack().pop().getInt();
  if (scene == "self")
    scene = Engine::instance()->getInterpreter()->mTSName;
  Engine::instance()->getInterpreter()->mTSActive[scene][level][row] = false;
  return 0;
}

int ScriptFunctions::endScene(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  ctx.mExecuteOnce = true;
  Engine::instance()->clearGui();
  return 0;
}

int ScriptFunctions::instObj(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string objname = ctx.stack().pop().getString();
  int state = ctx.stack().pop().getInt();
  for (unsigned i = 2; i < numArgs; ++i){
    TR_USE(ADV_ScriptFunc);
    TR_BREAK("state lists - implement me"); //TODO state lists
    state = ctx.stack().pop().getInt();
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

int ScriptFunctions::command(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string cmd = ctx.stack().pop().getString();
  Engine::instance()->setCommand(cmd, true);
  return 0;
}

int ScriptFunctions::invDown(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int move = ctx.stack().pop().getInt();
  RoomObject* room = Engine::instance()->getContainingRoom(ctx.mOwner);
  int maxit = 1000;
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr)
    maxit = chr->getInventory()->getNumItemSlots();
  room->getInventory()->addScrollOffset(move, maxit);
  return 0;
}

int ScriptFunctions::invUp(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int move = ctx.stack().pop().getInt();
  RoomObject* room = Engine::instance()->getContainingRoom(ctx.mOwner);
  int maxit = 1000;
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr)
    maxit = chr->getInventory()->getNumItemSlots();
  room->getInventory()->addScrollOffset(-move, maxit);
  return 0;
}

int ScriptFunctions::setFont(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int fontid = ctx.stack().pop().getInt();
  if (!Engine::instance()->getFontRenderer()->loadFont(fontid))
    return 0;
  if (numArgs >= 2){
    CharacterObject* chr = ctx.getCharacter(ctx.stack().pop().getString());
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

int ScriptFunctions::setScreenchange(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  StackData data = ctx.stack().pop();
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

int ScriptFunctions::activate(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 3)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string scene = ctx.stack().pop().getString();
  int level = ctx.stack().pop().getInt();
  int row = ctx.stack().pop().getInt();
  if (scene == "self")
    scene = Engine::instance()->getInterpreter()->mTSName;
  Engine::instance()->getInterpreter()->mTSActive[scene][level][row] = true;
  return 0;
}

int ScriptFunctions::saveGame(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int slot = ctx.stack().pop().getInt();
  Engine::instance()->getSaver()->save(SaveStateProvider::saveSlotToPath(slot));
  return 0;
}

int ScriptFunctions::loadGame(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int slot = ctx.stack().pop().getInt();
  Engine::instance()->getSaver()->load(SaveStateProvider::saveSlotToPath(slot));
  return 0;
}

int ScriptFunctions::jiggle(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  float seconds = ctx.stack().pop().getFloat();
  int power = 10;
  if (numArgs >= 2)
    power = ctx.stack().pop().getInt();
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

int ScriptFunctions::randomNum(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String name = ctx.stack().pop().getString();
  int limit = ctx.stack().pop().getInt();
  int rnd = rand()%limit;
  Engine::instance()->getInterpreter()->setVariable(name, StackData(rnd+1));
  return 0;
}

int ScriptFunctions::getRequestedState(Character* cclass, const StackData& data){
  TR_USE(ADV_ScriptFunc);
  int state = 0;
  if (data.isNumber())
    state = data.getInt()+16;
  else{
    std::string statename = data.getString();
    bool found = false;
    for (unsigned i = 0; i < cclass->extrastatenames.size(); ++i){
      if (stricmp(cclass->extrastatenames[i].c_str(), statename.c_str()) == 0){
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

int ScriptFunctions::setChar(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string chrname = ctx.stack().pop().getString();
  StackData data = ctx.stack().pop();
  if (ctx.mSkip){
    for (unsigned i = 2; i < numArgs; ++i){
      ctx.stack().pop();
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
  if (obj){
    obj->clearNextStates();
    TR_DEBUG("setting new state %i for char %s", state, chrname.c_str());
    obj->setState(state);
  }
  for (unsigned i = 2; i < numArgs; ++i){
    data = ctx.stack().pop();
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

int ScriptFunctions::setString(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String varname = ctx.stack().pop().getString();
  String val = ctx.stack().pop().getString();
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(val));
  return 0;
}

int ScriptFunctions::loadNum(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String varname = ctx.stack().pop().getString();
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

int ScriptFunctions::saveNum(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String varname = ctx.stack().pop().getString();
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
  //insert new content
  data[varname] = val;
  //save content
  std::ofstream out(file.c_str());
  for (std::map<std::string,int>::iterator iter = data.begin(); iter != data.end(); ++iter){
    out << iter->first << " " << iter->second << std::endl;
  }
  out.close();
  return 0;
}

int ScriptFunctions::textEnabled(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool enabled = ctx.stack().pop().getBool();
  Engine::instance()->setTextEnabled(enabled);
  return 0;
}

int ScriptFunctions::realTime(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool enabled = ctx.stack().pop().getBool();
  //this is intentionally left blank, consider implementing if engine too slow otherwise
  return 0;
}

int ScriptFunctions::setCharLight(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 4 || numArgs > 5)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string charname = ctx.stack().pop().getString();
  Color c;
  c.r = (unsigned char)ctx.stack().pop().getInt();
  c.g = (unsigned char)ctx.stack().pop().getInt();
  c.b = (unsigned char)ctx.stack().pop().getInt();
  bool fade = false;
  if (numArgs >= 5){
    std::string fading = ctx.stack().pop().getString();
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
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname, room, name);
    cso->base.lighting = c;
  }
  return 0;
}

int ScriptFunctions::group(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string groupname = ctx.stack().pop().getString();
  ObjectGroup* grp = new ObjectGroup(groupname);
  for (unsigned i = 1; i < numArgs; ++i){
    std::string object = ctx.stack().pop().getString();
    grp->add(object);
  }
  Engine::instance()->getInterpreter()->mGroups.push_back(grp);
  return 0;
}

int ScriptFunctions::stopSkip(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  ctx.mSkip = false;
  return 0;
}

int ScriptFunctions::playSwf(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 5)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string moviename = ctx.stack().pop().getString();
  int x = 0;
  int y = 0;
  int width = Engine::instance()->getSettings()->resolution.x;
  int height = Engine::instance()->getSettings()->resolution.y;
  if (numArgs >= 2){
    x = ctx.stack().pop().getInt();
  }
  if (numArgs >= 3){
    y = ctx.stack().pop().getInt();
  }
  if (numArgs >= 4){
    width = ctx.stack().pop().getInt();
  }
  if (numArgs >= 5){
    height = ctx.stack().pop().getInt();
  }
  VideoPlayer* vp = SoundEngine::instance()->getMovie(moviename, true);
  if (vp){
    vp->initLayer(x, y, width, height);
    vp->play(false);
  }
  return 0;
}

int ScriptFunctions::stopSwf(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
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

int ScriptFunctions::playVideo(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2 || numArgs > 6)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string moviename = ctx.stack().pop().getString();
  bool suspend = ctx.stack().pop().getBool();
  int x = 0;
  int y = 0;
  int width = Engine::instance()->getSettings()->resolution.x;
  int height = Engine::instance()->getSettings()->resolution.y;
  if (numArgs >= 3){
    x = ctx.stack().pop().getInt();
  }
  if (numArgs >= 4){
    y = ctx.stack().pop().getInt();
  }
  if (numArgs >= 5){
    width = ctx.stack().pop().getInt();
  }
  if (numArgs >= 6){
    height = ctx.stack().pop().getInt();
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

int ScriptFunctions::setWalkmap(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 4)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string room = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt();
  pos.y = ctx.stack().pop().getInt();
  bool walkable = ctx.stack().pop().getBool();
  RoomObject* rm = Engine::instance()->getRoom(room);
  if (rm)
    rm->modifyWalkmap(pos, walkable);
  else{
    SaveStateProvider::SaveRoom* srm = Engine::instance()->getSaver()->getRoom(room);
    srm->walkmap[pos] = walkable;
  }
  return 0;
}

int ScriptFunctions::stepTo(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string name = ctx.stack().pop().getString();
  std::string dirname = ctx.stack().pop().getString();
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
    chr->setDepth((int)(pos.y/Engine::instance()->getWalkGridSize(false)));
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

int ScriptFunctions::moveObj(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 4 || numArgs > 5)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string name = ctx.stack().pop().getString();
  //remove whitespaces in object names
  for(int size = name.size()-1; size >= 0; --size){
    if (name[size] == ' ')
      name.erase(size, 1);
  }
  Vec2i newpos;
  newpos.x = ctx.stack().pop().getInt();
  newpos.y = ctx.stack().pop().getInt();
  int speed = ctx.stack().pop().getInt();
  bool hold = false;
  if (numArgs >= 5){
    std::string wait = ctx.stack().pop().getString();
    if (wait == "wait")
      hold = true;
  }
  Object2D* obj = Engine::instance()->getObject(name, false);
  if (obj == NULL){
    std::string room;
    SaveStateProvider::SaveObject* obj = Engine::instance()->getSaver()->findObject(name, room);
    if (obj)
      obj->position = newpos;
    else
      TR_ERROR("moveObj: Object %s does not exist", name.c_str());
    return 0;
  }
  if (speed == 0 || ctx.mSkip){
    TR_DETAIL("object %s positioned to %i %i", name.c_str(), newpos.x, newpos.y);
    Engine::instance()->getAnimator()->remove(obj);
    obj->setPosition(newpos);
    return 0;
  }
  std::list<Vec2i> path;
  //path.push_back(obj->getPosition());
  path.push_back(newpos);
  float factor;
  if (speed <= 10)
    factor = (float)(11-speed);
  else{
    //we have ms => calculate speedfactor
    factor = (newpos-obj->getPosition()).length()*20.0f/speed;
  }
  if (hold){
    ctx.suspend(0, NULL/*new PositionSuspender(obj, newpos)*/);
    obj->setSuspensionScript(&ctx);
  }
  Engine::instance()->getAnimator()->add(obj, path, factor);
  return 0;
}

int ScriptFunctions::quit(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Engine::instance()->quit();
  return 0;
}

int ScriptFunctions::musicVolume(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int volume = ctx.stack().pop().getInt();
  SoundEngine::instance()->setMusicVolume(volume/100.0f);
  return 0;
}

int ScriptFunctions::setParticles(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 6)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string object = ctx.stack().pop().getString();
  int speed = ctx.stack().pop().getInt();
  int amount = ctx.stack().pop().getInt();
  int direction = ctx.stack().pop().getInt();
  int rotation = ctx.stack().pop().getInt();
  int variation = ctx.stack().pop().getInt();
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

int ScriptFunctions::startParticles(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs > 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool fast = false;
  if (numArgs >= 1){
    std::string arg = ctx.stack().pop().getString();
    fast = arg == "fast";
  }
  Engine::instance()->getParticleEngine()->activate(true, fast);
  return 0;
}

int ScriptFunctions::stopParticles(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs > 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool fast = false;
  if (numArgs >= 1){
    std::string arg = ctx.stack().pop().getString();
    fast = arg == "fast";
  }
  Engine::instance()->getParticleEngine()->activate(false, fast);
  return 0;
}

int ScriptFunctions::function(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string scriptname = ctx.stack().pop().getString();
  TR_DEBUG("Function %s started", scriptname.c_str());
  ExecutionContext* func = Engine::instance()->getInterpreter()->getScript(scriptname);
  int numExecutions = 1;
  if (numArgs > 1){
    StackData d = ctx.stack().pop();
    if (d.getInt() == 0){
      std::string txt = d.getString();
      bool loop = false;
      if (txt == "inf" || txt == "infinitly")
        loop = true;
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

int ScriptFunctions::stopFunction(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string scriptname = ctx.stack().pop().getString();
  TR_DEBUG("Function %s stopped", scriptname.c_str());
  ExecutionContext* stopped = Engine::instance()->getInterpreter()->removeScript(scriptname, false);
  if (stopped == &ctx){
    //script removes itself, skip remaining instructions
    ctx.mPC = 1000000;
  }
  return 0;
}

int ScriptFunctions::speechVolume(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int volume = ctx.stack().pop().getInt();
  SoundEngine::instance()->setSpeechVolume(volume/100.0f);
  return 0;
}

int ScriptFunctions::setLanguage(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string language = ctx.stack().pop().getString();
  Engine::instance()->getInterpreter()->setLanguage(language);
  return 0;
}

int ScriptFunctions::enterText(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs > 9)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int textnum = -1;
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setEnabled(true);
  String varname;
  int maxchars = 100;
  String initalContent;
  if (numArgs >= 1){
    ExecutionContext* text = ctx.stack().pop().getEC();
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
    int x = ctx.stack().pop().getInt();
    if (x != -1)
      txtout->getPos().x = x;
  }
  if (numArgs >= 3){
    int y = ctx.stack().pop().getInt();
    if (y != -1)
      txtout->getPos().y = y;
  }
  if (numArgs >= 4){
    int font = ctx.stack().pop().getInt();
    if (font != -1)
      txtout->setFont(font);
  }
  if (numArgs >= 5){
    maxchars = ctx.stack().pop().getInt();
  }
  if (numArgs >= 6){
    int col = ctx.stack().pop().getInt();
    if (col != -1)
      txtout->getColor().r = col;
  }
  if (numArgs >= 7){
    int col = ctx.stack().pop().getInt();
    if (col != -1)
      txtout->getColor().g = col;
  }
  if (numArgs >= 8){
    int col = ctx.stack().pop().getInt();
    if (col != -1)
      txtout->getColor().b = col;
  }
  if (numArgs >= 9){
    initalContent = ctx.stack().pop().getString();
  }
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(initalContent));
  Engine::instance()->getInterpreter()->cutsceneMode(false);
  Engine::instance()->enterText(varname, maxchars, &ctx);
  ctx.mSuspended = true;
  return 0;
}

int ScriptFunctions::fadeSpeed(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int speed = ctx.stack().pop().getInt();
  if (speed <= 15){
    speed = 1500-(speed-1)*100;
  }
  SoundEngine::instance()->setFadingTime(speed);
  return 0;
}

int ScriptFunctions::setEAX(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string effect = ctx.stack().pop().getString();
  SoundEngine::instance()->setEAXEffect(toLower(effect));
  return 0;
}

int ScriptFunctions::bindText(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int textnum = ctx.stack().pop().getInt();
  String room = ctx.stack().pop().getString();
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

int ScriptFunctions::textOut(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 8)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int textnum = ctx.stack().pop().getInt();
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setEnabled(true);
  if (numArgs >= 2){
    ExecutionContext* text = ctx.stack().pop().getEC();
    Engine::instance()->getInterpreter()->executeImmediately(text, false);
    StackData result = text->stack().pop();
    if (!result.isNumber() || result.getInt() != -1){
      txtout->setText(text);
    }
  }
  if (numArgs >= 3){
    int x = ctx.stack().pop().getInt();
    if (x != -1)
      txtout->getPos().x = x;
  }
  if (numArgs >= 4){
    int y = ctx.stack().pop().getInt();
    if (y != -1)
      txtout->getPos().y = y;
  }
  if (numArgs >= 5){
    int font = ctx.stack().pop().getInt();
    if (font != -1)
      txtout->setFont(font);
  }
  if (numArgs >= 6){
    int col = ctx.stack().pop().getInt();
    if (col != -1)
      txtout->getColor().r = col;
  }
  if (numArgs >= 7){
    int col = ctx.stack().pop().getInt();
    if (col != -1)
      txtout->getColor().g = col;
  }
  if (numArgs >= 8){
    int col = ctx.stack().pop().getInt();
    if (col != -1)
      txtout->getColor().b = col;
  }
  return 0;
}

int ScriptFunctions::textSpeed(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string speed = ctx.stack().pop().getString();
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

int ScriptFunctions::setPos(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 3 || numArgs > 4)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string roomname = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt();
  pos.y = ctx.stack().pop().getInt();
  pos = pos * -Engine::instance()->getWalkGridSize(false);
  bool dontscroll = false;
  if (numArgs > 3)
    dontscroll = ctx.stack().pop().getBool();
  if (numArgs > 4){
    LookDir blenddir = UNSPECIFIED;
    string dir = ctx.stack().pop().getString();
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

int ScriptFunctions::miniCut(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs > 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool hide = true;
  if (numArgs == 1){
    String hstr = ctx.stack().pop().getString();
    if (hstr != "donthide")
      TR_BREAK("donthide expected");
    hide = false;
  }
  Engine::instance()->getInterpreter()->cutsceneMode(hide);
  return 0;
}

int ScriptFunctions::miniCutEnd(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Engine::instance()->getInterpreter()->mGlobalSuspend = false;
  Engine::instance()->getInterpreter()->mHideUI = false;
  return 0;
}

int ScriptFunctions::breakExec(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  //don't know anymore why this was there, but it is bad => see elevator (room 8) functiondemo
  //ctx.resetEvents(true);
  //it was there because of this: pickup keycard and use machine (simple test adventure)
  return 0;
}

int ScriptFunctions::particleView(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  StackData view = ctx.stack().pop();
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

int ScriptFunctions::textHide(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int textnum = ctx.stack().pop().getInt();
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

int ScriptFunctions::stopEffect(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1 || numArgs > 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string effect = ctx.stack().pop().getString();
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
    int idx = ctx.stack().pop().getInt();
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

int ScriptFunctions::startEffect(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string effect = ctx.stack().pop().getString();
  if (effect == "slowmotion"){
    int speed = ctx.stack().pop().getInt();
    bool fade = false;
    if (numArgs > 2){
      std::string fadestr = ctx.stack().pop().getString();
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
    int strength = ctx.stack().pop().getInt();
    bool fade = false;
    if (numArgs > 2){
      std::string fadestr = ctx.stack().pop().getString();
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
    int strength = ctx.stack().pop().getInt();
    disableMainEffect();
    ef->activate(false, strength/50.0f);
  }
  else if (effect == "heat"){
    bool fade = false;
    if (numArgs > 1){
      std::string fadestr = ctx.stack().pop().getString();
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
    int strength = ctx.stack().pop().getInt();
    ef->activate(false, strength/500.0f);
  }
  else if (effect == "lightning"){
    int slot = ctx.stack().pop().getInt();
    double x1 = (double)ctx.stack().pop().getInt();
    double y1 = (double)ctx.stack().pop().getInt();
    double x2 = (double)ctx.stack().pop().getInt();
    double y2 = (double)ctx.stack().pop().getInt();
    x1 /= (double)Engine::instance()->getSettings()->resolution.x;
    y1 /= (double)Engine::instance()->getSettings()->resolution.y;
    x2 /= (double)Engine::instance()->getSettings()->resolution.x;
    y2 /= (double)Engine::instance()->getSettings()->resolution.y;
    Color c;
    c.r = (unsigned char)ctx.stack().pop().getInt();
    c.g = (unsigned char)ctx.stack().pop().getInt();
    c.b = (unsigned char)ctx.stack().pop().getInt();
    int spikes = ctx.stack().pop().getInt();
    double heightfactor = Engine::instance()->getSettings()->resolution.x+Engine::instance()->getSettings()->resolution.y/2.0;
    double height = ctx.stack().pop().getInt()/heightfactor;
    int speed = ctx.stack().pop().getInt();
    speed = (speed+1)*25;
    ef->activate(false, slot, x1, y1, x2, y2, c.r, c.g, c.b, spikes, height, speed);
  }
  else if (effect == "zoom"){
    Vec2i pos;
    pos.x = ctx.stack().pop().getInt();
    pos.y = ctx.stack().pop().getInt();
    double factor = ctx.stack().pop().getFloat()/100.0f;
    int fading = 0;
    if (numArgs > 4){
      std::string fadestr = ctx.stack().pop().getString();
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
      fadein = ctx.stack().pop().getInt();
    int fadeout = 0;
    if (numArgs > 2)
      ctx.stack().pop().getInt();
    ef->activate(true, fadein, fadeout);
  }
  else{
    TR_USE(ADV_ScriptFunc);
    TR_BREAK("Unknown effect %s", effect.c_str());
  }
  return 0;
}

int ScriptFunctions::linkChar(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string character = ctx.stack().pop().getString();
  std::string object = ctx.stack().pop().getString();
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

int ScriptFunctions::stopZooming(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string character = ctx.stack().pop().getString();
  bool stopzooming = ctx.stack().pop().getBool();
  CharacterObject* chr = ctx.getCharacter(character);
  if (!chr)
    TR_BREAK("Unknown character %s", character.c_str());
  chr->setNoZooming(stopzooming, stopzooming && ctx.isSkipping());
  return 0;
}

int ScriptFunctions::unlinkChar(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string character = ctx.stack().pop().getString();
  CharacterObject* chr = ctx.getCharacter(character);
  if (!chr)
    TR_BREAK("Unknown character %s", character.c_str());
  chr->setLinkObject(NULL);
  return 0;
}

int ScriptFunctions::saveString(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String varname = ctx.stack().pop().getString();
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
  //insert new content
  data[varname] = val;
  //save content
  std::ofstream out(file.c_str());
  for (std::map<std::string,StackData>::iterator iter = data.begin(); iter != data.end(); ++iter){
    out << iter->first << " " << iter->second << std::endl;
  }
  out.close();
  return 0;
}

int ScriptFunctions::loadString(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  String varname = ctx.stack().pop().getString();
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

int ScriptFunctions::showMouse(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool show = ctx.stack().pop().getBool();
  Engine::instance()->showMouse(show);
  return 0;
}

int ScriptFunctions::charZoom(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2 || numArgs > 3)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string charname = ctx.stack().pop().getString();
  int size = ctx.stack().pop().getInt();
  bool fade = false;
  if (numArgs >= 3){
    std::string fadestr = ctx.stack().pop().getString();
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
    std::string dummy;
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname, dummy, dummy);
    cso->scale = size/100.0f;
  }
  return 0;
}

int ScriptFunctions::setWalkSound(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string charname = ctx.stack().pop().getString();
  std::string soundname = ctx.stack().pop().getString();
  CharacterObject* chr = ctx.getCharacter(charname);
  if (chr){
    SoundPlayer* plyr = SoundEngine::instance()->getSound(soundname, SoundEngine::PLAYER_CREATE_ALWAYS | SoundEngine::PLAYER_UNMANAGED);
    chr->setWalkSound(plyr);
  }
  else{
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(ctx.resolveCharName(charname));
    if (cso == NULL)
      TR_BREAK("Character %s not found", charname);
    else
      cso->walksound = soundname;
  }
  return 0;
}

int ScriptFunctions::hideAllTexts(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Engine::instance()->getFontRenderer()->enableTextouts(false);
  return 0;
}

int ScriptFunctions::enableMouse(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool enable = ctx.stack().pop().getBool();
  Engine::instance()->enableMouse(enable);
  return 0;
}

int ScriptFunctions::setRectWalkmap(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 6)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string room = ctx.stack().pop().getString();
  Vec2i pos1;
  Vec2i pos2;
  pos1.x = ctx.stack().pop().getInt();
  pos1.y = ctx.stack().pop().getInt();
  pos2.x = ctx.stack().pop().getInt();
  pos2.y = ctx.stack().pop().getInt();
  bool walkable = ctx.stack().pop().getBool();
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

int ScriptFunctions::exchange(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string char1 = ctx.stack().pop().getString();
  std::string char2 = ctx.stack().pop().getString();
  CharacterObject* c1 = ctx.getCharacter(char1);
  CharacterObject* c2 = ctx.getCharacter(char2);
  if (c1 == NULL || c2 == NULL)
    TR_BREAK("Character %s or %s is unknown", char1.c_str(), char2.c_str());
  Inventory* inv = c1->getInventory();
  c1->setInventory(c2->getInventory());
  c2->setInventory(inv);
  return 0;
}

int ScriptFunctions::enableMenu(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool enable = ctx.stack().pop().getBool();
  Engine::instance()->enableMenu(enable);
  return 0;
}

int ScriptFunctions::setTransparency(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int transparency = ctx.stack().pop().getInt();
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

int ScriptFunctions::showAllText(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 0)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Engine::instance()->getFontRenderer()->enableTextouts(true);
  return 0;
}

int ScriptFunctions::instMouse(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int state = ctx.stack().pop().getInt();
  CursorObject* cursor = Engine::instance()->getCursor();
  Engine::instance()->getInterpreter()->setPrevState(cursor, cursor);
  cursor->setState(state+1);
  return 0;
}

int ScriptFunctions::showInventory(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int inventory = ctx.stack().pop().getInt();
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr == NULL){
    TR_WARN("No character focussed");
    return 0;
  }
  Inventory* inv = chr->getInventory();
  inv->setCurrent(inventory);
  return 0;
}

int ScriptFunctions::setObjLight(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 4 || numArgs > 5)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string objname = ctx.stack().pop().getString();
  Color c;
  c.r = (unsigned char)ctx.stack().pop().getInt();
  c.g = (unsigned char)ctx.stack().pop().getInt();
  c.b = (unsigned char)ctx.stack().pop().getInt();
  bool fade = false;
  if (numArgs >= 5){
    std::string fading = ctx.stack().pop().getString();
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

int ScriptFunctions::textAlign(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int num = ctx.stack().pop().getInt();
  std::string alignstr = ctx.stack().pop().getString();
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

int ScriptFunctions::runSpeed(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  float speed = ctx.stack().pop().getInt()/100.0f;
  Engine::instance()->getInterpreter()->setRunSpeed(speed);
  return 0;
}

int ScriptFunctions::runTo(ExecutionContext& ctx, unsigned numArgs){
  return moveTo(ctx, numArgs, Engine::instance()->getInterpreter()->getRunSpeed());
}

int ScriptFunctions::enableFXShape(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  bool enable = ctx.stack().pop().getBool();
  Engine::instance()->enableFXShapes(enable);
  return 0;
}

int ScriptFunctions::scrollSpeed(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  int speed = ctx.stack().pop().getInt();
  bool follow = true;
  if (speed > 100){
    follow = false;
    speed -= 100;
  }
  Engine::instance()->setScrollSpeed(speed*0.0375f, follow);
  return 0;
}

int ScriptFunctions::loadChar(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 1)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  std::string name = ctx.stack().pop().getString();
  std::string dummy;
  CharacterObject* ch = Engine::instance()->loadCharacter(name, Engine::instance()->getCharacterClass(name), false, &ctx);
  ch->realize();
  delete ch;
  return 0;
}

int ScriptFunctions::offTextColor(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs != 3)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  Color col;
  col.r = ctx.stack().pop().getInt();
  col.g = ctx.stack().pop().getInt();
  col.b = ctx.stack().pop().getInt();
  Engine::instance()->getInterpreter()->setOfftextColor(col);
  return 0;
}

int ScriptFunctions::setItem(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  if (numArgs < 2)
    TR_BREAK("Unexpected number of arguments (%i)", numArgs);
  if (numArgs > 2)
    TR_BREAK("Implement me");
  std::string itemname = ctx.stack().pop().getString();
  int state = ctx.stack().pop().getInt();
  Object2D* item = Engine::instance()->getObject(itemname, true);
  if (item)
    item->setState(state);
  Engine::instance()->getInterpreter()->setItemState(itemname, state);
  return 0;
}

int ScriptFunctions::dummy(ExecutionContext& ctx, unsigned numArgs){
  for (unsigned i = 0; i < numArgs; ++i){
    ctx.stack().pop();
  }
  return 0;
}

int ScriptFunctions::isBoolEqual(ExecutionContext& ctx, unsigned numArgs){
  String boolname = ctx.stack().pop().getString();
  bool test = ctx.stack().pop().getBool();
  bool saved = Engine::instance()->getInterpreter()->mBooleans[boolname];
  ctx.stack().push(saved ? 1 : 0);
  ctx.stack().push(test ? 1 : 0);
  return 2;
}

int ScriptFunctions::isObjectInState(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  std::string objname = ctx.stack().pop().getString();
  int checkstate = ctx.stack().pop().getInt();
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj){
    ctx.stack().push(obj->getState());
  }
  else{
    std::string room;
    SaveStateProvider::SaveObject* so = Engine::instance()->getSaver()->findObject(objname, room);
    if (!so)
      TR_BREAK("Unknown object %s", objname.c_str());
    ctx.stack().push(so->state);
  }
  ctx.stack().push(checkstate);
  return 2;
}

int ScriptFunctions::isCommandSet(ExecutionContext& ctx, unsigned numArgs){
  EngineEvent check = EVT_NONE;
  if (numArgs >= 1){
    std::string evtname = ctx.stack().pop().getString();
    check = Engine::instance()->getInterpreter()->getEngineEvent(evtname);
  }
  EngineEvent evt = ctx.getCommandEvent();
  ctx.stack().push(evt);
  ctx.stack().push(check);
  return 2;
}

int ScriptFunctions::isLinkedObject(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  std::string linkname = Engine::instance()->getUseObjectName();
  ctx.stack().push(0);
  ctx.stack().push(_stricmp(linkname.c_str(), objname.c_str()));
  return 2;
}

int ScriptFunctions::isGiveLinkedObject(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  std::string linkname = Engine::instance()->getGiveObjectName();
  ctx.stack().push(0);
  ctx.stack().push(_stricmp(linkname.c_str(), objname.c_str()));
  return 2;
}

int ScriptFunctions::isNumEqual(ExecutionContext& ctx, unsigned numArgs){
  String varname = ctx.stack().pop().getString();
  int test = ctx.stack().pop().getInt();
  int saved = Engine::instance()->getInterpreter()->getVariable(varname).getInt();
  ctx.stack().push(saved);
  ctx.stack().push(test);
  return 2;
}

int ScriptFunctions::isCharFocussed(ExecutionContext& ctx, unsigned numArgs){
  std::string name = ctx.stack().pop().getString();
  ctx.stack().push(0);
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr && _stricmp(chr->getName().c_str(), name.c_str()) == 0)
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

int ScriptFunctions::isCharTriggering(ExecutionContext& ctx, unsigned numArgs){
  std::string name = ctx.stack().pop().getString();
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

int ScriptFunctions::isCharInRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string charname = ctx.stack().pop().getString();
  std::string roomname = ctx.stack().pop().getString();
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

int ScriptFunctions::isCharPossessingItem(ExecutionContext& ctx, unsigned numArgs){
  std::string charname = ctx.stack().pop().getString();
  std::string itemname = ctx.stack().pop().getString();
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

int ScriptFunctions::isKeyDownEqual(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  StackData sd = ctx.stack().pop();
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

int ScriptFunctions::isKeyPressedEqual(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  std::string key = ctx.stack().pop().getString();
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

int ScriptFunctions::isStringEqual(ExecutionContext& ctx, unsigned numArgs){
  String name = ctx.stack().pop().getString();
  String text = ctx.stack().pop().getString();
  String val = Engine::instance()->getInterpreter()->getVariable(name).getString();
  ctx.stack().push(0);
  ctx.stack().push(_stricmp(val.removeAll(' ').c_str(), text.removeAll(' ').c_str()));
  return 2;
}

int ScriptFunctions::isCurrentRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string room = ctx.stack().pop().getString();
  ctx.stack().push(0);
  RoomObject* ro = Engine::instance()->getRoom("");
  if (!ro){
    ctx.stack().push(1);
    return 2;
  }
  ctx.stack().push(_stricmp(room.c_str(), ro->getName().c_str()));
  return 2;
}

int ScriptFunctions::isMouseWheelEqual(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  std::string dir = ctx.stack().pop().getString();
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

int ScriptFunctions::isObjXPosEqual(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  std::string objname = ctx.stack().pop().getString();
  int xpos = ctx.stack().pop().getInt();
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

int ScriptFunctions::isObjYPosEqual(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  std::string objname = ctx.stack().pop().getString();
  int ypos = ctx.stack().pop().getInt();
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

int ScriptFunctions::isItemInState(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  std::string itemname = ctx.stack().pop().getString();
  int checkstate = ctx.stack().pop().getInt();
  Object2D* item = Engine::instance()->getObject(itemname, true);
  if (item){
    ctx.stack().push(item->getState());
  }
  else{
    TR_BREAK("Unknown item %s", itemname.c_str());
    ctx.stack().push(0);
  }
  ctx.stack().push(checkstate);
  return 2;
}
