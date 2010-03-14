#include "ScriptFunc.h"

#include <fstream>

#include "Engine.h"
#include "Script.h"
#include "SaveStateProvider.h"
#include "Inventory.h"
#include "Sound.h"

void ScriptFunctions::registerFunctions(PcdkScript* interpreter){
  interpreter->registerFunction("loadroom", loadRoom);
  interpreter->registerFunction("setfocus", setFocus);
  interpreter->registerFunction("showinfo", showInfo);
  interpreter->registerFunction("walkto", walkTo);
  interpreter->registerFunction("speech", speech);
  interpreter->registerFunction("pickup", pickup);
  interpreter->registerFunction("playsound", playSound);
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
  interpreter->registerFunction("link", link);
  interpreter->registerFunction("givelink", giveLink);
  interpreter->registerFunction("if_num", isNumEqual);
  interpreter->registerFunction("setnum", setNum);
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
  srand(time(NULL));
}

int ScriptFunctions::loadRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string room = ctx.stack().pop().getString();
  Engine::instance()->loadRoom(room, false);
  switch(Engine::instance()->getScreenChange()){
    case SC_DIRECT:
      break;
    case SC_FADEOUT:
      DebugBreak();
      break;
    case SC_RECTANGLE:
      DebugBreak();
      break;
    case SC_CIRCLE:{
      CircleScreenChange* csc = new CircleScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, 2000);
      Engine::instance()->getAnimator()->add(csc);
      }
      break;
    case SC_SHUTTERS:
      DebugBreak();
      break;
    case SC_CLOCK:
      DebugBreak();
      break;
    case SC_BLEND:
      break;
    case SC_BLEND_SLOW:
      break;
  }
  return 0;
}

int ScriptFunctions::setFocus(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  Engine::instance()->setFocus(character);
  return 0;
}

