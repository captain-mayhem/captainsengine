#include "Script.h"

#include <cassert>
#include <string>
#include <wx/msgdlg.h>

#include "Engine.h"
#include "Inventory.h"
#include "ScriptFunc.h"

CodeSegment::~CodeSegment(){
  --(*mRefCount);
  if (*mRefCount > 0)
    return;
  for (unsigned i = 0; i < mCodes.size(); ++i){
    delete mCodes[i];
  }
  delete mRefCount;
}

bool PcdkScript::mRemoveLinkObject = false;

PcdkScript::PcdkScript(AdvDocument* data) : mData(data), mGlobalSuspend(false) {
  ScriptFunctions::registerFunctions(this);
  mBooleans = data->getProjectSettings()->booleans;
  mCutScene = NULL;
  mTSLevel = 1;
  mNextTSLevel = 0;
}

PcdkScript::~PcdkScript(){
  stop();
}

void PcdkScript::stop(){
  mScripts.clear();
  mGlobalSuspend = false;
  delete mCutScene;
  mCutScene = NULL;
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    (*iter)->reset(true);
    delete *iter;
  }
  mScripts.clear();
  mPrevState.clear();
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
          f = ScriptFunctions::dummy;
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
      case ASTNode::TIMERFUNC:{
        TimerNode* timer = static_cast<TimerNode*>(node);
        CodeSegment* seg = new CodeSegment();
        transform(timer->getCommands(), seg, START);
        ExecutionContext* ctx = new ExecutionContext(seg, false, "");
        CTIMER* timercode = new CTIMER((int)(timer->getTime()*1000), ctx);
        codes->addCode(timercode);
        count += 1;
      }
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
mStack(), mPC(0), mHandler(NULL), mSuspended(false), mSleepTime(0), mOwner(NULL), mSkip(false){

}

ExecutionContext::ExecutionContext(const ExecutionContext& ctx){
  mCode = new CodeSegment(*ctx.mCode);
  mIsGameObject = ctx.mIsGameObject;
  mObjectInfo = ctx.mIsGameObject;
  mStack = ctx.mStack;
  mPC = ctx.mPC;
  mEvents = ctx.mEvents;
  mExecuteOnce = ctx.mExecuteOnce;
  mHandler = ctx.mHandler;
  mSuspended = ctx.mSuspended;
  mSleepTime = ctx.mSleepTime;
  mOwner = ctx.mOwner;
  mSkip = ctx.mSkip;
}

ExecutionContext::~ExecutionContext(){
  delete mCode;
}

void PcdkScript::registerFunction(std::string name, ScriptFunc func){
  mFunctions[name] = func;
}

void PcdkScript::update(unsigned time){
  for (std::list<std::pair<Object2D*,int> >::iterator iter = mPrevState.begin(); iter != mPrevState.end(); ++iter){
    iter->first->setState(iter->second);
  }
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ){
    std::set<EngineEvent> events;
    if (mGlobalSuspend){
      //only set loop1 which is always executed
      events = (*iter)->getEvents();
      (*iter)->resetEvents();
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
    mCutScene->setEvent((EngineEvent)(EVT_LEVEL+mTSLevel));
    ExecutionContext* oldcutscene = mCutScene;
    update(mCutScene, time);
    if (mCutScene && !mCutScene->mSuspended && mCutScene->mExecuteOnce && mCutScene == oldcutscene){
      mGlobalSuspend = false;
      Engine::instance()->setCommand(mPrevActiveCommand, false);
      delete mCutScene;
      mCutScene = NULL;
      //reload room if changed through cutscene
      CharacterObject* chr = Engine::instance()->getCharacter("self");
      if (chr)
        Engine::instance()->loadRoom(chr->getRoom(), false);
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
        mCutScene->resetEvent((EngineEvent)(EVT_LEVEL+mTSLevel));
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
  if (!script->mSuspended && script->mCode && script->mPC >= script->mCode->numInstructions()){
    if (script->mHandler)
      script->mHandler(*script);
    script->reset(false);
  }
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
  for (std::list<std::pair<Object2D*, int> >::iterator iter = mPrevState.begin(); iter != mPrevState.end(); ){
    if (iter->first == object){
      iter = mPrevState.erase(iter);
    }
    else
      ++iter;
  }
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
  script->reset(true);
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

void ExecutionContext::setEvents(std::set<EngineEvent>& events){
  mEvents = events;
}

void ExecutionContext::resetEvent(EngineEvent evt){
  mEvents.erase(evt);
}

void ExecutionContext::resetEvents(){
  mEvents.clear();
}

bool ExecutionContext::isEventSet(EngineEvent evt){
  return mEvents.find(evt) != mEvents.end();
}

bool ExecutionContext::isRunning(){
  return mPC > 0;
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
  mSkip = false;
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
    Engine::instance()->setUseObject("", "");
    if (ctx.isEventSet(EVT_GIVE_LINK)){
      ctx.resetEvent(EVT_GIVE_LINK);
      if (chr)
        chr->getScript()->setEvent(EVT_CANT_ALL);
    }
    Engine::instance()->setGiveObject("", "");
  }
}

std::ostream& PcdkScript::save(std::ostream& out){
  out << mBooleans.size() << std::endl;
  for (std::map<std::string,bool>::iterator iter = mBooleans.begin(); iter != mBooleans.end(); ++iter){
    out << iter->first << " " << iter->second << std::endl;
  }
  out << mVariables.size() << std::endl;
  for (std::map<std::string,StackData>::iterator iter = mVariables.begin(); iter != mVariables.end(); ++iter){
    out << iter->first << " " << iter->second << std::endl;
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
