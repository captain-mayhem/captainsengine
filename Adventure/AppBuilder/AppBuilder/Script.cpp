#include "Script.h"

#include <cassert>
#include <string>
#include <input/keyboard.h>

#include "Engine.h"
#include "Inventory.h"
#include "ScriptFunc.h"
#include <system/allocation.h>

TR_CHANNEL(ADV_Script)

CodeSegment::~CodeSegment(){
  --(*mRefCount);
  if (*mRefCount > 0)
    return;
  for (unsigned i = 0; i < mCodes.size(); ++i){
    delete mCodes[i];
  }
  delete mRefCount;
  for (unsigned i = 0; i < mEmbeddedContexts.size(); ++i){
    delete mEmbeddedContexts[i];
  }
}

bool PcdkScript::mRemoveLinkObject = false;

std::ostream& operator<<(std::ostream& strm, const ObjectGroup& group){
  strm << group.mName << " " << group.mObjects.size();
  for (unsigned i = 0; i < group.mObjects.size(); ++i){
    strm << " " << group.mObjects[i];
  }
  return strm;
}

std::istream& operator>>(std::istream& strm, ObjectGroup& data){
  int size;
  strm >> data.mName >> size;
  for (int i = 0; i < size; ++i){
    std::string obj;
    strm >> obj;
    data.mObjects.push_back(obj);
  }
  return strm;
}

PcdkScript::PcdkScript(AdvDocument* data) : mData(data), mGlobalSuspend(false) {
  ScriptFunctions::registerFunctions(this);
  mBooleans = data->getProjectSettings()->booleans;
  mCutScene = NULL;
  mTSLevel = 1;
  mNextTSLevel = 0;
  std::string key = "a";
  for (int i = 0; i < 26; ++i){
    key[0] = 'a'+i;
    mKeymap[key] = KEY_A+i;
  }
  for (int i = 0; i < 10; ++i){
    key[0] = '0'+i;
    mKeymap[key] = KEY_0+i;
  }
  key = "F1";
  for (int i = 0; i < 12; ++i){
    key[1] = '1'+i;
    mKeymap[key] = KEY_F1+i;
  }
  mKeymap["up"] = KEY_UP;
  mKeymap["down"] = KEY_DOWN;
  mKeymap["left"] = KEY_LEFT;
  mKeymap["right"] = KEY_RIGHT;
  mKeymap["strg"] = KEY_CTRL;
  mKeymap["alt"] = KEY_ALT;
  mKeymap["space"] = KEY_SPACE;
  mKeymap["enter"] = KEY_RETURN;
  mKeymap["backspace"] = KEY_BACKSPACE;
}

PcdkScript::~PcdkScript(){
  stop();
}

void PcdkScript::stop(){
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    if ((*iter)->mExecuteOnce && (*iter)->mOwner == NULL)
      delete *iter;
  }
  for (std::map<std::string,ExecutionContext*>::iterator iter = mScriptFunctions.begin(); iter != mScriptFunctions.end(); ++iter){
    delete iter->second;
  }
  mScripts.clear();
  mScriptFunctions.clear();
  mGlobalSuspend = false;
  delete mCutScene;
  mCutScene = NULL;
  mPrevState.clear();
  for (unsigned i = 0; i < mGroups.size(); ++i){
    delete mGroups[i];
  }
  mGroups.clear();
}

void myreportError(struct ANTLR3_BASE_RECOGNIZER_struct * recognizer){
  if	(recognizer->state->errorRecovery == ANTLR3_TRUE)
  {
    // Already in error recovery so don't display another error while doing so
    //
    return;
  }

  ANTLR3_PARSER_struct* parser = (ANTLR3_PARSER_struct*)(recognizer->super);
  recognizer->state->error = ANTLR3_FALSE;
  parser->tstream->istream->rewindLast(parser->tstream->istream);
  ppcdkParser pcdk = (ppcdkParser)parser->super;
  pcdkParser_complex_arg_return ret = pcdk->complex_arg(pcdk);

  // Signal we are in error recovery now
  //
  recognizer->state->errorRecovery = ANTLR3_TRUE;

  // Indicate this recognizer had an error while processing.
  //
  recognizer->state->errorCount++;

  // Call the error display routine
  //
  recognizer->displayRecognitionError(recognizer, recognizer->state->tokenNames);
}

