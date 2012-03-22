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
#include "Screenchange.h"
#include "Particles.h"
#include "Textout.h"
#include "PostProcessing.h"

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
  interpreter->registerFunction("playmusic", playMusic);
  interpreter->registerFunction("stopmusic", stopMusic);
  interpreter->registerFunction("wait", wait);
  interpreter->registerFunction("subroom", subRoom);
  interpreter->registerFunction("return", subRoomReturn);
  interpreter->registerFunction("realreturn", subRoomReturn);
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
  interpreter->registerRelVar("setchar", 2, "char:");
  interpreter->registerRelVar("setchar", 3, "char:"); //TODO 4, 5 , ..., INF
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
  interpreter->registerFunction("textspeed", textSpeed);
  interpreter->registerFunction("setpos", setPos);
  interpreter->registerFunction("minicut", miniCut);
  interpreter->registerFunction("break", breakExec);
  interpreter->registerFunction("particleview", particleView);
  interpreter->registerFunction("texthide", textHide);
  interpreter->registerFunction("playavi", playVideo);
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
  srand((unsigned)time(NULL));
}

int ScriptFunctions::loadRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string room = ctx.stack().pop().getString();
  Engine::instance()->loadRoom(room, false, &ctx);
  return 0;
}

int ScriptFunctions::setFocus(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  Engine::instance()->setFocus(character, &ctx);
  return 0;
}

int ScriptFunctions::showInfo(ExecutionContext& ctx, unsigned numArgs){
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
  std::string character = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt()-1;
  pos.y = ctx.stack().pop().getInt()-1;
  pos = pos * Engine::instance()->getWalkGridSize() + 
    Vec2i(Engine::instance()->getWalkGridSize()/2, Engine::instance()->getWalkGridSize()/2);
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
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (chr){
    //if (ctx.mSkip){
      //chr->setPosition(pos);
      //chr->setLookDir(dir);
      //RoomObject* ro = Engine::instance()->getContainingRoom(chr);
      //if (ro)
      //  chr->setScale(ro->getDepthScale(pos));
    //}
    //else{
      if (!ctx.isSkipping() && hold){
        chr->setSuspensionScript(&ctx);
        ctx.mSuspended = true;
      }
      Engine::instance()->walkTo(chr, pos, dir);
    //}
  }
  return 0;
}

int ScriptFunctions::speech(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  std::string text = ctx.stack().pop().getString();
  std::string sound = "";
  bool hold = true;
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
  CharacterObject* chr = Engine::instance()->getCharacter(character);
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
    int currState = chr->getState();
    chr->addNextState(currState);
    chr->setState(CharacterObject::calculateState(currState, chr->isWalking(), true));
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
  std::string character = ctx.stack().pop().getString();
  if (ctx.mSkip)
    return 0;
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (chr){
    int oldstate = chr->getState();
    LookDir dir = chr->getLookDir();
    if (dir == LEFT || dir == RIGHT)
      chr->setState(16);
    else if (dir == FRONT)
      chr->setState(15);
    chr->getAnimation()->registerAnimationEndHandler(chr);
    chr->addNextState(oldstate);
  }
  return 0;
}

int ScriptFunctions::playSound(ExecutionContext& ctx, unsigned numArgs){
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
    sr->base.lighting = c;
  }
  return 0;
}

int ScriptFunctions::setBool(ExecutionContext& ctx, unsigned numArgs){
  std::string boolname = ctx.stack().pop().getString();
  bool val = ctx.stack().pop().getBool();
  Engine::instance()->getInterpreter()->mBooleans[boolname] = val;
  return 0;
}

int ScriptFunctions::setObj(ExecutionContext& ctx, unsigned numArgs){
  TR_USE(ADV_ScriptFunc);
  std::string objname = ctx.stack().pop().getString();
  //TR_DEBUG("obj: %s", objname.c_str());
  int state = ctx.stack().pop().getInt();
  Object2D* obj = Engine::instance()->getObject(objname, false);
  SaveStateProvider::SaveObject* so = NULL;
  if (obj){
    obj->setState(state);
  }
  else{
    std::string room;
    so = Engine::instance()->getSaver()->findObject(objname, room);
    if (so)
      so->state = state;
    else
      DebugBreak();
  }
  for (unsigned i = 2; i < numArgs; ++i){
    state = ctx.stack().pop().getInt();
    if (obj){
      if (ctx.mSkip)
        obj->setState(state);
      else{
        obj->addNextState(state);
        obj->getAnimation()->registerAnimationEndHandler(obj);
      }
    }
    else{
      so->state = state;
    }
  }
  return 0;
}

