#include "Script.h"

#include <cassert>
#include <string>
#include <wx/msgdlg.h>

#include "Engine.h"
#include "Inventory.h"

CodeSegment::~CodeSegment(){
  for (unsigned i = 0; i < mCodes.size(); ++i){
    delete mCodes[i];
  }
}

bool PcdkScript::mRemoveLinkObject = false;

PcdkScript::PcdkScript(AdvDocument* data) : mData(data), mGlobalSuspend(false) {
  registerFunction("loadroom", loadRoom);
  registerFunction("setfocus", setFocus);
  registerFunction("showinfo", showInfo);
  registerFunction("walkto", walkTo);
  registerFunction("speech", speech);
  registerFunction("pickup", pickup);
  registerFunction("playsound", playSound);
  registerFunction("if_bool", isBoolEqual);
  registerFunction("if_obj", isObjectInState);
  registerFunction("setlight", setLight);
  registerFunction("setbool", setBool);
  registerFunction("if_command", isCommandSet);
  registerFunction("setobj", setObj);
  registerFunction("beamto", beamTo);
  registerFunction("additem", addItem);
  registerFunction("cutscene", cutScene);
  registerFunction("if_link", isLinkedObject);
  registerFunction("taskbar", taskbar);
  registerFunction("follow", follow);
  registerFunction("lookto", lookTo);
  registerFunction("textscene", textScene);
  registerFunction("if_givelink", isGiveLinkedObject);
  registerFunction("delitem", delItem);
  registerFunction("loopsound", loopSound);
  registerFunction("loopstop", loopStop);
  registerFunction("playmusic", playMusic);
  registerFunction("stopmusic", stopMusic);
  registerFunction("wait", wait);
  registerFunction("subroom", subRoom);
  registerFunction("return", subRoomReturn);
  registerFunction("link", link);
  registerFunction("givelink", giveLink);
  registerFunction("if_num", isNumEqual);
  registerFunction("setnum", setNum);
  registerFunction("offspeech", offSpeech);
  registerFunction("unloadroom", unloadRoom);
  registerFunction("restart", restart);
  registerFunction("gotolevel", gotoLevel);
  registerFunction("deactivate", deactivate);
  registerFunction("endscene", endScene);
  registerFunction("instobj", instObj);
  registerFunction("command", command);
  registerFunction("inv_down", invDown);
  registerFunction("inv_up", invUp);
  mBooleans = data->getProjectSettings()->booleans;
  mCutScene = NULL;
  mTSLevel = 1;
}

PcdkScript::~PcdkScript(){
  stop();
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    (*iter)->reset(true);
    delete *iter;
  }
}

void PcdkScript::stop(){
  mScripts.clear();
  mGlobalSuspend = false;
  delete mCutScene;
  mCutScene = NULL;
}

ExecutionContext* PcdkScript::parseProgram(std::string program){
  pANTLR3_INPUT_STREAM input;
  ppcdkLexer lexer;
  pANTLR3_COMMON_TOKEN_STREAM tokStream;
  ppcdkParser parser;
  pcdkParser_prog_return pcdkAST;
  input = antlr3NewAsciiStringInPlaceStream(((pANTLR3_UINT8)program.c_str()), (ANTLR3_UINT32)program.size(), (pANTLR3_UINT8)"Script");
  lexer = pcdkLexerNew(input);
  tokStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
  parser = pcdkParserNew(tokStream);
  pcdkAST = parser->prog(parser);
  NodeList* p = pcdkAST.nodes;
  if (parser->pParser->rec->state->errorCount > 0){
    wxMessageBox("Error parsing script");
    return NULL;
  }
  parser->free(parser);
  tokStream->free(tokStream);
  lexer->free(lexer);
  input->free(input);
  mIsGameObject = false;
  mObjectInfo = "";
  mLastRelation = NULL;
  mUnresolvedLoad = NULL;
  CodeSegment* segment = new CodeSegment;
  transform(p, segment, START);
  for (std::list<std::pair<CBRA*,unsigned> >::iterator iter = mUnresolvedBranches.begin(); iter != mUnresolvedBranches.end(); ++iter){
    unsigned diff = segment->numInstructions() - iter->second;
    iter->first->setOffset(diff+1);
  }
  mUnresolvedBranches.clear();
  delete p;
  if (segment->numInstructions() == 0){
    delete segment;
    return NULL;
  }
  return new ExecutionContext(segment, mIsGameObject, mObjectInfo);
}