int ScriptFunctions::showInfo(ExecutionContext& ctx, unsigned numArgs){
  std::string text = ctx.stack().pop().getString();
  bool show = ctx.stack().pop().getBool();
  if (show){
    std::vector<Vec2i> breakinfo;
    Vec2i pos = Engine::instance()->getCursorPos();
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, 1, breakinfo);
    Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2, pos.y-ext.y, text, DEPTH_GAME_FONT, 1, breakinfo);
  }
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
  if (numArgs >= 4)
    dir = (LookDir)(ctx.stack().pop().getInt()-1);
  bool hold = true;
  if (numArgs >= 5){
    std::string dw = ctx.stack().pop().getString();
    if (dw == "dontwait")
      hold = false;
  }
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (chr){
    if (ctx.mSkip){
      chr->setPosition(pos);
      chr->setLookDir(dir);
    }
    else{
      if (hold){
        chr->setSuspensionScript(&ctx);
        ctx.mSuspended = true;
      }
      Engine::instance()->walkTo(chr, pos, dir);
    }
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
      plyr = SoundEngine::instance()->getSound(sound);
      if (plyr){
        plyr->play(false);
        plyr->setSpeaker(chr);
      }
    }
    std::vector<Vec2i> breakinfo;
    Vec2i pos = chr->getOverheadPos();
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, chr->getFontID(), breakinfo);
    str = &Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2,pos.y-ext.y, text, 
      DEPTH_GAME_FONT, chr->getFontID(), breakinfo, chr->getTextColor(), plyr ? 10000 : 100*text.length());
    //stop speaking
    Engine::instance()->getFontRenderer()->removeText(chr);
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
    chr->addNextState(chr->getState());
    LookDir dir = chr->getLookDir();
    if (dir == LEFT || dir == RIGHT)
      chr->setState(16);
    else if (dir == FRONT)
      chr->setState(15);
    chr->getAnimation()->registerAnimationEndHandler(chr);
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
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound);
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
  if (numArgs >= 5) //TODO
    fade = ctx.stack().pop().getString() == "fade";
  RoomObject* roomobj = Engine::instance()->getRoom(room);
  if (roomobj){
    roomobj->setLightingColor(c);
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
  std::string objname = ctx.stack().pop().getString();
  int state = ctx.stack().pop().getInt();
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj){
    obj->setState(state);
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
  if (charname == "self"){
    //focussed char, therefore change room
    Engine::instance()->loadRoom(roomname, false);
    CharacterObject* obj = Engine::instance()->getCharacter(charname);
    if (obj){
      Engine::instance()->getAnimator()->remove(obj);
      obj->setPosition((pos*Engine::instance()->getWalkGridSize())+Vec2i(Engine::instance()->getWalkGridSize()/2, Engine::instance()->getWalkGridSize()/2));
      int state = CharacterObject::calculateState(obj->getState(), false, false);
      obj->setState(state);
      obj->setRoom(Engine::instance()->getRoom(roomname)->getName());
    }
  }
  else{
    CharacterObject* obj = Engine::instance()->extractCharacter(charname);
    if (!obj){
      obj = Engine::instance()->loadCharacter(charname, Engine::instance()->getCharacterClass(charname), false);
      Engine::instance()->getSaver()->removeCharacter(charname);
    }
    if (obj){
      Engine::instance()->getAnimator()->remove(obj);
      obj->setPosition(pos*Engine::instance()->getWalkGridSize()+Vec2i(Engine::instance()->getWalkGridSize()/2, Engine::instance()->getWalkGridSize()/2));
      int state = CharacterObject::calculateState(obj->getState(), false, false);
      obj->setState(state);
      RoomObject* room = Engine::instance()->getRoom(roomname);
      if (room){
        obj->setRoom(room->getName());
        Engine::instance()->getSaver()->removeCharacter(obj->getName());
        room->addObject(obj);
        //scrolling
        //Vec2i scrollOffset = Engine::instance()->getSettings()->resolution/2-
        //  (obj->getPosition()-Vec2i(0,obj->getSize().y/2));
        //Engine::instance()->getRoom("")->setScrollOffset(scrollOffset); //this function limits the scrolling
        //obj->setScrollOffset(scrollOffset);
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
  if (chr){
    Object2D* item = Engine::instance()->createItem(itemname);
    chr->getInventory()->addItem(item, inventory);
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
    chr1->setLookDir(dir);
  }
  else{
    std::string char2 = d.getString();
    CharacterObject* chr2 = Engine::instance()->getCharacter(char2);
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
    //TODO
    pos.x = ctx.stack().pop().getInt();
    pos.y = ctx.stack().pop().getInt();
    width = ctx.stack().pop().getInt();
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
    //ctx.mExecuteOnce = true;
    //ctx.mOwner = NULL;
    chr->getInventory()->removeItem(itemname, inventory, &ctx);
  }
  return 0;
}

int ScriptFunctions::loopSound(ExecutionContext& ctx, unsigned numArgs){
  std::string sound = ctx.stack().pop().getString();
  int volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().pop().getInt();
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound);
  if (sp){
    sp->setVolume(volume/100.0f);
    sp->play(true);
  }
  return 0;
}

int ScriptFunctions::loopStop(ExecutionContext& ctx, unsigned numArgs){
  std::string sound = ctx.stack().pop().getString();
  SoundPlayer* sp = SoundEngine::instance()->getSound(sound);
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
  if (sp)
    sp->play(true);
  return 0;
}

int ScriptFunctions::stopMusic(ExecutionContext& ctx, unsigned numArgs){
  SoundPlayer* sp = SoundEngine::instance()->getMusic("");
  if (sp)
    sp->stop();
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
  Engine::instance()->loadRoom(roomname, true);
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
  Engine::instance()->getInterpreter()->mVariables[varname] = StackData(val);
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
  std::vector<Vec2i> breakinfo;
  Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, 1, breakinfo);
  str = &Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2,pos.y-ext.y, text, 
    DEPTH_GAME_FONT, 1, breakinfo, Color(), plyr ? 10000 : 100*text.length());
  if (sound != ""){
    plyr = SoundEngine::instance()->getSound(sound);
    if (plyr)
      plyr->play(false);
  }
  if (hold){
    if (plyr)
      plyr->setSuspensionScript(&ctx);
    else if (str)
      str->setSuspensionScript(&ctx);
    ctx.mSuspended = true;
  }
  return 0;
}