int ScriptFunctions::beamTo(ExecutionContext& ctx, unsigned numArgs){
  std::string charname = ctx.stack().pop().getString();
  std::string roomname = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt()-1;
  pos.y = ctx.stack().pop().getInt()-1;
  LookDir dir = UNSPECIFIED;
  if (numArgs >= 5)
    dir = (LookDir)(ctx.stack().pop().getInt()-1);
  CharacterObject* focussedChar = Engine::instance()->getCharacter("self");
  if (focussedChar){
     if(_stricmp(charname.c_str(), focussedChar->getName().c_str()) == 0)
       charname = "self";
  }
  if (charname == "self"){
    //focussed char, therefore change room
    Engine::instance()->loadRoom(roomname, false, &ctx);
    CharacterObject* obj = Engine::instance()->getCharacter(charname);
    if (obj){
      Engine::instance()->getAnimator()->remove(obj);
      int state = CharacterObject::calculateState(obj->getState(), false, false);
      obj->setState(state);
      if (dir != UNSPECIFIED)
        obj->setLookDir(dir);
      RoomObject* ro = Engine::instance()->getRoom(roomname);
      std::string realname;
      Vec2i scrolloffset;
      if (ro == NULL){
        SaveStateProvider::SaveRoom* sro = Engine::instance()->getSaver()->getRoom(roomname);
        realname = sro->base.name;
        scrolloffset = sro->scrolloffset;
      }
      else{
        realname = ro->getName();
        scrolloffset = ro->getScrollOffset();
      }
      obj->setRoom(realname);
      obj->setPosition((pos*Engine::instance()->getWalkGridSize())+Vec2i(Engine::instance()->getWalkGridSize()/2, Engine::instance()->getWalkGridSize()/2), true);
      obj->setScrollOffset(scrolloffset);
      //obj->setScale(ro->getDepthScale(obj->getPosition()));
    }
  }
  else{
    CharacterObject* obj = Engine::instance()->extractCharacter(charname);
    if (!obj){
      obj = Engine::instance()->loadCharacter(charname, Engine::instance()->getCharacterClass(charname), false, &ctx);
      Engine::instance()->getSaver()->getRoom(obj->getRoom());
      Engine::instance()->getSaver()->removeCharacter(obj->getName());
    }
    if (obj){
      Engine::instance()->getAnimator()->remove(obj);
      obj->setPosition(pos*Engine::instance()->getWalkGridSize()+Vec2i(Engine::instance()->getWalkGridSize()/2, Engine::instance()->getWalkGridSize()/2));
      int state = CharacterObject::calculateState(obj->getState(), false, false);
      obj->setState(state);
      if (dir != UNSPECIFIED)
        obj->setLookDir(dir);
      RoomObject* room = Engine::instance()->getRoom(roomname);
      if (room){
        obj->setRoom(room->getName());
        obj->setScale(room->getDepthScale(obj->getPosition()));
        Engine::instance()->getSaver()->removeCharacter(obj->getName());
        room->addObject(obj);
      }
      else{
        obj->setRoom(roomname);
        Engine::instance()->getSaver()->removeCharacter(obj->getName());
        Engine::instance()->getSaver()->getRoom(roomname);
        obj->save();
        delete obj;
      }
    }
  }
  return 0;
}

int ScriptFunctions::addItem(ExecutionContext& ctx, unsigned numArgs){
  std::string charname = ctx.stack().pop().getString();
  std::string itemname = ctx.stack().pop().getString();
  if (itemname == "givelink")
    itemname = Engine::instance()->getGiveObjectName();
  int inventory = 1;
  if (numArgs >= 3)
    inventory = ctx.stack().pop().getInt();
  CharacterObject* chr = Engine::instance()->getCharacter(charname);
  Object2D* item = Engine::instance()->createItem(itemname);
  if (chr){
    chr->getInventory()->addItem(item, inventory);
  }
  else{
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname);
    if (!cso)
      DebugBreak();
    cso->inventory.items[inventory].push_back(item->getName());
    delete item;
  }
  return 0;
}

int ScriptFunctions::cutScene(ExecutionContext& ctx, unsigned numArgs){
  std::string scriptname = ctx.stack().pop().getString();
  Engine::instance()->getInterpreter()->mGlobalSuspend = true;
  ExecutionContext* context = Engine::instance()->loadScript(Script::CUTSCENE, scriptname);
  Engine::instance()->getInterpreter()->executeCutscene(context, false);
  return 0;
}

int ScriptFunctions::taskbar(ExecutionContext& ctx, unsigned numArgs){
  bool state = ctx.stack().pop().getBool();
  Engine::instance()->showTaskbar(state);
  return 0;
}

int ScriptFunctions::follow(ExecutionContext& ctx, unsigned numArgs){
  std::string char1 = ctx.stack().pop().getString();
  std::string char2 = ctx.stack().pop().getString();
  bool hold = true;
  if (numArgs >= 3){
    if (ctx.stack().pop().getString() == "dontwait"){
      hold = false;
    }
  }
  CharacterObject* chr1 = Engine::instance()->getCharacter(char1);
  CharacterObject* chr2 = Engine::instance()->getCharacter(char2);
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
      Engine::instance()->walkTo(chr1, chr2->getPosition(), UNSPECIFIED);
    }
  }
  return 0;
}