unsigned PcdkScript::transform(NodeList* program, CodeSegment* codes, TrMode mode){
  unsigned count = 0;
  if (mode == ARGLIST){
    program->reset(false);
    while(program->hasPrev()){
      ASTNode* ast = program->prev();
      count += transform(ast, codes);
    }
  }
  else{
    program->reset(true);
    while(program->hasNext()){
      ASTNode* ast = program->next();
      count += transform(ast, codes);
    }
  }
  return count;
}

unsigned PcdkScript::transform(ASTNode* node, CodeSegment* codes){
  unsigned count = 0;
  switch(node->getType()){
      case ASTNode::FUNCCALL:{
        FuncCallNode* fc = static_cast<FuncCallNode*>(node);
        if (fc->getName() == "break"){
          CBRA* jmp = new CBRA();
          codes->addCode(jmp);
          ++count;
          mUnresolvedBranches.push_back(std::make_pair(jmp,codes->numInstructions()));
          break;
        }
        else if (fc->getName() == "showinfo"){
          mIsGameObject = true;
          NodeList* nl = fc->getArguments();
          ASTNode* node = nl->next();
          assert(node->getType() == ASTNode::IDENTIFIER);
          IdentNode* id = static_cast<IdentNode*>(node);
          mObjectInfo = id->value();
          nl->reset(true);
        }
        ScriptFunc f = mFunctions[fc->getName()];
        if (f == NULL){
          f = dummy;
          DebugBreak();
        }
        count += transform(fc->getArguments(), codes, ARGLIST);
        codes->addCode(new CCALL(f, fc->getArguments()->size()));
        ++count;
      }
      break;
      case ASTNode::IDENTIFIER:{
        IdentNode* id = static_cast<IdentNode*>(node);
        codes->addCode(new CPUSH(id->value()));
        ++count;
        if (mUnresolvedLoad){
          mUnresolvedLoad->changeVariable(id->value());
          mUnresolvedLoad = NULL;
        }
      }
      break;
      case ASTNode::INTEGER:{
        IntNode* number = static_cast<IntNode*>(node);
        codes->addCode(new CPUSH(number->value()));
        ++count;
      }
      break;
      case ASTNode::REALNUM:{
        RealNode* number = static_cast<RealNode*>(node);
        codes->addCode(new CPUSH(number->value()));
        ++count;
      }
      break;
      case ASTNode::EVENT:{
        EventNode* evt = static_cast<EventNode*>(node);
        EngineEvent evtcode = getEngineEvent(evt->getEvent());
        CBNEEVT* cevt = new CBNEEVT(evtcode);
        codes->addCode(cevt);
        ++count;
        int offset = transform(evt->getBlock(), codes, START);
        cevt->setOffset(offset+1);
        count += offset;
      }
      break;
      case ASTNode::CONDITION:{
        CondNode* cond = static_cast<CondNode*>(node);
        ScriptFunc f = mFunctions[cond->getCondFuncName()];
        if(f == NULL){
          DebugBreak();
        }
        count += transform(cond->getArguments(), codes, ARGLIST);
        codes->addCode(new CCALL(f, cond->getArguments()->size()));
        CBRA* cez = getBranchInstr(mLastRelation, cond->isNegated());
        codes->addCode(cez);
        count += 2;
        int offset = transform(cond->getIfBlock(), codes, START);
        cez->setOffset(offset+1);
        count += offset;
      }
      break;
      case ASTNode::VARIABLE:{
        VariableNode* var = static_cast<VariableNode*>(node);
        codes->addCode(new CLOAD(var->name()));
        ++count;
      }
      break;
      case ASTNode::RELATIONAL:{
        RelationalNode* relnode = static_cast<RelationalNode*>(node);
        if (relnode->type() == RelationalNode::REL_PLUS || relnode->type() == RelationalNode::REL_MINUS){
          //CLOAD var; visit child; CADD/CSUB
          CLOAD* ld = new CLOAD("");
          codes->addCode(ld);
          count += 1;
          count += transform(relnode->child(), codes);
          if (relnode->type() == RelationalNode::REL_PLUS)
            codes->addCode(new CADD());
          else if (relnode->type() == RelationalNode::REL_MINUS)
            codes->addCode(new CSUB());
          else
            DebugBreak();
          count += 1;
          mUnresolvedLoad = ld;
        }
        else{
          mLastRelation = relnode;
          count += transform(relnode->child(), codes);
        }
      }
      break;
      case ASTNode::LEVELDEF:{
        LevelNode* lvl = static_cast<LevelNode*>(node);
        EngineEvent evtcode = (EngineEvent)(EVT_LEVEL+lvl->getLevel());
        CBNEEVT* cevt = new CBNEEVT(evtcode);
        codes->addCode(cevt);
        ++count;
        int offset = transform(lvl->getBlock(), codes, ARGLIST);
        cevt->setOffset(offset+1);
        count += offset;
      }
      break;
      case ASTNode::ROWDEF:{
        RowNode* row = static_cast<RowNode*>(node);
        CBNEROW* crow = new CBNEROW(row->getRow(), row->getText(), row->isVisible());
        codes->addCode(crow);
        ++count;
        int offset = transform(row->getBlock(), codes, START);
        crow->setOffset(offset+1);
        count += offset;
      }
      break;
      default:
        //wxMessageBox("Unhandled AST-Type");
        break;
  }
  return count;
}