int ScriptFunctions::unloadRoom(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->unloadRoom(NULL, true);
  return 0;
}

int ScriptFunctions::restart(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->setFocus("none");
  Engine::instance()->getSaver()->clear();
  Engine::instance()->exitGame();
  Engine::instance()->initGame();
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
  Engine::instance()->getInterpreter()->mCutScene->mExecuteOnce = true;
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
    Engine::instance()->getInterpreter()->mPrevState.push_back(std::make_pair(obj, obj->getState()));
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
  if (numArgs >= 2){
    CharacterObject* chr = Engine::instance()->getCharacter(ctx.stack().pop().getString());
  }
  return 0;
}

int ScriptFunctions::setScreenchange(ExecutionContext& ctx, unsigned numArgs){
  StackData data = ctx.stack().pop();
  int screenchange = 0;
  if (data.getString().size() > 1){
    //TODO
    DebugBreak();
  }
  else{
    screenchange = data.getInt();
    Engine::instance()->setScreenChange((ScreenChange)screenchange);
  }
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
      timecount += time;
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
  Engine::instance()->getInterpreter()->mVariables[name] = StackData(rnd+1);
  return 0;
}

int ScriptFunctions::setChar(ExecutionContext& ctx, unsigned numArgs){
  std::string chrname = ctx.stack().pop().getString();
  StackData data = ctx.stack().pop();
  int state = 0;
  if (data.getInt() != 0)
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
  Engine::instance()->getInterpreter()->mVariables[varname] = StackData(val);
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
  Engine::instance()->getInterpreter()->mVariables[varname] = StackData(val);
  return 0;
}

int ScriptFunctions::saveNum(ExecutionContext& ctx, unsigned numArgs){
  std::string varname = ctx.stack().pop().getString();
  int val = Engine::instance()->getInterpreter()->mVariables[varname].getInt();
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
  //TODO
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
  return 0;
}

int ScriptFunctions::group(ExecutionContext& ctx, unsigned numArgs){
  std::string groupname = ctx.stack().pop().getString();
  for (unsigned i = 2; i < numArgs; ++i){
    std::string object = ctx.stack().pop().getString();
  }
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
  return 0;
}

int ScriptFunctions::stopSwf(ExecutionContext& ctx, unsigned numArgs){
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
  else
    ctx.stack().push(0);
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
  ctx.stack().push(strcmp(linkname.c_str(), objname.c_str()));
  return 2;
}

int ScriptFunctions::isGiveLinkedObject(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  std::string linkname = Engine::instance()->getGiveObjectName();
  ctx.stack().push(0);
  ctx.stack().push(strcmp(linkname.c_str(), objname.c_str()));
  return 2;
}

int ScriptFunctions::isNumEqual(ExecutionContext& ctx, unsigned numArgs){
  std::string varname = ctx.stack().pop().getString();
  int test = ctx.stack().pop().getInt();
  int saved = Engine::instance()->getInterpreter()->mVariables[varname].getInt();
  ctx.stack().push(saved);
  ctx.stack().push(test);
  return 2;
}

int ScriptFunctions::isCharFocussed(ExecutionContext& ctx, unsigned numArgs){
  std::string name = ctx.stack().pop().getString();
  ctx.stack().push(0);
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr && chr->getName() == name)
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

int ScriptFunctions::isCharTriggering(ExecutionContext& ctx, unsigned numArgs){
  std::string name = ctx.stack().pop().getString();
  ctx.stack().push(0);
  CharacterObject* chr = Engine::instance()->getCharacter(name);
  RoomObject* room = Engine::instance()->getRoom("");
  if (chr && room){
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
  CharacterObject* chr = Engine::instance()->getCharacter(charname);
  if (stricmp(chr->getRoom().c_str(), roomname.c_str()) == 0)
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
  if (!chr)
    DebugBreak();
  if (chr->getInventory()->getItem(itemname))
    ctx.stack().push(0);
  else
    ctx.stack().push(1);
  return 2;
}