int ScriptFunctions::lookTo(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  StackData d = ctx.stack().pop();
  LookDir dir = UNSPECIFIED;
  CharacterObject* chr1 = Engine::instance()->getCharacter(character);
  if (d.getInt() != 0){
    dir = (LookDir)(d.getInt()-1);
    if (chr1)
      chr1->setLookDir(dir);
    else{
      std::string dummy;
      SaveStateProvider::CharSaveObject* chs = Engine::instance()->getSaver()->findCharacter(character, dummy, dummy);
      if (!chs)
        DebugBreak();
      chs->base.state = CharacterObject::calculateState(chs->base.state, false, false);
    }
  }
  else{
    std::string char2 = d.getString();
    CharacterObject* chr2 = Engine::instance()->getCharacter(char2);
    if (chr1 == NULL || chr2 == NULL) //look at another character not possible, they are in different rooms
      return 0;
    Vec2i dir = chr2->getPosition()-chr1->getPosition();
    chr1->setLookDir(dir);
  }
  return 0;
}

int ScriptFunctions::textScene(ExecutionContext& ctx, unsigned numArgs){
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
  std::string charname = ctx.stack().pop().getString();
  std::string itemname = ctx.stack().pop().getString();
  if (itemname == "givelink"){
    itemname = Engine::instance()->getGiveObjectName();
  }
  int inventory = 1;
  if (numArgs >= 3)
    inventory = ctx.stack().pop().getInt();
  CharacterObject* chr = Engine::instance()->getCharacter(charname);
  if (chr){
    chr->getInventory()->removeItem(itemname, inventory, &ctx);
  }
  else
    DebugBreak();
  return 0;
}

int ScriptFunctions::loopSound(ExecutionContext& ctx, unsigned numArgs){
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
  std::string sound = ctx.stack().pop().getString();
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound, SoundEngine::PLAYER_DEFAULT);
  if (sp)
    sp->stop();
  return 0;
}

int ScriptFunctions::playMusic(ExecutionContext& ctx, unsigned numArgs){
  std::string music = ctx.stack().pop().getString();
  std::string pattern;
  if (numArgs >= 2){
    pattern = ctx.stack().pop().getString();
    if (!pattern.empty())
      DebugBreak();
  }
  SoundPlayer* sp = SoundEngine::instance()->getMusic(music);
  if (sp){
    sp->play(true);
  }
  return 0;
}

int ScriptFunctions::stopMusic(ExecutionContext& ctx, unsigned numArgs){
  SoundPlayer* sp = SoundEngine::instance()->getMusic("");
  if (sp){
    sp->fadeVolume(SoundEngine::instance()->getMusicVolume(), 0.0f, SoundEngine::instance()->getFadingTime());
  }
  return 0;
}

int ScriptFunctions::wait(ExecutionContext& ctx, unsigned numArgs){
  float seconds = ctx.stack().pop().getFloat();
  if (ctx.mSkip)
    return 0;
  ctx.mSleepTime = (int)(seconds*1000);
  ctx.mSuspended = true;
  return 0;
}

int ScriptFunctions::subRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string roomname = ctx.stack().pop().getString();
  int fading_time = 0;
  if (numArgs >= 2){
    fading_time = ctx.stack().pop().getInt();
  }
  Engine::instance()->loadRoom(roomname, true, &ctx);
  return 0;
}

int ScriptFunctions::subRoomReturn(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->unloadRoom(NULL, false);
  return 0;
}

int ScriptFunctions::link(ExecutionContext& ctx, unsigned numArgs){
  std::string objectname = ctx.stack().pop().getString();
  Engine::instance()->setUseObject(objectname, ctx.mObjectInfo);
  PcdkScript::mRemoveLinkObject = false;
  return 0;
}

int ScriptFunctions::giveLink(ExecutionContext& ctx, unsigned numArgs){
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
  std::string varname = ctx.stack().pop().getString();
  int val = ctx.stack().pop().getInt();
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(val));
  return 0;
}

int ScriptFunctions::offSpeech(ExecutionContext& ctx, unsigned numArgs){
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt()*Engine::instance()->getWalkGridSize();
  pos.y = ctx.stack().pop().getInt()*Engine::instance()->getWalkGridSize();
  std::string text = ctx.stack().pop().getString();
  std::string sound = "";
  bool hold = true;
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
  RoomObject* room = Engine::instance()->getRoom("");
  if (room){
    pos = pos+room->getScrollOffset();
  }
  std::vector<Vec2i> breakinfo;
  int fontid = Engine::instance()->getFontID();
  if (!Engine::instance()->isTextEnabled())
      text = "";
  Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, fontid, breakinfo);
  str = Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2,pos.y-ext.y, text, 
    DEPTH_GAME_FONT, fontid, breakinfo, Engine::instance()->getSettings()->offspeechcolor, plyr ? 100000 : Engine::instance()->getInterpreter()->getTextSpeed()*text.length());
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
  Engine::instance()->unloadRoom(NULL, true);
  Engine::instance()->forceNotToRenderUnloadingRoom();
  return 0;
}

