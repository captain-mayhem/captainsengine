#include "ScriptFunc.h"
#include "Engine.h"
#include "Script.h"
#include "SaveStateProvider.h"
#include "Inventory.h"

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
}

int ScriptFunctions::loadRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string room = ctx.stack().pop().getString();
  Engine::instance()->loadRoom(room, false);
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
    Vec2i pos = Engine::instance()->getCursorPos();
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, 1);
    Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2, pos.y-ext.y, text, DEPTH_GAME_FONT, 1);
  }
  Engine::instance()->setObjectString(text);
  return 0;
}

int ScriptFunctions::walkTo(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt()-1;
  pos.y = ctx.stack().pop().getInt()-1;
  pos = pos * Engine::instance()->getWalkGridSize();
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
    if (hold){
      chr->setSuspensionScript(&ctx);
      ctx.mSuspended = true;
    }
    Engine::instance()->walkTo(chr, pos, dir);
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
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  FontRenderer::String* str = NULL;
  if (chr){
    Vec2i pos = chr->getOverheadPos();
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, chr->getFontID());
    str = &Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2,pos.y-ext.y, text, 
      DEPTH_GAME_FONT, chr->getFontID(), chr->getTextColor(), 3000);
    //stop speaking
    Engine::instance()->getFontRenderer()->removeText(chr);
    str->setSpeaker(chr);
    int currState = chr->getState();
    chr->addNextState(currState);
    chr->setState(CharacterObject::calculateState(currState, chr->isWalking(), true));
    if (sound != "") //TODO
      DebugBreak();
  }
  if (hold && str){
    str->setSuspensionScript(&ctx);
    ctx.mSuspended = true;
  }
  return 0;
}

int ScriptFunctions::pickup(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
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
  //TODO SOUND
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
      obj->addNextState(state);
      obj->getAnimation()->registerAnimationEndHandler(obj);
    }
  }
  return 0;
}

int ScriptFunctions::beamTo(ExecutionContext& ctx, unsigned numArgs){
  std::string charname = ctx.stack().pop().getString();
  std::string roomname = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt();
  pos.y = ctx.stack().pop().getInt();
  LookDir dir = UNSPECIFIED;
  if (numArgs >= 5)
    dir = (LookDir)(ctx.stack().pop().getInt()-1);
  if (charname == "self"){
    //focussed char, therefore change room
    Engine::instance()->loadRoom(roomname, false);
    CharacterObject* obj = Engine::instance()->getCharacter(charname);
    if (obj){
      Engine::instance()->getAnimator()->remove(obj);
      obj->setPosition(pos*Engine::instance()->getWalkGridSize());
      int state = CharacterObject::calculateState(obj->getState(), false, false);
      obj->setState(state);
    }
  }
  else{
    CharacterObject* obj = Engine::instance()->extractCharacter(charname);
    if (obj){
      Engine::instance()->getAnimator()->remove(obj);
      obj->setPosition(pos*Engine::instance()->getWalkGridSize());
      int state = CharacterObject::calculateState(obj->getState(), false, false);
      obj->setState(state);
      Engine::instance()->getSaver()->getRoom(roomname);
      obj->save();
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
  //TODO taskbar
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
    if (hold){
      chr1->setSuspensionScript(&ctx);
      ctx.mSuspended = true;
    }
    Engine::instance()->walkTo(chr1, chr2->getPosition(), UNSPECIFIED);
  }
  return 0;
}

int ScriptFunctions::lookTo(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  StackData d = ctx.stack().pop();
  LookDir dir = UNSPECIFIED;
  CharacterObject* chr1 = Engine::instance()->getCharacter(character);
  if (d.getInt() != 0){
    dir = (LookDir)d.getInt();
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
    chr->getInventory()->removeItem(itemname, inventory);
  }
  return 0;
}

int ScriptFunctions::loopSound(ExecutionContext& ctx, unsigned numArgs){
  std::string sound = ctx.stack().pop().getString();
  int volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().pop().getInt();
  //TODO sound
  return 0;
}

int ScriptFunctions::loopStop(ExecutionContext& ctx, unsigned numArgs){
  std::string sound = ctx.stack().pop().getString();
  //TODO sound
  return 0;
}

int ScriptFunctions::playMusic(ExecutionContext& ctx, unsigned numArgs){
  std::string music = ctx.stack().pop().getString();
  std::string pattern;
  if (numArgs >= 2)
    pattern = ctx.stack().pop().getString();
  //TODO sound
  return 0;
}

int ScriptFunctions::stopMusic(ExecutionContext& ctx, unsigned numArgs){
  //TODO sound
  return 0;
}

int ScriptFunctions::wait(ExecutionContext& ctx, unsigned numArgs){
  float seconds = ctx.stack().pop().getFloat();
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
  Object2D* obj = Engine::instance()->getObject(objectname, true);
  Engine::instance()->setUseObject(obj, ctx.mObjectInfo);
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
  Engine::instance()->setGiveObject(obj, ctx.mObjectInfo);
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
  if (numArgs >= 4){ //TODO SOUND
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
  FontRenderer::String* str = NULL;
  Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, 1);
  str = &Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2,pos.y-ext.y, text, 
    DEPTH_GAME_FONT, 1, Color(), 3000);
  if (sound != "") //TODO
    DebugBreak();
  if (hold && str){
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
  Engine::instance()->getInterpreter()->mTSLevel = level;
  return 0;
}

int ScriptFunctions::deactivate(ExecutionContext& ctx, unsigned numArgs){
  std::string scene = ctx.stack().pop().getString();
  int level = ctx.stack().pop().getInt();
  int row = ctx.stack().pop().getInt();
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