CBRA* PcdkScript::getBranchInstr(RelationalNode* relnode, bool negated){
  mLastRelation = NULL;
  if (relnode == NULL){
    if (!negated)
      return new CBNE;
    return new CBE;
  }
  if (!negated){
    if (relnode->type() == RelationalNode::REL_EQUAL)
      return new CBNE;
    else if (relnode->type() == RelationalNode::REL_GREATER)
      return new CBLE;
    else if (relnode->type() == RelationalNode::REL_LESS)
      return new CBGE;
  }
  else{
    if (relnode->type() == RelationalNode::REL_EQUAL)
      return new CBE;
    else if (relnode->type() == RelationalNode::REL_GREATER)
      return new CBGT;
    else if (relnode->type() == RelationalNode::REL_LESS)
      return new CBLT;
  }
  return NULL;
}

ExecutionContext::ExecutionContext(CodeSegment* segment, bool isGameObject, const std::string& objectinfo) : 
mCode(segment), mIsGameObject(isGameObject), mObjectInfo(objectinfo),
mStack(), mPC(0), mHandler(NULL), mSuspended(false), mSleepTime(0){

}

ExecutionContext::~ExecutionContext(){
  delete mCode;
}

void PcdkScript::registerFunction(std::string name, ScriptFunc func){
  mFunctions[name] = func;
}

void PcdkScript::update(unsigned time){
  if (!mGlobalSuspend){
    for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
      update(*iter, time);
      if ((*iter)->mExecuteOnce){
        iter = mScripts.erase(iter);
      }
      if (iter == mScripts.end())
        break;
    }
  }
  if (mCutScene){
    mTSPos = mTSPosOrig;
    mCutScene->setEvent((EngineEvent)(EVT_LEVEL+mTSLevel));
    update(mCutScene, time);
    if (mCutScene && !mCutScene->mSuspended && mCutScene->mExecuteOnce){
      mGlobalSuspend = false;
      Engine::instance()->setCommand(mPrevActiveCommand, false);
      delete mCutScene;
      mCutScene = NULL;
    }
    if (mTSPos == mTSPosOrig && mCutScene && !mCutScene->mSuspended){
      //no viable alternatives found (or no textscene)
      mTSLevel = 1;
    }
  }
}

void PcdkScript::update(ExecutionContext* ctx, unsigned time){
  if (ctx->mSleepTime > 0 && ctx->mSuspended){
    (ctx->mSleepTime -= time);
    if (ctx->mSleepTime <= 0){
      ctx->mSleepTime = 0;
      ctx->mSuspended = false;
    }
  }
  executeImmediately(ctx);
}

void PcdkScript::execute(ExecutionContext* script, bool executeOnce){
  if (script == NULL)
    return;
  script->mExecuteOnce = executeOnce;
  mScripts.push_back(script);
}