int ScriptFunctions::restart(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->reset();
  return 0;
}

int ScriptFunctions::gotoLevel(ExecutionContext& ctx, unsigned numArgs){
  int level = ctx.stack().pop().getInt();
  Engine::instance()->getInterpreter()->mNextTSLevel = level;
  return 0;
}

int ScriptFunctions::deactivate(ExecutionContext& ctx, unsigned numArgs){
  std::string scene = ctx.stack().pop().getString();
  int level = ctx.stack().pop().getInt();
  int row = ctx.stack().pop().getInt();
  if (scene == "self")
    scene = Engine::instance()->getInterpreter()->mTSName;
  Engine::instance()->getInterpreter()->mTSActive[scene][level][row] = false;
  return 0;
}

int ScriptFunctions::endScene(ExecutionContext& ctx, unsigned numArgs){
  ctx.mExecuteOnce = true;
  Engine::instance()->clearGui();
  return 0;
}

int ScriptFunctions::instObj(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  int state = ctx.stack().pop().getInt();
  for (unsigned i = 2; i < numArgs; ++i){
    DebugBreak(); //TODO state lists
    state = ctx.stack().pop().getInt();
  }
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj){
    Engine::instance()->getInterpreter()->mPrevState.insert(std::make_pair(obj, obj->getState()));
    obj->setState(state);
  }
  return 0;
}

int ScriptFunctions::command(ExecutionContext& ctx, unsigned numArgs){
  std::string cmd = ctx.stack().pop().getString();
  Engine::instance()->setCommand(cmd, true);
  return 0;
}

int ScriptFunctions::invDown(ExecutionContext& ctx, unsigned numArgs){
  int move = ctx.stack().pop().getInt();
  RoomObject* room = Engine::instance()->getContainingRoom(ctx.mOwner);
  room->getInventory()->addScrollOffset(move);
  return 0;
}

int ScriptFunctions::invUp(ExecutionContext& ctx, unsigned numArgs){
  int move = ctx.stack().pop().getInt();
  RoomObject* room = Engine::instance()->getContainingRoom(ctx.mOwner);
  room->getInventory()->addScrollOffset(-move);
  return 0;
}

int ScriptFunctions::setFont(ExecutionContext& ctx, unsigned numArgs){
  int fontid = ctx.stack().pop().getInt();
  if (!Engine::instance()->getFontRenderer()->loadFont(fontid))
    return 0;
  if (numArgs >= 2){
    CharacterObject* chr = Engine::instance()->getCharacter(ctx.stack().pop().getString());
    if (!chr)
      DebugBreak();
    chr->setFontID(fontid);
  }
  else{
    Engine::instance()->setFontID(fontid);
  }
  return 0;
}

int ScriptFunctions::setScreenchange(ExecutionContext& ctx, unsigned numArgs){
  StackData data = ctx.stack().pop();
  ScreenChange screenchange = SC_DIRECT;
  if (data.getString().size() > 1){
    std::string name = data.getString();
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
    else
      DebugBreak();
  }
  else{
    screenchange = (ScreenChange)data.getInt();
  }
  Engine::instance()->setScreenChange(screenchange);
  return 0;
}

int ScriptFunctions::activate(ExecutionContext& ctx, unsigned numArgs){
  std::string scene = ctx.stack().pop().getString();
  int level = ctx.stack().pop().getInt();
  int row = ctx.stack().pop().getInt();
  if (scene == "self")
    scene = Engine::instance()->getInterpreter()->mTSName;
  Engine::instance()->getInterpreter()->mTSActive[scene][level][row] = true;
  return 0;
}

int ScriptFunctions::saveGame(ExecutionContext& ctx, unsigned numArgs){
  int slot = ctx.stack().pop().getInt();
  Engine::instance()->getSaver()->save(SaveStateProvider::saveSlotToPath(slot));
  return 0;
}

int ScriptFunctions::loadGame(ExecutionContext& ctx, unsigned numArgs){
  int slot = ctx.stack().pop().getInt();
  Engine::instance()->getSaver()->load(SaveStateProvider::saveSlotToPath(slot));
  return 0;
}

int ScriptFunctions::jiggle(ExecutionContext& ctx, unsigned numArgs){
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
    Engine::instance()->getAnimator()->add(room, path, power);
  }
  return 0;
}

int ScriptFunctions::randomNum(ExecutionContext& ctx, unsigned numArgs){
  std::string name = ctx.stack().pop().getString();
  int limit = ctx.stack().pop().getInt();
  int rnd = rand()%limit;
  Engine::instance()->getInterpreter()->setVariable(name, StackData(rnd+1));
  return 0;
}