void*	
recoverFromMismatchedToken  (pANTLR3_BASE_RECOGNIZER recognizer, ANTLR3_UINT32 ttype, pANTLR3_BITSET_LIST follow){
  recognizer->mismatch(recognizer, ttype, follow);
  return NULL;
}

static std::string internal_stringify(ASTNode* node){
  std::string ret;
  switch(node->getType())
  {
  case ASTNode::RELATIONAL:
    {
      RelationalNode* rel = (RelationalNode*)node;
      if (rel->type() == RelationalNode::REL_EQUAL){
        //do nothing
      }
      else{
        DebugBreak();
      }
      ret += internal_stringify(rel->child());
    }
    break;
  case ASTNode::INTEGER:
    {
      IntNode* num = (IntNode*)node;
      int n = num->value();
      char tmp[64];
      sprintf(tmp, "%i", n);
      ret += tmp;
    }
    break;
  default:
    DebugBreak();
  }
  return ret;
}

ASTNode* stringify(ASTNode* node){
  IdentNode* ret = new IdentNode(internal_stringify(node)+" ");
  return ret;
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
  parser->pParser->super = parser;
  //parser->pParser->rec->reportError = myreportError;
  parser->pParser->rec->recoverFromMismatchedToken = recoverFromMismatchedToken;
  pcdkAST = parser->prog(parser);
  NodeList* p = pcdkAST.nodes;
  if (parser->pParser->rec->state->errorCount > 0){
    TR_USE(ADV_Script)
    CGE::Engine::instance()->messageBox("Error parsing script", "Error");
    TR_ERROR("Error parsing script %s", program.c_str());
#undef free
    parser->free(parser);
    tokStream->free(tokStream);
    lexer->free(lexer);
    input->free(input);
#ifdef _CRTDBG_MAP_ALLOC
#define free _free_dbg
#endif
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
  mUnresolvedBlockEnd = NULL;
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

unsigned PcdkScript::transform(NodeList* program, CodeSegment* codes, TrMode mode, int seperateContext){
  unsigned count = 0;
  if (mode == ARGLIST){
    program->reset(false);
    unsigned argNum = program->size();
    while(program->hasPrev()){
      ASTNode* ast = program->prev();
      if (argNum == seperateContext){
        CodeSegment* newcodes = new CodeSegment();
        transform(ast, newcodes);
        ExecutionContext* ctx = new ExecutionContext(newcodes, false, "");
        codes->addEmbeddedContext(ctx);
        CPUSH* ecp = new CPUSH(ctx);
        codes->addCode(ecp);
        ++count;
      }
      else
        count += transform(ast, codes);
      --argNum;
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
  TR_USE(ADV_Script);
  unsigned count = 0;
  int seperateArgument = -1;
  switch(node->getType()){
      case ASTNode::FUNCCALL:{
        FuncCallNode* fc = static_cast<FuncCallNode*>(node);
        if (fc->getName() == "break"){
          ScriptFunc f = mFunctions["break"];
          codes->addCode(new CCALL(f, 0));
          ++count;
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
        else if (fc->getName() == "textout"){
          seperateArgument = 2;
        }
        else if (fc->getName() == "minicut"){
          mUnresolvedBlockEnd = new CCALL(ScriptFunctions::miniCutEnd, 0);
        }
        ScriptFunc f = mFunctions[fc->getName()];
        if (f == NULL){
          f = ScriptFunctions::dummy;
          DebugBreak();
        }
        count += transform(fc->getArguments(), codes, ARGLIST, seperateArgument);
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
        if (evtcode == EVT_LOOP2){
          CSTATE* state = new CSTATE(CSTATE::IDLE);
          codes->addCode(state);
          ++count;
        }
        int offset = transform(evt->getBlock(), codes, START);
        if (evtcode == EVT_LOOP2){
          CSTATE* state = new CSTATE(CSTATE::NORMAL);
          codes->addCode(state);
          ++offset;
        }
        if (mUnresolvedBlockEnd != NULL){
          codes->addCode(mUnresolvedBlockEnd);
          ++offset;
          mUnresolvedBlockEnd = NULL;
        }
        cevt->setOffset(offset+1);
        count += offset;
      }
      break;
      case ASTNode::CONDITION:{
        CondNode* cond = static_cast<CondNode*>(node);
        ScriptFunc f = mFunctions[cond->getCondFuncName()];
        if(f == NULL){
          f = ScriptFunctions::dummy;
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
      case ASTNode::TIMERFUNC:{
        TimerNode* timer = static_cast<TimerNode*>(node);
        CodeSegment* seg = new CodeSegment();
        transform(timer->getCommands(), seg, START);
        ExecutionContext* ctx = new ExecutionContext(seg, false, "");
        CTIMER* timercode = new CTIMER((int)(timer->getTime()*1000), ctx);
        codes->addCode(timercode);
        count += 1;
      }
      break;
      case ASTNode::ARITHMETIC:{
        ArithmeticNode* arnode = static_cast<ArithmeticNode*>(node);
        count += transform(arnode->left(), codes);
        count += transform(arnode->right(), codes);
        if (arnode->type() == ArithmeticNode::AR_PLUS)
          codes->addCode(new CADD());
        else if (arnode->type() == ArithmeticNode::AR_MINUS)
          codes->addCode(new CSUB());
        else if (arnode->type() == ArithmeticNode::AR_TIMES)
          codes->addCode(new CMUL());
        else if (arnode->type() == ArithmeticNode::AR_DIV)
          codes->addCode(new CDIV());
        else
          DebugBreak();
        count += 1;
      }
      break;
      case ASTNode::CONCATENATION:{
        ConcatenationNode* concatnode = static_cast<ConcatenationNode*>(node);
        count += transform(concatnode->left(), codes);
        count += transform(concatnode->right(), codes);
        codes->addCode(new CCONCAT());
        count += 1;
      }
      break;
      default:
        TR_ERROR("Unhandled AST-Type");
        DebugBreak();
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

void PcdkScript::registerFunction(std::string name, ScriptFunc func){
  mFunctions[name] = func;
}

void PcdkScript::update(unsigned time){
  //check if a character becomes current during that script run
  mACharacterAtScriptStart = Engine::instance()->getCharacter("self") != NULL;
  //for (std::list<std::pair<Object2D*,int> >::iterator iter = mPrevState.begin(); iter != mPrevState.end(); ++iter){
    /*if (iter->first->getScript()->isEventSet(EVT_MOUSEOUT)){
      iter->first->getScript()->resetEvent(EVT_MOUSEOUT);
      iter->first->setState(iter->second);
      iter = mPrevState.erase(iter);
      if (iter != mPrevState.begin())
        --iter;
      if (iter == mPrevState.end())
        break;
    }*/
  //}
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ){
    std::list<EngineEvent> events;
    if (mGlobalSuspend){
      //only set loop1 which is always executed
      events = (*iter)->getEvents();
      (*iter)->resetEvents(false);
      (*iter)->setEvent(EVT_LOOP1);
    }
    update(*iter, time);
    if (mGlobalSuspend){
      (*iter)->setEvents(events);
    }
    if ((*iter)->mExecuteOnce && !(*iter)->mSuspended){
      if ((*iter)->mOwner == NULL)
        delete *iter;
      iter = mScripts.erase(iter);
    }
    else
      ++iter;
  }
  if (mCutScene){
    mTSPos = mTSPosOrig;
    mCutScene->resetEvents(false);
    mCutScene->setEvent((EngineEvent)(EVT_LEVEL+mTSLevel));
    ExecutionContext* oldcutscene = mCutScene;
    mCutScene = NULL;
    update(oldcutscene, time);
    if (mCutScene == NULL)
      mCutScene = oldcutscene;
    else
      delete oldcutscene;
    if (mCutScene && !mCutScene->mSuspended && mCutScene->mExecuteOnce && mCutScene == oldcutscene){
      mGlobalSuspend = false;
      Engine::instance()->setCommand(mPrevActiveCommand, false);
      delete mCutScene;
      mCutScene = NULL;
      //reload room if changed through cutscene
      CharacterObject* chr = Engine::instance()->getCharacter("self");
      if (chr)
        Engine::instance()->loadRoom(chr->getRoom(), false, NULL);
    }
    if (mCutScene && !mCutScene->mSuspended && mCutScene == oldcutscene){
      int count = 0;
      for (std::map<int, bool>::iterator iter = mTSActive[mTSName][mTSLevel].begin(); iter != mTSActive[mTSName][mTSLevel].end(); ++iter){
        if (iter->second)
          ++count;
      }
      //no viable alternatives found (or no textscene)
      if (count == 0 && mTSLevel > 1)
        --mTSLevel;
      //goto another level
      if (mNextTSLevel != 0){
        //mCutScene->resetEvent((EngineEvent)(EVT_LEVEL+mTSLevel));
        mTSLevel = mNextTSLevel;
        mNextTSLevel = 0;
      }
    }
  }
}

void PcdkScript::update(ExecutionContext* ctx, unsigned time){
  if (ctx->mSleepTime > 0 && ctx->mSuspended){
    (ctx->mSleepTime -= time);
    if (ctx->mSleepTime <= 0){
      ctx->mSleepTime = 0;
      ctx->resume();
    }
  }
  executeImmediately(ctx);
}

void PcdkScript::execute(ExecutionContext* script, bool executeOnce){
  if (script == NULL)
    return;
  script->mExecuteOnce = executeOnce;
  //disallow double add
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    if (*iter == script)
      return;
  }
  mScripts.push_back(script);
}

void PcdkScript::executeImmediately(ExecutionContext* script, bool clearStackAfterExec){
  do{
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
    if (!script->mSuspended && script->mCode && script->mPC >= script->mCode->numInstructions()){
      //if (script->mHandler)
      //  script->mHandler(*script);
      clickEndHandler(*script);
      script->reset(false, clearStackAfterExec);
      if (!script->getEvents().empty())
        script->getEvents().pop_front();
    }
  } while (!script->getEvents().empty() && !script->mExecuteOnce);
}

void PcdkScript::executeCutscene(ExecutionContext* script, bool looping){
  mGlobalSuspend = true;
  if (mCutScene != NULL){
    Engine::instance()->setCommand(mPrevActiveCommand, false);
    delete mCutScene;
  }
  mCutScene = script;
  if (script == NULL){
    mGlobalSuspend = false;
    return;
  }
  script->mExecuteOnce = !looping;
  mPrevActiveCommand = Engine::instance()->getActiveCommand();
  if (looping)
    Engine::instance()->setCommand("walkto", false);
  else
    Engine::instance()->setCommand("none", false);
}

void PcdkScript::remove(Object2D* object){
  /*for (std::list<std::pair<Object2D*, int> >::iterator iter = mPrevState.begin(); iter != mPrevState.end(); ){
    if (iter->first == object){
      iter = mPrevState.erase(iter);
    }
    else
      ++iter;
  }*/
  applyPrevState(object);
  remove(object->getScript());
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
  script->reset(true, true);
  delete script;
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
  else if (eventname == "loop1")
    return EVT_LOOP1;
  else if (eventname == "loop2")
    return EVT_LOOP2;
  else if (eventname == "level")
    return EVT_LEVEL;
  else if (eventname == "doubleclick")
    return EVT_DBLCLCK;
  else if (eventname == "mouseout")
    return EVT_MOUSEOUT;
  else if (eventname == "release")
    return EVT_RELEASE;
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

ExecutionContext::ExecutionContext(CodeSegment* segment, bool isGameObject, const std::string& objectinfo) : 
mCode(segment), mIsGameObject(isGameObject), mObjectInfo(objectinfo),
mStack(), mPC(0), mSuspended(false), mSleepTime(0), mOwner(NULL), mSkip(false), mIdle(false), mEventHandled(false){

}

ExecutionContext::ExecutionContext(const ExecutionContext& ctx){
  mCode = new CodeSegment(*ctx.mCode);
  mIsGameObject = ctx.mIsGameObject;
  mObjectInfo = ctx.mIsGameObject;
  mStack = ctx.mStack;
  mPC = ctx.mPC;
  mEvents = ctx.mEvents;
  mExecuteOnce = ctx.mExecuteOnce;
  mSuspended = ctx.mSuspended;
  mSleepTime = ctx.mSleepTime;
  mOwner = ctx.mOwner;
  mSkip = ctx.mSkip;
  mIdle = ctx.mIdle;
  mEventHandled = ctx.mEventHandled;
}

ExecutionContext::~ExecutionContext(){
  delete mCode;
}

void ExecutionContext::setEvent(EngineEvent evt){
  if (!mSuspended)
    mEvents.push_back(evt);
}

void ExecutionContext::setEvents(std::list<EngineEvent>& events){
  mEvents = events;
}

void ExecutionContext::resetEvent(EngineEvent evt){
  if (mEvents.front() != evt)
    DebugBreak();
  mEvents.pop_front();
  mEventHandled = false;
}

void ExecutionContext::resetEvents(bool leaveCurrentUntouched){
  if (leaveCurrentUntouched){
    while(mEvents.size() > 1)
      mEvents.pop_back();
  }
  else{
    mEvents.clear();
    mEventHandled = false;
  }
}

bool ExecutionContext::isEventSet(EngineEvent evt){
  if (mEvents.empty())
    return false;
  return mEvents.front() == evt;
}

bool ExecutionContext::isRunning(){
  return mPC > 0 && !mIdle;
}

EngineEvent ExecutionContext::getCommandEvent(){
  for (std::list<EngineEvent>::iterator iter = mEvents.begin(); iter != mEvents.end(); ++iter){
    EngineEvent curr = *iter;
    if (curr >= EVT_USER_BEGIN && curr <= EVT_USER_MIRROR_END)
      return curr;
  }
  return EVT_NONE;
}

void ExecutionContext::reset(bool clearEvents, bool clearStack){
  if (clearStack)
    mStack.clear();
  if (clearEvents)
    mEvents.clear();
  mPC = 0;
  mSuspended = false;
  mSkip = false;
  mIdle = false;
  mEventHandled = false;
}

void ExecutionContext::resume(){
  mSuspended = false;
  if (Engine::instance()->getInterpreter()->isBlockingScriptRunning() && mIdle) 
    reset(true, true);
}

void PcdkScript::clickEndHandler(ExecutionContext& ctx){
  //if (ctx.isEventSet(EVT_CLICK)){
  //  ctx.resetEvent(EVT_CLICK);
  //}
  EngineEvent evt = ctx.getCommandEvent();
  if (evt != EVT_NONE && ctx.isEventSet(evt) && !ctx.isEventHandled()){
    //an action remained unhandled
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (chr){
      if (evt <= EVT_USER_END){
        //ctx.resetEvent(evt);
        if (ctx.mIsGameObject && Engine::instance()->getInterpreter()->mACharacterAtScriptStart)
          chr->getScript()->setEvent(static_cast<EngineEvent>(evt+EVT_USER_RANGE));
      }
      else{
        //if (Engine::instance()->getInterpreter()->mACharacterAtScriptStart)
          chr->getScript()->setEvent(EVT_CANT_ALL);
      }
    }
  }
  /*if (!mRemoveLinkObject)
    mRemoveLinkObject = true;
  else*/
  if (ctx.mIsGameObject){
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (ctx.isEventSet(EVT_LINK)){
      //ctx.resetEvent(EVT_LINK);
      if (chr  && !ctx.isEventHandled())
        chr->getScript()->setEvent(EVT_CANT_ALL);
      Engine::instance()->setUseObject("", "");
    }
    if (ctx.isEventSet(EVT_GIVE_LINK)){
      //ctx.resetEvent(EVT_GIVE_LINK);
      if (chr  && !ctx.isEventHandled())
        chr->getScript()->setEvent(EVT_CANT_ALL);
      Engine::instance()->setGiveObject("", "");
    }
  }
}

std::ostream& PcdkScript::save(std::ostream& out){
  out << mBooleans.size() << std::endl;
  for (std::map<std::string,bool>::iterator iter = mBooleans.begin(); iter != mBooleans.end(); ++iter){
    out << iter->first << " " << iter->second << std::endl;
  }
  out << mVariables.size() << std::endl;
  for (std::map<std::string,StackData>::iterator iter = mVariables.begin(); iter != mVariables.end(); ++iter){
    std::string name = iter->first;
    if (name.empty())
      name = "none";
    out << name << " " << iter->second << std::endl;
  }
  out << mTSActive.size() << std::endl;
  for (std::map<std::string, std::map<int, std::map<int, bool> > >::iterator iter = mTSActive.begin(); iter != mTSActive.end(); ++iter){
    out << iter->second.size() << " " << iter->first << std::endl;
    for (std::map<int, std::map<int, bool> >::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2){
      out << iter2->second.size() << " " << iter2->first << std::endl;
      for (std::map<int, bool>::iterator iter3 = iter2->second.begin(); iter3 != iter2->second.end(); ++iter3){
        out << iter3->first << " " << iter3->second << std::endl;
      }
    }
  }
  out << mGroups.size() << std::endl;
  for (unsigned i = 0; i < mGroups.size(); ++i){
    out << *mGroups[i];
  }
  out << std::endl;
  out << mScriptFunctions.size();
  for (std::map<std::string, ExecutionContext*>::iterator iter = mScriptFunctions.begin(); iter != mScriptFunctions.end(); ++iter){
    out << " " << iter->first;
  }
  out << std::endl;
  return out;
}

std::istream& PcdkScript::load(std::istream& in){
  mBooleans.clear();
  mVariables.clear();
  mTSActive.clear();
  int num1, num2, num3;
  in >> num1;
  std::string name;
  bool value;
  for (int i = 0; i < num1; ++i){
    in >> name >> value;
    mBooleans[name] = value;
  }
  StackData data;
  in >> num1;
  for (int i = 0; i < num1; ++i){
    in >> name >> data;
    mVariables[name] = data;
  }
  in >> num1;
  int secondval, thirdval;
  for (int i = 0; i < num1; ++i){
    in >> num2; in >> name;
    for (int j = 0; j < num2; ++j){
      in >> num3; in >> secondval;
      for (int k = 0; k < num3; ++k){
        in >> thirdval >> value;
        mTSActive[name][secondval][thirdval] = value;
      }
    }
  }
  in >> num1;
  for (int i = 0; i < num1; ++i){
    mGroups.push_back(new ObjectGroup(""));
    in >> *mGroups.back();
  }
  in >> num1;
  for (int i = 0; i < num1; ++i){
    std::string scriptname;
    in >> scriptname;
    ExecutionContext* ctx = getScript(scriptname);
    execute(ctx, false);
  }
  return in;
}

std::ostream& operator<<(std::ostream& strm, const StackData& data){
  strm << data.mType << " " << (data.mStr.empty() ? "none" : data.mStr) << " " << data.mInt;
  return strm;
}

std::istream& operator>>(std::istream& strm, StackData& data){
  int tmp;
  strm >> tmp >> data.mStr >> data.mInt;
  data.mType = (StackData::Type)tmp;
  if (data.mStr == "none")
    data.mStr = "";
  return strm;
}

enum TimeVal{
  TM_HOUR,
  TM_MINUTE,
  TM_SECOND,
  TM_YEAR,
  TM_MONTH,
  TM_DAY,
};

inline int getTime(TimeVal tv){
#ifdef WIN32
  SYSTEMTIME time;
  GetSystemTime(&time);
  SystemTimeToTzSpecificLocalTime(NULL, &time, &time);
  switch(tv){
    case TM_HOUR:
      return time.wHour;
    case TM_MINUTE:
      return time.wMinute;
    case TM_SECOND:
      return time.wSecond;
    case TM_YEAR:
      return time.wYear;
    case TM_MONTH:
      return time.wMonth;
    case TM_DAY:
      return time.wDay;
  }
#else
  time_t tim;
  time(&tim);
  struct tm* time = localtime(&tim);
  switch(tv){
    case TM_HOUR:
      return time->tm_hour;
    case TM_MINUTE:
      return time->tm_min;
    case TM_SECOND:
      return time->tm_sec;
    case TM_YEAR:
      return time->tm_year+1900;
    case TM_MONTH:
      return time->tm_mon+1;
    case TM_DAY:
      return time->tm_mday;
  }
#endif
  return 0;
}

#ifdef WIN32
#define GETTIME(x) \
    
#endif

StackData PcdkScript::getVariable(const std::string& name){
  if (name == "mousex"){
    return Engine::instance()->getCursorPos().x;
  }
  else if (name == "mousey"){
    return Engine::instance()->getCursorPos().y;
  }
  else if (name == "hour"){
    return getTime(TM_HOUR);
  }
  else if (name == "minute"){
    return getTime(TM_MINUTE);
  }
  else if (name == "second"){
    return getTime(TM_SECOND);
  }
  else if (name == "year"){
    return getTime(TM_YEAR);
  }
  else if (name == "month"){
    return getTime(TM_MONTH);
  }
  else if (name == "day"){
    return getTime(TM_DAY);
  }
  else if (name == "currentroom"){
    DebugBreak();
  }
  else if (name == "roomx"){
    DebugBreak();
  }
  else if (name == "roomy"){
    DebugBreak();
  }
  else if (name == "charx"){
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (!chr)
      return 0;
    return chr->getPosition().x;
  }
  else if (name == "chary"){
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (!chr)
      return 0;
    return chr->getPosition().y;
  }
  else if (name == "charzoom"){
    DebugBreak();
  }
  else if (name.size() > 6 && name.substr(0,6) == "charx:"){
    DebugBreak();
  }
  else if (name.size() > 6 && name.substr(0,6) == "charxy:"){
    DebugBreak();
  }
  else if (name.size() > 9 && name.substr(0,9) == "charzoom:"){
    DebugBreak();
  }
  else if (name.size() > 4 && name.substr(0,4) == "obj:"){
    DebugBreak();
  }
  else if (name.size() > 5 && name.substr(0,5) == "objx:"){
    DebugBreak();
  }
  else if (name.size() > 5 && name.substr(0,5) == "objy:"){
    DebugBreak();
  }
  else if (name == "actiontext"){
    DebugBreak();
  }
  else if (name == "empty"){
    return "";
  }
  std::map<std::string, StackData>::iterator iter = mVariables.find(name);
  if (iter != mVariables.end())
    return iter->second;
  return 0;
}

void PcdkScript::setVariable(const std::string& name, const StackData& value){
  if (name == "mousex"){
    DebugBreak();
  }
  else if (name == "mousey"){
    DebugBreak();
  }
  mVariables[name] = value;
}

void PcdkScript::applyPrevState(Object2D* obj){
  std::map<Object2D*, int>::iterator iter = mPrevState.find(obj);
  if (iter != mPrevState.end()){
    obj->setState(iter->second);
    mPrevState.erase(iter);
  }
}

ExecutionContext* PcdkScript::getScript(const std::string& name){
  std::map<std::string,ExecutionContext*>::iterator iter = mScriptFunctions.find(name);
  if (iter != mScriptFunctions.end())
    return iter->second;
  Script* scr = mData->getScript(Script::CUTSCENE, name);
  ExecutionContext* ctx = parseProgram(scr->text);
  mScriptFunctions[name] = ctx;
  return ctx;
}

void PcdkScript::removeScript(const std::string& name){
  std::map<std::string,ExecutionContext*>::iterator iter = mScriptFunctions.find(name);
  if (iter != mScriptFunctions.end()){
    remove(iter->second);
    mScriptFunctions.erase(iter);
  }
}