void PcdkScript::executeImmediately(ExecutionContext* script){
  if (script->mSuspended)
    return;
  CCode* code = script->mCode->get(script->mPC);
  while(code){
    int result = script->mPC = code->execute(*script, script->mPC);
    if (script->mSuspended)
      break;
    code = script->mCode->get(script->mPC);
  }
  //script ran through
  if (!script->mSuspended && script->mPC >= script->mCode->numInstructions()){
    if (script->mHandler)
      script->mHandler(*script);
    script->reset(false);
  }
}

void PcdkScript::executeCutscene(ExecutionContext* script, bool looping){
  if (script == NULL)
    return;
  mGlobalSuspend = true;
  if (mCutScene != NULL){
    Engine::instance()->setCommand(mPrevActiveCommand, false);
    delete mCutScene;
  }
  script->mExecuteOnce = !looping;
  mCutScene = script;
  mPrevActiveCommand = Engine::instance()->getActiveCommand();
  if (looping)
    Engine::instance()->setCommand("walkto", false);
  else
    Engine::instance()->setCommand("none", false);
}

void PcdkScript::remove(ExecutionContext* script){
  if (script == NULL)
    return;
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    if (*iter == script){
      iter = mScripts.erase(iter);
      break;
    }
    if (iter == mScripts.end())
      break;
  }
  script->reset(true);
  delete script;
}

int PcdkScript::loadRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string room = ctx.stack().pop().getString();
  Engine::instance()->loadRoom(room, false);
  return 0;
}

int PcdkScript::setFocus(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  Engine::instance()->setFocus(character);
  return 0;
}