int ScriptFunctions::setChar(ExecutionContext& ctx, unsigned numArgs){
  std::string chrname = ctx.stack().pop().getString();
  StackData data = ctx.stack().pop();
  if (ctx.mSkip)
    return 0;
  int state = 0;
  if (data.isInt())
    state = data.getInt()+16;
  else{
    std::string statename = data.getString();
    //TODO 
    DebugBreak();
  }
  CharacterObject* obj = Engine::instance()->getCharacter(chrname);
  int oldstate;
  if (obj){
    oldstate = obj->getState();
    obj->setState(state);
  }
  for (unsigned i = 2; i < numArgs; ++i){
    data = ctx.stack().pop();
    if (data.getInt() != 0)
      state = data.getInt()+16;
    else{
      std::string statename = data.getString();
      //TODO
      DebugBreak();
    }
    if (obj){
      obj->addNextState(state);
    }
  }
  if (obj){
    obj->addNextState(oldstate);
    obj->getAnimation()->registerAnimationEndHandler(obj);
  }
  return 0;
}

int ScriptFunctions::setString(ExecutionContext& ctx, unsigned numArgs){
  std::string varname = ctx.stack().pop().getString();
  std::string val = ctx.stack().pop().getString();
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(val));
  return 0;
}

int ScriptFunctions::loadNum(ExecutionContext& ctx, unsigned numArgs){
  std::string varname = ctx.stack().pop().getString();
  int val = 0;
  std::string file = Engine::instance()->getSettings()->savedir+"/num.sav";
  std::ifstream in(file.c_str());
  while(in){
    std::string name;
    int value;
    in >> name >> value;
    if (name == varname){
      val = value;
      break;
    }
  }
  in.close();
  Engine::instance()->getInterpreter()->setVariable(varname, StackData(val));
  return 0;
}

int ScriptFunctions::saveNum(ExecutionContext& ctx, unsigned numArgs){
  std::string varname = ctx.stack().pop().getString();
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
  bool enabled = ctx.stack().pop().getBool();
  Engine::instance()->setTextEnabled(enabled);
  return 0;
}

int ScriptFunctions::realTime(ExecutionContext& ctx, unsigned numArgs){
  bool enabled = ctx.stack().pop().getBool();
  //this is intentionally left blank, consider implementing if engine too slow otherwise
  return 0;
}

int ScriptFunctions::setCharLight(ExecutionContext& ctx, unsigned numArgs){
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
  CharacterObject* chr = Engine::instance()->getCharacter(charname);
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
  std::string groupname = ctx.stack().pop().getString();
  ObjectGroup* grp = new ObjectGroup(groupname);
  for (unsigned i = 2; i < numArgs; ++i){
    std::string object = ctx.stack().pop().getString();
    grp->add(object);
  }
  Engine::instance()->getInterpreter()->mGroups.push_back(grp);
  return 0;
}

int ScriptFunctions::stopSkip(ExecutionContext& ctx, unsigned numArgs){
  ctx.mSkip = false;
  return 0;
}

int ScriptFunctions::playSwf(ExecutionContext& ctx, unsigned numArgs){
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
  std::string room = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt();
  pos.y = ctx.stack().pop().getInt();
  bool walkable = ctx.stack().pop().getBool();
  RoomObject* rm = Engine::instance()->getRoom(room);
  if (rm)
    rm->modifyWalkmap(pos, walkable);
  else
    DebugBreak();
  return 0;
}

int ScriptFunctions::stepTo(ExecutionContext& ctx, unsigned numArgs){
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
  else
    DebugBreak();
  CharacterObject* chr = Engine::instance()->getCharacter(name);
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
    chr->setDepth(pos.y/Engine::instance()->getWalkGridSize());
    //RoomObject* ro = Engine::instance()->getContainingRoom(chr);
    //if (ro)
    //  chr->setScale(ro->getDepthScale(pos));
  }
  else{
    DebugBreak();
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
  std::string name = ctx.stack().pop().getString();
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
    //DebugBreak();
    return 0;
  }
  if (speed == 0 || ctx.mSkip){
    obj->setPosition(newpos);
    return 0;
  }
  std::list<Vec2i> path;
  //path.push_back(obj->getPosition());
  path.push_back(newpos);
  if (speed < 1000)
    speed = 11-speed;
  else
    DebugBreak();
  if (hold){
    ctx.suspend(0, NULL/*new PositionSuspender(obj, newpos)*/);
    obj->setSuspensionScript(&ctx);
  }
  Engine::instance()->getAnimator()->add(obj, path, speed);
  //DebugBreak();
  return 0;
}

int ScriptFunctions::quit(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->quit();
  return 0;
}

int ScriptFunctions::musicVolume(ExecutionContext& ctx, unsigned numArgs){
  int volume = ctx.stack().pop().getInt();
  SoundEngine::instance()->setMusicVolume(volume/100.0f);
  return 0;
}

int ScriptFunctions::setParticles(ExecutionContext& ctx, unsigned numArgs){
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
  bool fast = false;
  if (numArgs >= 1){
    std::string arg = ctx.stack().pop().getString();
    fast = arg == "fast";
  }
  Engine::instance()->getParticleEngine()->activate(true, fast);
  return 0;
}

int ScriptFunctions::stopParticles(ExecutionContext& ctx, unsigned numArgs){
  bool fast = false;
  if (numArgs >= 1){
    std::string arg = ctx.stack().pop().getString();
    fast = arg == "fast";
  }
  Engine::instance()->getParticleEngine()->activate(false, fast);
  return 0;
}

int ScriptFunctions::function(ExecutionContext& ctx, unsigned numArgs){
  std::string scriptname = ctx.stack().pop().getString();
  ExecutionContext* func = Engine::instance()->getInterpreter()->getScript(scriptname);
  int numExecutions = 1;
  if (numArgs > 1){
    StackData d = ctx.stack().pop();
    if (d.getInt() == 0){
      std::string txt = d.getString();
      bool loop = false;
      if (txt == "inf")
        loop = true;
      else
        DebugBreak();
      Engine::instance()->getInterpreter()->execute(func, !loop);
    }
    else{
      numExecutions = atoi(d.getString().c_str());
      DebugBreak();
    }
  }
  else
    DebugBreak();
  return 0;
}

int ScriptFunctions::stopFunction(ExecutionContext& ctx, unsigned numArgs){
  std::string scriptname = ctx.stack().pop().getString();
  Engine::instance()->getInterpreter()->removeScript(scriptname);
  return 0;
}

int ScriptFunctions::speechVolume(ExecutionContext& ctx, unsigned numArgs){
  int volume = ctx.stack().pop().getInt();
  SoundEngine::instance()->setSpeechVolume(volume/100.0f);
  return 0;
}

int ScriptFunctions::setLanguage(ExecutionContext& ctx, unsigned numArgs){
  std::string language = ctx.stack().pop().getString();
  Engine::instance()->getInterpreter()->setLanguage(language);
  return 0;
}