int PcdkScript::showInfo(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::walkTo(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::speech(ExecutionContext& ctx, unsigned numArgs){
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
    chr->setNextState(currState);
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

int PcdkScript::pickup(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (chr){
    chr->setNextState(chr->getState());
    LookDir dir = chr->getLookDir();
    if (dir == LEFT || dir == RIGHT)
      chr->setState(16);
    else if (dir == FRONT)
      chr->setState(15);
    chr->getAnimation()->registerAnimationEndHandler(chr);
  }
  return 0;
}

int PcdkScript::playSound(ExecutionContext& ctx, unsigned numArgs){
  std::string sound = ctx.stack().pop().getString();
  unsigned volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().pop().getInt();
  //TODO SOUND
  return 0;
}

int PcdkScript::setLight(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::setBool(ExecutionContext& ctx, unsigned numArgs){
  std::string boolname = ctx.stack().pop().getString();
  bool val = ctx.stack().pop().getBool();
  Engine::instance()->getInterpreter()->mBooleans[boolname] = val;
  return 0;
}

int PcdkScript::setObj(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  int state = ctx.stack().pop().getInt();
  for (unsigned i = 2; i < numArgs; ++i){
    DebugBreak(); //TODO state lists
    state = ctx.stack().pop().getInt();
  }
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj){
    obj->setState(state);
  }
  return 0;
}

int PcdkScript::beamTo(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::addItem(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::cutScene(ExecutionContext& ctx, unsigned numArgs){
  std::string scriptname = ctx.stack().pop().getString();
  Engine::instance()->getInterpreter()->mGlobalSuspend = true;
  ExecutionContext* context = Engine::instance()->loadScript(Script::CUTSCENE, scriptname);
  Engine::instance()->getInterpreter()->executeCutscene(context, false);
  return 0;
}

int PcdkScript::taskbar(ExecutionContext& ctx, unsigned numArgs){
  bool state = ctx.stack().pop().getBool();
  //TODO taskbar
  return 0;
}

int PcdkScript::follow(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::lookTo(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::textScene(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::delItem(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::loopSound(ExecutionContext& ctx, unsigned numArgs){
  std::string sound = ctx.stack().pop().getString();
  int volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().pop().getInt();
  //TODO sound
  return 0;
}

int PcdkScript::loopStop(ExecutionContext& ctx, unsigned numArgs){
  std::string sound = ctx.stack().pop().getString();
  //TODO sound
  return 0;
}

int PcdkScript::playMusic(ExecutionContext& ctx, unsigned numArgs){
  std::string music = ctx.stack().pop().getString();
  std::string pattern;
  if (numArgs >= 2)
    pattern = ctx.stack().pop().getString();
  //TODO sound
  return 0;
}

int PcdkScript::stopMusic(ExecutionContext& ctx, unsigned numArgs){
  //TODO sound
  return 0;
}

int PcdkScript::wait(ExecutionContext& ctx, unsigned numArgs){
  float seconds = ctx.stack().pop().getFloat();
  ctx.mSleepTime = (int)(seconds*1000);
  ctx.mSuspended = true;
  return 0;
}

int PcdkScript::subRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string roomname = ctx.stack().pop().getString();
  int fading_time = 0;
  if (numArgs >= 2){
    fading_time = ctx.stack().pop().getInt();
  }
  Engine::instance()->loadRoom(roomname, true);
  return 0;
}

int PcdkScript::subRoomReturn(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->unloadRoom(NULL, false);
  return 0;
}

int PcdkScript::link(ExecutionContext& ctx, unsigned numArgs){
  std::string objectname = ctx.stack().pop().getString();
  Object2D* obj = Engine::instance()->getObject(objectname, true);
  Engine::instance()->setUseObject(obj, ctx.mObjectInfo);
  mRemoveLinkObject = false;
  return 0;
}

int PcdkScript::giveLink(ExecutionContext& ctx, unsigned numArgs){
  Object2D* obj = NULL;
  if (numArgs >= 1){
    std::string objectname = ctx.stack().pop().getString();
    obj = Engine::instance()->getObject(objectname, true);
  }
  Engine::instance()->setGiveObject(obj, ctx.mObjectInfo);
  mRemoveLinkObject = false;
  return 0;
}

int PcdkScript::setNum(ExecutionContext& ctx, unsigned numArgs){
  std::string varname = ctx.stack().pop().getString();
  int val = ctx.stack().pop().getInt();
  Engine::instance()->getInterpreter()->mVariables[varname] = StackData(val);
  return 0;
}

int PcdkScript::offSpeech(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::unloadRoom(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->unloadRoom(NULL, true);
  return 0;
}

int PcdkScript::restart(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->setFocus("none");
  Engine::instance()->getSaver()->clear();
  Engine::instance()->exitGame();
  Engine::instance()->initGame();
  return 0;
}

int PcdkScript::gotoLevel(ExecutionContext& ctx, unsigned numArgs){
  int level = ctx.stack().pop().getInt();
  Engine::instance()->getInterpreter()->mTSLevel = level;
  return 0;
}

int PcdkScript::deactivate(ExecutionContext& ctx, unsigned numArgs){
  std::string scene = ctx.stack().pop().getString();
  int level = ctx.stack().pop().getInt();
  int row = ctx.stack().pop().getInt();
  Engine::instance()->getInterpreter()->mTSActive[scene][level][row] = false;
  return 0;
}

int PcdkScript::endScene(ExecutionContext& ctx, unsigned numArgs){
  Engine::instance()->getInterpreter()->mCutScene->mExecuteOnce = true;
  return 0;
}

int PcdkScript::instObj(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  int state = ctx.stack().pop().getInt();
  for (unsigned i = 2; i < numArgs; ++i){
    DebugBreak(); //TODO state lists
    state = ctx.stack().pop().getInt();
  }
  Object2D* obj = Engine::instance()->getObject(objname, false);
  if (obj){
    obj->setState(state);
  }
  return 0;
}

int PcdkScript::command(ExecutionContext& ctx, unsigned numArgs){
  std::string cmd = ctx.stack().pop().getString();
  Engine::instance()->setCommand(cmd, true);
  return 0;
}

int PcdkScript::invDown(ExecutionContext& ctx, unsigned numArgs){
  return 0;
}

int PcdkScript::invUp(ExecutionContext& ctx, unsigned numArgs){
  return 0;
}


int PcdkScript::dummy(ExecutionContext& ctx, unsigned numArgs){
  for (unsigned i = 0; i < numArgs; ++i){
    ctx.stack().pop();
  }
  return 0;
}

int PcdkScript::isBoolEqual(ExecutionContext& ctx, unsigned numArgs){
  std::string boolname = ctx.stack().pop().getString();
  bool test = ctx.stack().pop().getBool();
  bool saved = Engine::instance()->getInterpreter()->mBooleans[boolname];
  ctx.stack().push(saved ? 1 : 0);
  ctx.stack().push(test ? 1 : 0);
  return 2;
}

int PcdkScript::isObjectInState(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::isCommandSet(ExecutionContext& ctx, unsigned numArgs){
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

int PcdkScript::isLinkedObject(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  std::string linkname = Engine::instance()->getUseObjectName();
  ctx.stack().push(0);
  ctx.stack().push(strcmp(linkname.c_str(), objname.c_str()));
  return 2;
}

int PcdkScript::isGiveLinkedObject(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  std::string linkname = Engine::instance()->getGiveObjectName();
  ctx.stack().push(0);
  ctx.stack().push(strcmp(linkname.c_str(), objname.c_str()));
  return 2;
}

int PcdkScript::isNumEqual(ExecutionContext& ctx, unsigned numArgs){
  std::string varname = ctx.stack().pop().getString();
  int test = ctx.stack().pop().getInt();
  int saved = Engine::instance()->getInterpreter()->mVariables[varname].getInt();
  ctx.stack().push(saved);
  ctx.stack().push(test);
  return 2;
}

EngineEvent PcdkScript::getEngineEvent(const std::string eventname){
  if (eventname == "mouse")
    return EVT_MOUSE;
  else if (eventname == "click")
    return EVT_CLICK;
  else if (eventname == "link")
    return EVT_LINK;
  else if (eventname == "cantall")
    return EVT_CANT_ALL;
  else if (eventname == "givelink")
    return EVT_GIVE_LINK;
  else if (eventname == "enter")
    return EVT_ENTER;
  else if (eventname == "exit")
    return EVT_EXIT;
  else if (eventname == "loop2")
    return EVT_LOOP2;
  else if (eventname == "level")
    return EVT_LEVEL;
  std::map<std::string,unsigned>::iterator iter = mData->getProjectSettings()->commands.find(eventname);
  if (iter != mData->getProjectSettings()->commands.end()){
    return static_cast<EngineEvent>(iter->second);
  }
  if (eventname.substr(0, 4) == "cant"){
    std::map<std::string,unsigned>::iterator iter = mData->getProjectSettings()->commands.find(eventname.substr(4));
    if (iter != mData->getProjectSettings()->commands.end()){
      return static_cast<EngineEvent>(iter->second+EVT_USER_RANGE);
    }
  }
  DebugBreak();
  return EVT_NONE;
}

void ExecutionContext::setEvent(EngineEvent evt){
  if (!mSuspended)
    mEvents.insert(evt);
}

void ExecutionContext::resetEvent(EngineEvent evt){
  mEvents.erase(evt);
}

bool ExecutionContext::isEventSet(EngineEvent evt){
  return mEvents.find(evt) != mEvents.end();
}

EngineEvent ExecutionContext::getCommandEvent(){
  for (std::set<EngineEvent>::iterator iter = mEvents.begin(); iter != mEvents.end(); ++iter){
    EngineEvent curr = *iter;
    if (curr >= EVT_USER_BEGIN && curr <= EVT_USER_MIRROR_END)
      return curr;
  }
  return EVT_NONE;
}

void ExecutionContext::reset(bool clearEvents){
  mStack.clear();
  if (clearEvents)
    mEvents.clear();
  mPC = 0;
  mSuspended = false;
  mHandler = NULL;
}

void PcdkScript::clickEndHandler(ExecutionContext& ctx){
  if (ctx.isEventSet(EVT_CLICK)){
    ctx.resetEvent(EVT_CLICK);
  }
  EngineEvent evt = ctx.getCommandEvent();
  if (evt != EVT_NONE){
    //an action remained unhandled
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (chr){
      if (evt <= EVT_USER_END){
        ctx.resetEvent(evt);
        if (ctx.mIsGameObject)
          chr->getScript()->setEvent(static_cast<EngineEvent>(evt+EVT_USER_RANGE));
      }
    }
  }
  if (!mRemoveLinkObject)
    mRemoveLinkObject = true;
  else if (ctx.mIsGameObject){
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (ctx.isEventSet(EVT_LINK)){
      ctx.resetEvent(EVT_LINK);
      if (chr)
        chr->getScript()->setEvent(EVT_CANT_ALL);
    }
    Engine::instance()->setUseObject(NULL, "");
    if (ctx.isEventSet(EVT_GIVE_LINK)){
      ctx.resetEvent(EVT_GIVE_LINK);
      if (chr)
        chr->getScript()->setEvent(EVT_CANT_ALL);
    }
    Engine::instance()->setGiveObject(NULL, "");
  }
}