int ScriptFunctions::enterText(ExecutionContext& ctx, unsigned numArgs){
  int textnum = -1;
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setEnabled(true);
  std::string varname;
  int maxchars = 100;
  if (numArgs >= 1){
    ExecutionContext* text = ctx.stack().pop().getEC();
    //get and init the variable
    CCode* code = text->getCode()->get(text->getCode()->numInstructions()-2);
    if (code->getType() != CCode::LOAD)
      DebugBreak();
    else{
      CLOAD* load = (CLOAD*)code;
      varname = load->getVarname();
      Engine::instance()->getInterpreter()->setVariable(varname, StackData(std::string("")));
    }

    Engine::instance()->getInterpreter()->executeImmediately(text, false);
    StackData result = text->stack().pop();
    if (!result.isInt() || result.getInt() != -1){
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
  Engine::instance()->getInterpreter()->mGlobalSuspend = true;
  Engine::instance()->enterText(varname, maxchars, &ctx);
  ctx.mSuspended = true;
  return 0;
}

int ScriptFunctions::fadeSpeed(ExecutionContext& ctx, unsigned numArgs){
  int speed = ctx.stack().pop().getInt();
  if (speed <= 15){
    speed = 1500-(speed-1)*100;
  }
  SoundEngine::instance()->setFadingTime(speed);
  return 0;
}

int ScriptFunctions::setEAX(ExecutionContext& ctx, unsigned numArgs){
  std::string effect = ctx.stack().pop().getString();
  SoundEngine::instance()->setEAXEffect(effect);
  return 0;
}

int ScriptFunctions::bindText(ExecutionContext& ctx, unsigned numArgs){
  int textnum = ctx.stack().pop().getInt();
  std::string room = ctx.stack().pop().getString();
  if (room == "any")
    DebugBreak();
  else if (room == "taskbar")
    DebugBreak();
  else if (room == "menu")
    DebugBreak();
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setRoom(room);
  return 0;
}

int ScriptFunctions::textOut(ExecutionContext& ctx, unsigned numArgs){
  int textnum = ctx.stack().pop().getInt();
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setEnabled(true);
  if (numArgs >= 2){
    ExecutionContext* text = ctx.stack().pop().getEC();
    Engine::instance()->getInterpreter()->executeImmediately(text, false);
    StackData result = text->stack().pop();
    if (!result.isInt() || result.getInt() != -1){
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
  std::string speed = ctx.stack().pop().getString();
  int numspeed = 100;
  if (speed == "slow")
    numspeed = 110;
  else if (speed == "normal")
    numspeed = 100;
  else if (speed == "fast")
    numspeed = 90;
  else
    DebugBreak();
  Engine::instance()->getInterpreter()->setTextSpeed(numspeed);
  return 0;
}

int ScriptFunctions::setPos(ExecutionContext& ctx, unsigned numArgs){
  std::string roomname = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt();
  pos.y = ctx.stack().pop().getInt();
  pos = pos * -Engine::instance()->getWalkGridSize();
  bool dontscroll = ctx.stack().pop().getBool();
  if (numArgs > 4)
    DebugBreak();
  //std::string dir = ctx.stack().pop().getString();
  RoomObject* room = Engine::instance()->getRoom(roomname);
  if (room){
    if (dontscroll || ctx.isSkipping())
      room->setScrollOffset(pos);
    else
      Engine::instance()->getAnimator()->add(room, pos, 0.1f);
  }
  else{
    SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(roomname);
    sr->scrolloffset = pos;
  }
  return 0;
}

int ScriptFunctions::miniCut(ExecutionContext& ctx, unsigned numArgs){
  if (numArgs > 0)
    DebugBreak();
  Engine::instance()->getInterpreter()->mGlobalSuspend = true;
  return 0;
}

int ScriptFunctions::miniCutEnd(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->getInterpreter()->mGlobalSuspend = false;
  return 0;
}

int ScriptFunctions::breakExec(ExecutionContext& ctx, unsigned numArgs){
  //don't know anymore why this was there, but it is bad => see elevator (room 8) functiondemo
  //ctx.resetEvents(true);
  //it was there because of this: pickup keycard and use machine (simple test adventure)
  return 0;
}

int ScriptFunctions::particleView(ExecutionContext& ctx, unsigned numArgs){
  StackData view = ctx.stack().pop();
  int viewnum = 0;
  if (view.isString()){
    std::string val = view.getString();
    if (val == "behindfront")
      viewnum = 2;
    else if (val == "back")
      viewnum = 4;
    else
      DebugBreak();
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
      DebugBreak();
      break;
  }
  Engine::instance()->getParticleEngine()->setDepth(depth);
  return 0;
}

int ScriptFunctions::textHide(ExecutionContext& ctx, unsigned numArgs){
  int textnum = ctx.stack().pop().getInt();
  Textout* txtout = Engine::instance()->getFontRenderer()->getTextout(textnum);
  txtout->setEnabled(false);
  return 0;
}

int ScriptFunctions::stopEffect(ExecutionContext& ctx, unsigned numArgs){
  std::string effect = ctx.stack().pop().getString();
  if (effect == "all"){
    Engine::instance()->getPostProcessor()->stopEffects();
  }
  else{
    DebugBreak();
  }
  return 0;
}

int ScriptFunctions::startEffect(ExecutionContext& ctx, unsigned numArgs){
  std::string effect = ctx.stack().pop().getString();
  PostProcessor::Effect* ef = Engine::instance()->getPostProcessor()->getEffect(effect);
  if (effect == "noise" || effect == "darkbloom"){
    int strength = ctx.stack().pop().getInt();
    bool fade = false;
    if (numArgs > 2){
      std::string fadestr = ctx.stack().pop().getString();
      if (fadestr == "fade")
        fade = true;
    }
    if (effect == "darkbloom")
      ef->activate(fade, strength/50.0f, fade);
    else if (effect == "noise")
      ef->activate(fade, strength/99.0f);
  }
  else if (effect == "hell"){
    ef->activate(false, 1.0, false);
  }
  else
    DebugBreak();
  return 0;
}

int ScriptFunctions::linkChar(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  std::string object = ctx.stack().pop().getString();
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (!chr)
    DebugBreak();
  Object2D* obj = Engine::instance()->getObject(object, false);
  if (!obj)
    DebugBreak();
  chr->setLinkObject(obj);
  return 0;
}

int ScriptFunctions::stopZooming(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  bool stopzooming = ctx.stack().pop().getBool();
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (!chr)
    DebugBreak();
  chr->setNoZooming(stopzooming, stopzooming && ctx.isSkipping());
  return 0;
}

int ScriptFunctions::unlinkChar(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (!chr)
    DebugBreak();
  chr->setLinkObject(NULL);
  return 0;
}

int ScriptFunctions::saveString(ExecutionContext& ctx, unsigned numArgs){
  std::string varname = ctx.stack().pop().getString();
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
  std::string varname = ctx.stack().pop().getString();
  StackData val(std::string("none"));
  std::string file = Engine::instance()->getSettings()->savedir+"/string.sav";
  std::ifstream in(file.c_str());
  while(in){
    std::string name;
    StackData value;
    in >> name >> value;
    if (name == varname){
      val = value;
      break;
    }
  }
  in.close();
  Engine::instance()->getInterpreter()->setVariable(varname, val);
  return 0;
}

int ScriptFunctions::showMouse(ExecutionContext& ctx, unsigned numArgs){
  bool show = ctx.stack().pop().getBool();
  if (!show)
    DebugBreak();
  return 0;
}

int ScriptFunctions::charZoom(ExecutionContext& ctx, unsigned numArgs){
  std::string charname = ctx.stack().pop().getString();
  int size = ctx.stack().pop().getInt();
  if (numArgs >= 3)
    DebugBreak();
  CharacterObject* chr = Engine::instance()->getCharacter(charname);
  chr->setUserScale(size/100.0f);
  return 0;
}

int ScriptFunctions::setWalkSound(ExecutionContext& ctx, unsigned numArgs){
  std::string charname = ctx.stack().pop().getString();
  std::string soundname = ctx.stack().pop().getString();
  CharacterObject* chr = Engine::instance()->getCharacter(charname);
  if (chr){
    SoundPlayer* plyr = SoundEngine::instance()->getSound(soundname, SoundEngine::PLAYER_CREATE_ALWAYS | SoundEngine::PLAYER_UNMANAGED);
    chr->setWalkSound(plyr);
  }
  else{
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname);
    cso->walksound = soundname;
  }
  return 0;
}

int ScriptFunctions::dummy(ExecutionContext& ctx, unsigned numArgs){
  for (unsigned i = 0; i < numArgs; ++i){
    ctx.stack().pop();
  }
  return 0;
}

int ScriptFunctions::isBoolEqual(ExecutionContext& ctx, unsigned numArgs){
  std::string boolname = ctx.stack().pop().getString();
  bool test = ctx.stack().pop().getBool();
  bool saved = Engine::instance()->getInterpreter()->mBooleans[boolname];
  ctx.stack().push(saved ? 1 : 0);
  ctx.stack().push(test ? 1 : 0);
  return 2;
}

int ScriptFunctions::isObjectInState(ExecutionContext& ctx, unsigned numArgs){
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
      DebugBreak();
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
  std::string varname = ctx.stack().pop().getString();
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
  CharacterObject* chr = Engine::instance()->getCharacter(name);
  if (!chr){ //when the character is not found, he or she is not in the current room, so no triggering possible
    ctx.stack().push(3);
    return 2;
  }
  RoomObject* room = Engine::instance()->getRoom("");
  if (room){
    //find the position of the script
    Vec2i pos = room->getScriptPosition(&ctx);
    Vec2i charpos = chr->getPosition();
    charpos.x /= Engine::instance()->getWalkGridSize();
    charpos.y /= Engine::instance()->getWalkGridSize();
    if (pos == charpos)
      ctx.stack().push(0);
    else
      ctx.stack().push(1);
  }
  else{
    ctx.stack().push(2);
    DebugBreak();
  }
  return 2;
}

int ScriptFunctions::isCharInRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string charname = ctx.stack().pop().getString();
  std::string roomname = ctx.stack().pop().getString();
  ctx.stack().push(0);
  std::string room;
  std::string name;
  CharacterObject* chr = Engine::instance()->getCharacter(charname);
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
  CharacterObject* chr = Engine::instance()->getCharacter(charname);
  if (!chr){
    if (charname == "self"){ //no focussed char, so he cannot possess it
      ctx.stack().push(1);
      return 2;
    }
    std::string room;
    std::string realName;
    SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname, room, realName);
    bool found = false;
    for (std::map<int,std::vector<std::string> >::iterator iter = cso->inventory.items.begin(); iter != cso->inventory.items.end(); ++iter){
      for (unsigned i = 0; i < iter->second.size(); ++i){
        if (_stricmp(itemname.c_str(), iter->second[i].c_str()) == 0){
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
    DebugBreak();
  int keycode = iter->second;
  ctx.stack().push(0);
  if (Engine::instance()->isKeyDown(keycode))
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

int ScriptFunctions::isKeyPressedEqual(ExecutionContext& ctx, unsigned numArgs){
  std::string key = ctx.stack().pop().getString();
  std::map<std::string,int>::iterator iter = Engine::instance()->getInterpreter()->mKeymap.find(key);
  if (iter == Engine::instance()->getInterpreter()->mKeymap.end())
    DebugBreak();
  int keycode = iter->second;
  ctx.stack().push(0);
  if (Engine::instance()->isKeyPressed(keycode))
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

int ScriptFunctions::isStringEqual(ExecutionContext& ctx, unsigned numArgs){
  std::string name = ctx.stack().pop().getString();
  std::string text = ctx.stack().pop().getString();
  std::string val = Engine::instance()->getInterpreter()->getVariable(name).getString();
  ctx.stack().push(0);
  ctx.stack().push(_stricmp(val.c_str(), text.c_str()));
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
  std::string dir = ctx.stack().pop().getString();
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
    DebugBreak();
  return 2;
}

int ScriptFunctions::isObjXPosEqual(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  int xpos = ctx.stack().pop().getInt();
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj == NULL)
    DebugBreak();
  ctx.stack().push(obj->getPosition().x);
  ctx.stack().push(xpos);
  return 2;
}
