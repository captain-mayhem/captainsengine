#include "Script.h"
extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <cassert>
#include <string>
#include <input/keyboard.h>

#include "Engine.h"
#include "Inventory.h"
#include "ScriptFunc.h"
#include "Sound.h"
#include "Textout.h"
#include "Console.h"
#include "LuaChunk.h"
#include <system/allocation.h>

using namespace adv;

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
    mEmbeddedContexts[i]->unref();
  }
}

bool PcdkScript::mRemoveLinkObject = false;

namespace adv{

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

}

const char* luaRunner =
"function execScript()\n"
"  return _state.script()\n"
"end\n"
"\n"
"function on_newidx(table, ev, eventfunc)\n"
"  local event = getEvent(ev)\n"
"  if _state:eventFired(event) then\n"
"    if event == 172 then --loop2\n"
"      _state:setIdle(true)\n"
"    end\n"
"    _state:eventHandled()\n"
"    eventfunc()\n"
"    if event == 172 then --loop2\n"
"      _state:setIdle(false)\n"
"    end\n"
"  end\n"
"end\n"
"\n"
"function level(lvl, eventfunc)\n"
"  local event = 177 + lvl --level event\n"
"  if _state:eventFired(event) then\n"
"    _state:eventHandled()\n"
"    eventfunc()\n"
"  end\n"
"end\n"
;

PcdkScript::PcdkScript(AdvDocument* data) : mData(data), mGlobalSuspend(false), mTextSpeed(100), mTimeAccu(0), mRunSpeed(1.0f), mScriptMutex(true), mExecMutex(true),
mOffAlign(ALGN_CENTER), mOffRelative(false){
  TR_USE(ADV_Script);
  mL = luaL_newstate();
  luaL_openlibs(mL);
  ScriptFunctions::registerFunctions(this);
  mOfftextColor = data->getProjectSettings()->offspeechcolor;
  mCutScene = NULL;
  mTSLevel = 1;
  mNextTSLevel = 0;
  mHideUI = false;
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
  mKeymap["esc"] = KEY_ESCAPE;

  mLanguage = "origin";

  lua_newtable(mL);
  lua_newtable(mL);
  lua_pushcfunction(mL, getDataVar);
  lua_setfield(mL, -2, "__index");
  //lua_pushcfunction(mL, setSpecialVar);
  //lua_setfield(mL, -2, "__newindex");
  lua_setmetatable(mL, -2);
  lua_setglobal(mL, "data");

  lua_newtable(mL);
  lua_newtable(mL);
  lua_pushcfunction(mL, getSpecialVar);
  lua_setfield(mL, -2, "__index");
  lua_pushcfunction(mL, setSpecialVar);
  lua_setfield(mL, -2, "__newindex");
  lua_setmetatable(mL, -2);
  lua_setglobal(mL, "var");

  lua_newtable(mL);
  for (std::map<String, bool>::iterator iter = data->getProjectSettings()->booleans.begin(); iter != data->getProjectSettings()->booleans.end(); ++iter){
    lua_pushboolean(mL, iter->second);
    lua_setfield(mL, -2, iter->first.c_str());
  }
  lua_setglobal(mL, "bool");

  if (luaL_dostring(mL, luaRunner) != LUA_OK){
    TR_ERROR("Failed to load luaRunner script: %s", lua_tostring(mL, -1));
    lua_pop(mL, 1);
  }
  
  lua_pushcfunction(mL, eventStrToEventId);
  lua_setglobal(mL, "getEvent");

  lua_pushglobaltable(mL);
  lua_newtable(mL);
  lua_pushcfunction(mL, varToString);
  lua_setfield(mL, -2, "__index");
  lua_setmetatable(mL, -2);
  lua_pop(mL, 1);
}

PcdkScript::~PcdkScript(){
  stop();
  lua_close(mL);
}

void PcdkScript::stop(){
  mScriptMutex.lock();
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    if ((*iter)->mExecuteOnce && (*iter)->mOwner == NULL)
      (*iter)->unref();
  }
  for (std::map<std::string,ExecutionContext*>::iterator iter = mScriptFunctions.begin(); iter != mScriptFunctions.end(); ++iter){
    iter->second->unref();
  }
  mScripts.clear();
  mTimers.clear(); //no need to destroy them, destroyed above
  mScriptFunctions.clear();
  mGlobalSuspend = false;
  mHideUI = false;
  mCutScene->unref();
  mCutScene = NULL;
  mPrevState.clear();
  for (unsigned i = 0; i < mGroups.size(); ++i){
    delete mGroups[i];
  }
  mGroups.clear();
  mLanguage = "origin";
  mRunSpeed = 1.0f;
  mOfftextColor = mData->getProjectSettings()->offspeechcolor;
  mItemStates.clear();
  mOffAlign = ALGN_CENTER;
  mOffRelative = false;
  mScriptMutex.unlock();
}

void reportParseError(struct ANTLR3_BASE_RECOGNIZER_struct * recognizer){
  TR_USE(ADV_Script);

  if	(recognizer->state->errorRecovery == ANTLR3_TRUE)
  {
    // Already in error recovery so don't display another error while doing so
    //
    return;
  }

  //ANTLR3_PARSER_struct* parser = (ANTLR3_PARSER_struct*)(recognizer->super);
  //recognizer->state->error = ANTLR3_FALSE;
  //parser->tstream->istream->rewindLast(parser->tstream->istream);
  //ppcdkParser pcdk = (ppcdkParser)parser->super;
  //pcdkParser_complex_arg_return ret = pcdk->complex_arg(pcdk);

  // Signal we are in error recovery now
  //
  recognizer->state->errorRecovery = ANTLR3_TRUE;

  // Indicate this recognizer had an error while processing.
  //
  recognizer->state->errorCount++;

  //pANTLR3_COMMON_TOKEN token = recognizer->state->exception->token;
  TR_ERROR("Parse error at line %i, position %i", recognizer->state->exception->line, recognizer->state->exception->charPositionInLine);

  // Call the error display routine
  //
  recognizer->displayRecognitionError(recognizer, recognizer->state->tokenNames);
}

void*	
recoverFromMismatchedToken  (pANTLR3_BASE_RECOGNIZER recognizer, ANTLR3_UINT32 ttype, pANTLR3_BITSET_LIST follow){
  recognizer->mismatch(recognizer, ttype, follow);
  return NULL;
}

std::string PcdkScript::internal_stringify(ASTNode* node){
  TR_USE(ADV_Script);
  std::string ret;
  switch(node->getType())
  {
  case ASTNode::IDENTIFIER:
    {
      IdentNode* id = static_cast<IdentNode*>(node);
      ret += id->value();
    }
    break;
  case ASTNode::RELATIONAL:
    {
      RelationalNode* rel = (RelationalNode*)node;
      if (rel->type() == RelationalNode::REL_EQUAL){
        //do nothing
      }
      else{
        TR_BREAK("Unhandled type");
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
  case ASTNode::VARIABLE:
    {
      VariableNode* var = static_cast<VariableNode*>(node);
      if (var->getChild() != NULL)
        TR_BREAK("Variable with child unhandled");
      Engine::instance()->getInterpreter()->getExecMutex().lock();
      StackData s = getVariable(var->name().c_str());
      Engine::instance()->getInterpreter()->getExecMutex().unlock();
      ret += s.getString();
    }
    break;
  case ASTNode::CONCATENATION:
    {
      ConcatenationNode* concat = static_cast<ConcatenationNode*>(node);
      ret += internal_stringify(concat->left());
      ret += " ";
      ret += internal_stringify(concat->right());
    }
    break;
  default:
    TR_BREAK("Unknown node type");
  }
  return ret;
}

ASTNode* stringify(ASTNode* node){
  IdentNode* ret = new IdentNode(Engine::instance()->getInterpreter()->internal_stringify(node)+" ");
  return ret;
}

ExecutionContext* PcdkScript::parseProgram(const std::string& program, ScriptLang type){
  mLoop1 = NULL;
  if (type == DEFAULT_SCRIPT)
    type = mData->getProjectSettings()->script_lang;
  if (type == PCDK_SCRIPT)
    return parseProgramPCDK(program);
  else if (type == LUA_SCRIPT)
    return parseProgramLUA(program);
  return NULL;
}

ExecutionContext* PcdkScript::parseProgramPCDK(const std::string& program){
  if (program.empty())
    return NULL;
  mMutex.lock();
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
  parser->pParser->rec->reportError = reportParseError;
  parser->pParser->rec->recoverFromMismatchedToken = recoverFromMismatchedToken;
  pcdkAST = parser->prog(parser);
  NodeList* p = pcdkAST.nodes;
  if (parser->pParser->rec->state->errorCount > 0){
    TR_USE(ADV_Script)
    //CGE::Engine::instance()->messageBox("Error parsing script", "Error");
    TR_ERROR("Error parsing script %s", program.c_str());
#undef free
    parser->free(parser);
    tokStream->free(tokStream);
    lexer->free(lexer);
    input->free(input);
    delete p;
    mMutex.unlock();
    return NULL;
  }
  parser->free(parser);
  tokStream->free(tokStream);
  lexer->free(lexer);
  input->free(input);
#ifdef _CRTDBG_MAP_ALLOC
#define free _free_dbg
#endif
  //sanitize AST
  if (p->first() && p->first()->getType() == ASTNode::EVENT){
    if (p->removeNotOfType(ASTNode::EVENT)){
      TR_USE(ADV_Script);
      TR_ERROR("Skipping non-event top-level statements in event script");
    }
  }
  //transform AST to CIL
  mIsGameObject = false;
  mObjectInfo = "";
  mLastRelation = NULL;
  mFirstArg = NULL;
  mUnresolvedBlockEnd = NULL;
  mInRow = false;
  CodeSegment* segment = new CodeSegment;
  segment->addCode(new CSTATE(CSTATE::NORMAL));
  transform(p, segment, START);
  for (std::list<std::pair<CBRA*,unsigned> >::iterator iter = mUnresolvedBranches.begin(); iter != mUnresolvedBranches.end(); ++iter){
    unsigned diff = segment->numInstructions() - iter->second;
    iter->first->setOffset(diff+1);
  }
  mUnresolvedBranches.clear();
  delete p;
  bool isGameObject = mIsGameObject;
  std::string objectInfo = mObjectInfo;
  mEvents.clear();
  mMutex.unlock();
  if (segment->numInstructions() <= 1 && mLoop1 == NULL){
    delete segment;
    return NULL;
  }
  /*LuaChunk ch;
  String ret = ch.serialize();
  FILE* f = fopen("C:/tmp/cmp.luac", "wb");
  fwrite(ret.c_str(), ret.size(), 1, f);
  fclose(f);*/
  ExecutionContext* ret = new ExecutionContext(segment, isGameObject, objectInfo);
  ret->setLoop1(mLoop1);
  return ret;
}

ExecutionContext* PcdkScript::parseProgramLUA(const std::string& program){
  TR_USE(ADV_Console);
  ExecutionContext* ret = new ExecutionContext(NULL, program.find("showinfo") != program.npos, "");
  lua_State* L = ret->getState();
  if (luaL_loadstring(L, program.c_str()) != LUA_OK){
    TR_ERROR("%s", lua_tostring(L, -1));
    lua_pop(L, 2);
    delete ret;
    return NULL;
  }
  lua_pushvalue(L, -1); //duplicate function for possible loop1 context
  initLuaContext(ret);
  if (program.find("loop1") != program.npos){
    ExecutionContext* loop1 = new ExecutionContext(L);
    ret->setLoop1(loop1);
  }
  else
    lua_pop(L, 1);
  return ret;
}

unsigned PcdkScript::transform(NodeList* program, CodeSegment* codes, TrMode mode, int seperateContext){
  unsigned count = 0;
  if (mode == ARGLIST){
    program->reset(true);
    mFirstArg = program->first();
    mCurrArg = 1;
    while (program->hasNext()){
      ASTNode* ast = program->next();
      if (mCurrArg == seperateContext){
        CodeSegment* newcodes = new CodeSegment();
        unsigned oldargs = mCurrArg;
        transform(ast, newcodes);
        mCurrArg = oldargs;
        ExecutionContext* ctx = new ExecutionContext(newcodes, false, "");
        codes->addEmbeddedContext(ctx);
        CPUSH* ecp = new CPUSH(ctx);
        codes->addCode(ecp);
        ++count;
      }
      else{
        unsigned oldargs = mCurrArg;
        count += transform(ast, codes);
        mCurrArg = oldargs;
      }
      ++mCurrArg;
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
        mCurrFunc = fc->getName();
        if (fc->getName() == "break"){
          //break within minicut, add minicut end before break
          if (mUnresolvedBlockEnd != NULL){
            codes->addCode(new CFCPUSH(ScriptFunctions::miniCutEnd, "minicutend"));
            codes->addCode(new CCALL(0, 0));
            count += 2;
          }
          ScriptFunc f = mFunctions["break"];
          codes->addCode(new CFCPUSH(f, "break"));
          codes->addCode(new CCALL(0, 0));
          count += 2;
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
          mObjectInfo = internal_stringify(node);
          nl->reset(true);
        }
        else if (fc->getName() == "minicut"){
          if (mUnresolvedBlockEnd == NULL){
            mUnresolvedBlockEnd = new CFCPUSH(ScriptFunctions::miniCutEnd, "minicut");
          }
        }
        ScriptFunc f = mFunctions[fc->getName()];
        if (f == NULL){
          f = ScriptFunctions::dummy;
          TR_BREAK("script function %s does not exist", fc->getName().c_str());
        }
        std::map<std::string,int>::iterator sepiter = mArgEC.find(fc->getName());
        if (sepiter != mArgEC.end())
          seperateArgument = sepiter->second;
        codes->addCode(new CFCPUSH(f, fc->getName()));
        ++count;
        count += transform(fc->getArguments(), codes, ARGLIST, seperateArgument);
        codes->addCode(new CCALL(fc->getArguments()->size(), 0));
        ++count;
      }
      break;
      case ASTNode::IDENTIFIER:{
        IdentNode* id = static_cast<IdentNode*>(node);
        codes->addCode(new CPUSH(id->value().c_str()));
        ++count;
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
        unsigned numEvents = evt->getNumEvents();
        EngineEvent evtcode = EVT_NONE;
        for (unsigned i = 0; i < numEvents; ++i){
          evtcode = getEngineEvent(evt->getEvent(i));
          //duplicate event handler
          if (mEvents.find(evtcode) != mEvents.end()){
            TR_ERROR("duplicate event handler %i - ignoring...", evtcode);
            break;
          }
          mEvents.insert(evtcode);
          if (i < numEvents - 1){
            //handle multiple events
            if (evtcode == EVT_LOOP1 || evtcode == EVT_LOOP2)
              TR_BREAK("Not handled correctly yet.");
            CBEEVT* cevt = new CBEEVT(evtcode);
            cevt->setOffset(numEvents - i);
            codes->addCode(cevt);
            ++count;
          }
        }
        CodeSegment* oldcodes = codes;
        if (evtcode == EVT_LOOP1){
          CodeSegment* cs = new CodeSegment;
          ExecutionContext* loop1 = new ExecutionContext(cs, false, "");
          mLoop1 = loop1; //loop1 as seperate execution context
          codes = cs;
        }
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
          codes->addCode(new CCALL(0, 0));
          offset += 2;
          mUnresolvedBlockEnd = NULL;
        }
        cevt->setOffset(offset+1);
        if (evtcode == EVT_LOOP1){
          codes = oldcodes;
        }
        else
          count += offset;
      }
      break;
      case ASTNode::CONDITION:{
        CondNode* cond = static_cast<CondNode*>(node);
        mCurrFunc = cond->getCondFuncName();
        ScriptFunc f = mFunctions[cond->getCondFuncName()];
        if(f == NULL){
          f = ScriptFunctions::dummy;
          TR_BREAK("conditional script function %s does not exist", cond->getCondFuncName().c_str());
        }
        codes->addCode(new CFCPUSH(f, cond->getCondFuncName()));
        ++count;
        count += transform(cond->getArguments(), codes, ARGLIST);
        codes->addCode(new CCALL(cond->getArguments()->size(), 2));
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
        if (var->getChild() != NULL){ //build variable name dynamically
          codes->addCode(new CPUSH(var->name()));
          ++count;
          count += transform(var->getChild(), codes);
          codes->addCode(new CCONCAT());
          codes->addCode(new CSLOAD());
          count += 2;
        }
        else{
          codes->addCode(new CLOAD(var->name().c_str()));
          ++count;
        }
      }
      break;
      case ASTNode::RELATIONAL:{
        RelationalNode* relnode = static_cast<RelationalNode*>(node);
        if (relnode->type() == RelationalNode::REL_PLUS || relnode->type() == RelationalNode::REL_MINUS || relnode->type() == RelationalNode::REL_TIMES){
          //CLOAD var; visit child; CADD/CSUB
          std::map<std::string, std::map<int, std::string> >::iterator funciter = mRelVars.find(mCurrFunc);
          if (funciter == mRelVars.end()){
            if (mCurrFunc == "offspeech" && mCurrArg == 3){
              //allow strings that look like relative numbers / variables
              const char* tmp = " ";
              if (relnode->type() == RelationalNode::REL_PLUS)
                tmp = "+";
              else if (relnode->type() == RelationalNode::REL_MINUS)
                tmp = "-";
              else if (relnode->type() == RelationalNode::REL_TIMES)
                tmp = "*";
              CPUSH* push = new CPUSH(tmp);
              codes->addCode(push);
              count += 1;
              count += transform(relnode->child(), codes);
              codes->addCode(new CCONCAT());
              count += 1;
              break;
            }
            else
              TR_BREAK("special handling needed");
          }
          std::map<int, std::string>::iterator argiter = funciter->second.find(mCurrArg);
          std::string prefix = "";
          if (argiter == funciter->second.end())
            TR_BREAK("Unknown argument");
          else
            prefix = argiter->second;
          if (!prefix.empty())
            codes->addCode(new CPUSH(prefix));
          count += transform(mFirstArg, codes);
          if (!prefix.empty()){
            codes->addCode(new CCONCAT());
            count += 2;
          }
          codes->addCode(new CSLOAD());
          ++count;
          count += transform(relnode->child(), codes);
          if (relnode->type() == RelationalNode::REL_PLUS)
            codes->addCode(new CADD());
          else if (relnode->type() == RelationalNode::REL_MINUS)
            codes->addCode(new CSUB());
          else if (relnode->type() == RelationalNode::REL_TIMES){
            if (relnode->negated()){
              codes->addCode(new CPUSH(-1));
              codes->addCode(new CMUL());
              count += 2;
            }
            codes->addCode(new CMUL());
          }
          else
            TR_BREAK("Unknown type");
          count += 1;
        }
        else if (relnode->type() == RelationalNode::REL_EQUAL || relnode->type() == RelationalNode::REL_LESS || relnode->type() == RelationalNode::REL_GREATER){
          mLastRelation = relnode;
          count += transform(relnode->child(), codes);
          if (relnode->negated()){
            codes->addCode(new CPUSH(-1));
            codes->addCode(new CMUL());
            count += 2;
          }
        }
        else
          TR_BREAK("Unhandled rel type");
      }
      break;
      case ASTNode::LEVELDEF:{
        LevelNode* lvl = static_cast<LevelNode*>(node);
        EngineEvent evtcode = (EngineEvent)(EVT_LEVEL+lvl->getLevel());
        CBNEEVT* cevt = new CBNEEVT(evtcode);
        codes->addCode(cevt);
        ++count;
        int offset = transform(lvl->getBlock(), codes, START);
        cevt->setOffset(offset+1);
        count += offset;
      }
      break;
      case ASTNode::ROWDEF:{
        RowNode* row = static_cast<RowNode*>(node);
        if (!mInRow){
          CBNEROW* crow = new CBNEROW(row->getRow(), row->getText().c_str(), row->isVisible());
          codes->addCode(crow);
          ++count;
          mInRow = true;
          int offset = transform(row->getBlock(), codes, START);
          mInRow = false;
          crow->setOffset(offset + 1);
          count += offset;
        }
        else{
          TR_ERROR("Skipping nested row statement: %s", row->getText().c_str());
        }
      }
      break;
      case ASTNode::TIMERFUNC:{
        TimerNode* timer = static_cast<TimerNode*>(node);
        count += transform(timer->getTime(), codes, ARGLIST);
        CodeSegment* seg = new CodeSegment();
        transform(timer->getCommands(), seg, START);
        ExecutionContext* ctx = new ExecutionContext(seg, false, "");
        CTIMER* timercode = new CTIMER(ctx);
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
        else if (arnode->type() == ArithmeticNode::AR_IDIV)
          codes->addCode(new CIDIV());
        else
          TR_BREAK("Unknown type");
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
      case ASTNode::UNARY_ARITH:{
        UnaryArithNode* unari = static_cast<UnaryArithNode*>(node);
        count += transform(unari->node(), codes);
        if (unari->type() == UnaryArithNode::UA_DEC_SHIFT)
          codes->addCode(new CDecShift());
        else if (unari->type() == UnaryArithNode::UA_I2R)
          codes->addCode(new CI2R());
        else
          TR_BREAK("Unknown type");
        count += 1;
      }
      break;
      default:
        TR_BREAK("Unhandled AST-Type");
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
  lua_pushcfunction(mL, func);
  lua_setglobal(mL, name.c_str());
}

char const* condLuaHelper =
"function is_%s (...)\n"
"  local a,b = if_%s(...);\n"
"  return a == b\n"
"end\n";

void PcdkScript::registerConditional(std::string name, ScriptFunc func){
  registerFunction("if_" + name, func);
  char tmp[1024];
  sprintf(tmp, condLuaHelper, name.c_str(), name.c_str());
  if (luaL_dostring(mL, tmp) != LUA_OK){
    TR_USE(ADV_Script);
    TR_ERROR("Failed to compile cond helper script: %s", lua_tostring(mL, -1));
    lua_pop(mL, 1);
  }

}

void PcdkScript::registerRelVar(const std::string& function, int argnum, const std::string& prefix){
  mRelVars[function][argnum] = prefix;
}

bool PcdkScript::update(unsigned time){
  mTimeAccu += time;
  if (mTimeAccu < 20){ //limit hertz of script execution
    return false;
  }
  mTimeAccu -= 20;
  time = 20;
  applyPrevState(NULL);
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
  //copy for save list modifications
  mScriptMutex.lock();
  std::list<ExecutionContext*> tmpScripts = mScripts;
  for (std::list<ExecutionContext*>::iterator iter = tmpScripts.begin(); iter != tmpScripts.end(); ++iter){
    (*iter)->ref();
  }
  mScriptMutex.unlock();
  for (std::list<ExecutionContext*>::iterator iter = tmpScripts.begin(); iter != tmpScripts.end(); ++iter){
    std::list<EngineEvent> events;
    if (mGlobalSuspend){
      //only set loop1 which is always executed
      events = (*iter)->getEvents();
      if (!events.empty()){
        if (events.front() == EVT_CLICK){ //the cutscene should stop the current click
          events.pop_front(); //stop click
          events.pop_front(); //stop user event
        }
      }
      (*iter)->resetEvents(false);
      (*iter)->setEvent(EVT_LOOP1);
    }
    //don't delay enter when it exists
    if (!events.empty() && events.front() == EVT_ENTER){
      events.pop_front();
      (*iter)->getEvents().push_front(EVT_ENTER);
    }
    //this was the previous handling, but does not work for alühn cutscene realending, scrolling part
    //if (events.empty() || events.front() != EVT_ENTER) //execute only if it is not EVT_ENTER that is pending when being in global suspend
    
    ExecutionContext* current = *iter;
    update(current, time);

    if (mGlobalSuspend){
      current->setEvents(events);
    }
    if (current->mExecuteOnce && !current->mSuspended){
      if (current->mOwner == NULL){
        mTimers.erase(current);
        current->unref();
      }
      mScripts.remove(current);
    }
  }
  for (std::list<ExecutionContext*>::iterator iter = tmpScripts.begin(); iter != tmpScripts.end(); ++iter){
    if ((*iter)->unref()){
      continue;
    }
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
      oldcutscene->unref();
    if (mCutScene && !mCutScene->mSuspended && mCutScene->mExecuteOnce && mCutScene == oldcutscene){
      mGlobalSuspend = false;
      mHideUI = false;
      Engine::instance()->setCommand(mPrevActiveCommand, false);
      mCutScene->unref();
      mCutScene = NULL;
      //reload room if changed through cutscene
      CharacterObject* chr = Engine::instance()->getCharacter("self");
      if (chr)
        Engine::instance()->loadMainRoom(chr->getRoom(), NULL, Engine::instance()->getScreenChange());
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
  return true;
}

bool PcdkScript::willUpdate(unsigned interval){
  return mTimeAccu + interval >= 20;
}

void PcdkScript::update(ExecutionContext* ctx, unsigned time, bool execute){
  if (ctx->mSleepTime > 0 && ctx->mSuspended){
    (ctx->mSleepTime -= time);
    if (ctx->mSleepTime <= 0){
      ctx->mSleepTime = 0;
      ctx->resume();
    }
  }
  if (ctx->getLoop1() != NULL){ //update timer of loop1 scripts
    update(ctx->getLoop1(), time, false);
  }
  if (execute)
    executeImmediately(ctx);
}

void PcdkScript::execute(ExecutionContext* script, bool executeOnce){
  if (script == NULL)
    return;
  mScriptMutex.lock();
  script->mExecuteOnce = executeOnce;
  //disallow double add
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    if (*iter == script){
      mScriptMutex.unlock();
      return;
    }
  }
  mScripts.push_back(script);
  mScriptMutex.unlock();
}

int PcdkScript::luaPcdkCall(lua_State* L){
  lua_pushthread(L);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua_getfield(L, -1, "ec");
  ExecutionContext* script = (ExecutionContext*)lua_touserdata(L, -1);
  lua_pop(L, 2);
  CCode* code = script->mCode->get(script->mPC);
  while (code){
    int result = script->mPC = code->execute(*script, script->mPC);
    if (script->mSuspended)
      return lua_yieldk(L, lua_gettop(L), 0, luaPcdkCall);
    code = script->mCode->get(script->mPC);
  }
  return lua_gettop(L);
}

bool PcdkScript::executeImmediately(ExecutionContext* script, bool clearStackAfterExec){
  if (script->getLoop1() != NULL){
    if (!(!script->getEvents().empty() && script->getEvents().front() == EVT_ENTER)) //loop events only after enter
      script->getLoop1()->setEvent(EVT_LOOP1);
    executeImmediately(script->getLoop1(), clearStackAfterExec);
  }
  do{
    if (script->mSuspended)
      return false;
    lua_State* L = script->mL;
    mExecMutex.lock();
    if (script->mCode == NULL){
      //real lua script
      int numArgs = lua_gettop(L);
      if (script->mLuaRet != LUA_YIELD)
        lua_getglobal(L, "execScript");
      script->mLuaRet = lua_resume(script->mL, NULL, numArgs);
    }
    else{
      //pcdk script
      int numArgs = lua_gettop(L);
      if (script->mLuaRet != LUA_YIELD)
        lua_pushcfunction(L, luaPcdkCall);
      script->mLuaRet = lua_resume(script->mL, NULL, numArgs);
    }
    mExecMutex.unlock();
    if (script->mLuaRet == LUA_YIELD){
      return false;
    }
    else if (script->mLuaRet != LUA_OK){
      TR_USE(ADV_Console);
      int top = lua_gettop(script->mL);
      TR_ERROR("%i stack entries:", top);
      for (int i = 0; i < top; ++i){
        int t = lua_type(script->mL, i);
        switch (t){
        case LUA_TSTRING:
          TR_ERROR("%s", lua_tostring(script->mL, i));
          break;
        default:
          TR_ERROR("%s", lua_typename(script->mL, i));
          break;
        }
      }
      //const char* msg = lua_tostring(script->mL, -1);
      //TR_ERROR("%s", msg);
      lua_pop(script->mL, 1);
    }
    else{
      //lua_pop(script->mL, 1);
    }
    
    //script ran through
    if (!script->mSuspended && 
      ((script->mCode && script->mPC >= script->mCode->numInstructions()) || //pcdk
      (!script->mCode && script->mLuaRet == LUA_OK)) // lua
      ){
      //if (script->mHandler)
      //  script->mHandler(*script);
      clickEndHandler(*script);
      script->reset(false, clearStackAfterExec);
      if (!script->getEvents().empty())
        script->getEvents().pop_front();
    }
  } while (!script->getEvents().empty() && !script->mExecuteOnce);
  return true;
}

void PcdkScript::executeCutscene(ExecutionContext* script, bool looping){
  mGlobalSuspend = true;
  if (mCutScene != NULL){
    Engine::instance()->setCommand(mPrevActiveCommand, false);
    mCutScene->unref();
  }
  mCutScene = script;
  if (script == NULL){
    mGlobalSuspend = false;
    mHideUI = false;
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
  applyPrevState(object);
  remove(object->getScript());
}

void PcdkScript::remove(ExecutionContext* script){
  if (script == NULL)
    return;
  mScriptMutex.lock();
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    if (*iter == script){
      iter = mScripts.erase(iter);
      break;
    }
    if (iter == mScripts.end())
      break;
  }
  script->reset(true, true);
  mTimers.erase(script);
  Engine::instance()->removeScript(script);
  script->unref();
  mScriptMutex.unlock();
}

EngineEvent PcdkScript::getEngineEvent(const std::string eventname){
  if (eventname == "none")
    return EVT_NONE;
  else if (eventname == "mouse")
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
  else if (eventname == "rightclick")
    return EVT_RIGHTCLICK;
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
  TR_USE(ADV_Script);
  TR_BREAK("Unknown event %s", eventname.c_str());
  return EVT_NONE;
}

void PcdkScript::clickEndHandler(ExecutionContext& ctx){
  //if (ctx.isEventSet(EVT_CLICK)){
  //  ctx.resetEvent(EVT_CLICK);
  //}
  EngineEvent evt = ctx.getCommandEvent();
  if (evt != EVT_NONE && evt != EVT_LINK && evt != EVT_GIVE_LINK && ctx.isEventSet(evt) && !ctx.isEventHandled()){
    //an action remained unhandled
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (chr && chr->getScript() != NULL){
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
      if (chr  && !ctx.isEventHandled() && chr->getScript())
        chr->getScript()->setEvent(EVT_CANT_ALL);
      Engine::instance()->setUseObject("", "");
    }
    if (ctx.isEventSet(EVT_GIVE_LINK)){
      //ctx.resetEvent(EVT_GIVE_LINK);
      if (chr  && !ctx.isEventHandled() && chr->getScript())
        chr->getScript()->setEvent(EVT_CANT_ALL);
      Engine::instance()->setGiveObject("", "");
    }
  }
}

std::ostream& PcdkScript::save(std::ostream& out){
  lua_getglobal(mL, "bool");
  int numVars = 0;
  lua_pushnil(mL);
  while (lua_next(mL, -2)){
    ++numVars;
    lua_pop(mL, 1);
  }
  out << numVars << std::endl;
  lua_pushnil(mL);
  while (lua_next(mL, -2)){
    String name = lua_tostring(mL, -2);
    bool val = lua_toboolean(mL, -1) != 0;
    out << name << " " << val << std::endl;
    lua_pop(mL, 1);
  }
  lua_pop(mL, 1); //"bool"

  lua_getglobal(mL, "var");
  numVars = 0;
  lua_pushnil(mL);
  while (lua_next(mL, -2)){
    ++numVars;
    lua_pop(mL, 1);
  }
  out << numVars << std::endl;
  lua_pushnil(mL);
  while (lua_next(mL, -2)){
    std::string name = luaL_checkstring(mL, -2);
    if (name.empty())
      name = "none";
    StackData val = StackData::fromStack(mL, -1);
    out << name << " " << val << std::endl;
    lua_pop(mL, 1);
  }
  lua_pop(mL, 1); //"var"
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
    out << *mGroups[i] << std::endl;
  }
  out << mScriptFunctions.size();
  for (std::map<std::string, ExecutionContext*>::iterator iter = mScriptFunctions.begin(); iter != mScriptFunctions.end(); ++iter){
    out << " " << iter->first;
  }
  out << std::endl;
  out << mTextSpeed << " " << mRunSpeed << std::endl;
  out << mOfftextColor << std::endl;
  out << mOffAlign << " " << mOffRelative << std::endl;
  out << mTimers.size() << " ";
  for (std::set<ExecutionContext*>::iterator iter = mTimers.begin(); iter != mTimers.end(); ++iter){
    (*iter)->save(out);
    out << std::endl;
  }
  out << mItemStates.size() << std::endl;
  for (std::map<String,int>::iterator iter = mItemStates.begin(); iter != mItemStates.end(); ++iter){
    out << iter->first << " " << iter->second << std::endl;
  }
  return out;
}

std::istream& PcdkScript::load(std::istream& in, int version){
  lua_newtable(mL);
  lua_setglobal(mL, "bool");

  lua_newtable(mL);
  lua_newtable(mL);
  lua_pushcfunction(mL, getSpecialVar);
  lua_setfield(mL, -2, "__index");
  lua_pushcfunction(mL, setSpecialVar);
  lua_setfield(mL, -2, "__newindex");
  lua_setmetatable(mL, -2);
  lua_setglobal(mL, "var");

  mTSActive.clear();
  for (std::vector<ObjectGroup*>::iterator iter = mGroups.begin(); iter != mGroups.end(); ++iter){
    delete *iter;
  }
  mGroups.clear();
  int num1, num2, num3;
  in >> num1;
  std::string name;
  String name2;
  bool value;
  lua_getglobal(mL, "bool");
  for (int i = 0; i < num1; ++i){
    in >> name2 >> value;
    lua_pushboolean(mL, value);
    lua_setfield(mL, -2, name2.c_str());
  }
  lua_pop(mL, 1);
  StackData data;
  in >> num1;
  lua_getglobal(mL, "var");
  for (int i = 0; i < num1; ++i){
    in >> name >> data;
    StackData::pushStack(mL, data);
    lua_setfield(mL, -2, name.c_str());
  }
  lua_pop(mL, 1);
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
  in >> mTextSpeed >> mRunSpeed;
  in >> mOfftextColor;
  if (version >= 2){
    int tmp;
    in >> tmp;
    mOffAlign = (Alignment)tmp;
    in >> mOffRelative;
  }
  else{
    mOffAlign = ALGN_CENTER;
    mOffRelative = false;
  }
  //timers
  while(!mTimers.empty()){
    remove(*mTimers.begin());
  }
  unsigned numtimers;
  in >> numtimers;
  for (unsigned i = 0; i < numtimers; ++i){
    ExecutionContext* ctx = new ExecutionContext(in);
    addTimer(ctx);
  }
  //mItemStates.clear(); item states are already cleared before
  unsigned numitemstates;
  in >> numitemstates;
  for (unsigned i = 0; i < numitemstates; ++i){
    String name;
    int state;
    in >> name >> state;
    mItemStates[name] = state;
    Object2D* activeItem = Engine::instance()->getObject(name, true);
    if (activeItem)
      activeItem->setState(state);
  }
  return in;
}

namespace adv{

std::ostream& operator<<(std::ostream& strm, const StackData& data){
  strm << data.mType << " "; 
  if (data.mType == StackData::S){
    if (data.mStr.empty())
       strm << String("none");
    else 
      strm << data.mStr;
  }
  else
    strm << data.mInt;
  return strm;
}

std::istream& operator>>(std::istream& strm, StackData& data){
  int tmp;
  strm >> tmp;
  data.mType = (StackData::Type)tmp;
  if (data.mType == StackData::S){
    strm >> data.mStr;
    if (data.mStr == "none")
      data.mStr = "";
  }
  else
    strm >> data.mInt;
  return strm;
}

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

int PcdkScript::getSpecialVar(lua_State* L){
  TR_USE(ADV_Script);
  String name = lua_tostring(L, 2);
  String lname = name.toLower();
  if (name.size() > 0 && name[0] == '_'){
    if (name.size() > 6 && lname.substr(1, 6) == "volume"){
      lua_pushinteger(L, int(SoundEngine::instance()->getMusicVolume() * 100));
      return 1;
    }
    else if (name.size() > 9 && lname.substr(1, 9) == "charstate"){
      lua_pushinteger(L, 0);
      return 1;
    }
    else if (name.size() > 8 && lname.substr(1, 8) == "objstate"){
      TR_BREAK("Implement me");
      return 0;
    }
    else if (name.size() > 8 && lname.substr(1, 8) == "txtoutx:"){
      std::string idstr = lname.substr(9);
      Textout* txt = Engine::instance()->getFontRenderer()->getTextout(idstr);
      lua_pushinteger(L, Engine::instance()->getAnimator()->getTargetPoisition(txt).x);
      return 1;
    }
    else if (name.size() > 8 && lname.substr(1, 8) == "txtouty:"){
      std::string idstr = lname.substr(9);
      Textout* txt = Engine::instance()->getFontRenderer()->getTextout(idstr);
      lua_pushinteger(L, Engine::instance()->getAnimator()->getTargetPoisition(txt).y);
      return 1;
    }
  }
  else if (lname == "mousex"){
    lua_pushinteger(L, Engine::instance()->getCursorPos().x);
    return 1;
  }
  else if (lname == "mousey"){
    lua_pushinteger(L, Engine::instance()->getCursorPos().y);
    return 1;
  }
  else if (lname == "hour"){
    lua_pushinteger(L, getTime(TM_HOUR));
    return 1;
  }
  else if (lname == "minute"){
    lua_pushinteger(L, getTime(TM_MINUTE));
    return 1;
  }
  else if (lname == "second"){
    lua_pushinteger(L, getTime(TM_SECOND));
    return 1;
  }
  else if (lname == "year"){
    lua_pushinteger(L, getTime(TM_YEAR));
    return 1;
  }
  else if (lname == "month"){
    lua_pushinteger(L, getTime(TM_MONTH));
    return 1;
  }
  else if (lname == "day"){
    lua_pushinteger(L, getTime(TM_DAY));
    return 1;
  }
  else if (lname == "currentroom"){
    RoomObject* room = Engine::instance()->getRoom("");
    if (!room)
      TR_BREAK("Room not found");
    lua_pushstring(L, room->getName().c_str());
    return 1;
  }
  else if (lname == "roomx"){
    RoomObject* room = Engine::instance()->getRoom("");
    if (!room)
      TR_BREAK("Room not found");
    lua_pushnumber(L, -room->getScrollOffset().x / Engine::instance()->getWalkGridSize(false));
    return 1;
  }
  else if (lname == "roomy"){
    RoomObject* room = Engine::instance()->getRoom("");
    if (!room)
      TR_BREAK("Room not found");
    lua_pushnumber(L, -room->getScrollOffset().y / Engine::instance()->getWalkGridSize(false));
    return 1;
  }
  else if (lname == "roompx"){
    RoomObject* room = Engine::instance()->getRoom("");
    if (!room)
      TR_BREAK("Room not found");
    lua_pushnumber(L, -room->getScrollOffset().x);
    return 1;
  }
  else if (lname == "roompy"){
    RoomObject* room = Engine::instance()->getRoom("");
    if (!room)
      TR_BREAK("Room not found");
    lua_pushnumber(L, -room->getScrollOffset().y);
    return 1;
  }
  else if (lname == "charx"){
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (!chr){
      lua_pushinteger(L, 0);
      return 1;
    }
    lua_pushinteger(L, chr->getPosition().x);
    return 1;
  }
  else if (lname == "chary"){
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (!chr){
      lua_pushinteger(L, 0);
      return 1;
    }
    return chr->getPosition().y;
  }
  else if (lname == "charzoom"){
    TR_BREAK("Implement me");
    return 0;
  }
  else if (lname.size() > 9 && lname.substr(0, 9) == "charzoom:"){
    TR_BREAK("Implement me");
    return 0;
  }
  else if (name.size() > 5 && lname.substr(0, 5) == "char:"){
    CharacterObject* chr = Engine::instance()->getCharacter(name.substr(5));
    if (!chr){
      SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(name.substr(5));
      if (!cso)
        TR_BREAK("Character %s not found", name.substr(5).c_str());
      lua_pushinteger(L, cso->base.state);
      return 1;
    }
    lua_pushinteger(L, chr->getState());
    return 1;
  }
  else if (name.size() > 6 && lname.substr(0, 6) == "charx:"){
    int idx = 6;
    bool wmpos = false;
    if (name[6] == '_'){
      wmpos = true;
      idx = 7;
    }
    else if (name[6] == ' '){
      idx = 7;
    }
    String charname = name.substr(idx);
    charname = charname.removeAll(' ');
    CharacterObject* chr = Engine::instance()->getCharacter(charname);
    if (!chr){
      std::string room;
      std::string realname;
      SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname, room, realname);
      if (cso == NULL){
        TR_BREAK("Character %s not found", name.substr(idx).c_str());
        lua_pushinteger(L, 0);
        return 1;
      }
      if (wmpos){
        SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(room);
        lua_pushnumber(L, cso->base.position.x / Engine::instance()->getWalkGridSize(sr->doublewalkmap));
        return 1;
      }
      lua_pushinteger(L, cso->base.position.x);
      return 1;
    }
    lua_pushnumber(L, chr->getPosition().x / (wmpos ? chr->getWalkGridSize() : 1));
    return 1;
  }
  else if (name.size() > 6 && lname.substr(0, 6) == "chary:"){
    int idx = 6;
    bool wmpos = false;
    if (name[6] == '_'){
      wmpos = true;
      idx = 7;
    }
    else if (name[6] == ' '){
      idx = 7;
    }
    String charname = name.substr(idx);
    charname = charname.removeAll(' ');
    CharacterObject* chr = Engine::instance()->getCharacter(charname);
    if (!chr){
      std::string room;
      std::string realname;
      SaveStateProvider::CharSaveObject* cso = Engine::instance()->getSaver()->findCharacter(charname, room, realname);
      if (cso == NULL){
        TR_BREAK("Character %s not found", name.substr(idx).c_str());
        lua_pushinteger(L, 0);
        return 1;
      }
      if (wmpos){
        SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(room);
        lua_pushnumber(L, cso->base.position.y / Engine::instance()->getWalkGridSize(sr->doublewalkmap));
        return 1;
      }
      lua_pushinteger(L, cso->base.position.y);
      return 1;
    }
    lua_pushnumber(L, chr->getPosition().y / (wmpos ? chr->getWalkGridSize() : 1));
    return 1;
  }
  else if (name.size() > 9 && lname.substr(0, 9) == "charzoom:"){
    TR_BREAK("Implement me");
    return 0;
  }
  else if (name.size() > 4 && lname.substr(0, 4) == "obj:"){
    Object2D* obj = Engine::instance()->getObject(name.substr(4), false);
    if (obj == NULL)
      TR_BREAK("Object %s not found", name.substr(4).c_str());
    lua_pushinteger(L, obj->getState());
    return 1;
  }
  else if (name.size() > 5 && lname.substr(0, 5) == "objx:"){
    Object2D* obj = Engine::instance()->getObject(name.substr(5), false);
    if (obj == NULL){
      TR_BREAK("Object %s not found", name.substr(5).c_str());
      lua_pushinteger(L, 0);
    }
    else
      lua_pushinteger(L, obj->getPosition().x);
    return 1;
  }
  else if (name.size() > 5 && lname.substr(0, 5) == "objy:"){
    Object2D* obj = Engine::instance()->getObject(name.substr(5), false);
    if (obj == NULL)
      TR_BREAK("Object %s not found", name.substr(5).c_str());
    lua_pushinteger(L, obj->getPosition().y);
    return 1;
  }
  else if (name.size() > 8 && lname.substr(0, 8) == "tgtobjx:"){
    String objname = name.substr(8);
    objname = objname.removeAll(' ');
    ObjectGroup* grp = Engine::instance()->getInterpreter()->getGroup(objname);
    if (grp)
      objname = grp->getObjects()[0];
    Object2D* obj = Engine::instance()->getObject(objname, false);
    if (obj == NULL){
      std::string dummy;
      SaveStateProvider::SaveObject* so = Engine::instance()->getSaver()->findObject(objname, dummy);
      if (so == NULL)
        TR_BREAK("Object %s not found", name.substr(8).c_str());
      lua_pushinteger(L, so->position.x);
      return 1;
    }
    lua_pushinteger(L, Engine::instance()->getAnimator()->getTargetPoisition(obj).x);
    return 1;
  }
  else if (name.size() > 8 && lname.substr(0, 8) == "tgtobjy:"){
    String objname = name.substr(8);
    objname = objname.removeAll(' ');
    ObjectGroup* grp = Engine::instance()->getInterpreter()->getGroup(objname);
    if (grp)
      objname = grp->getObjects()[0];
    Object2D* obj = Engine::instance()->getObject(objname, false);
    if (obj == NULL){
      std::string dummy;
      SaveStateProvider::SaveObject* so = Engine::instance()->getSaver()->findObject(objname, dummy);
      if (so == NULL)
        TR_BREAK("Object %s not found", name.substr(8).c_str());
      lua_pushinteger(L, so->position.y);
      return 1;
    }
    lua_pushinteger(L, Engine::instance()->getAnimator()->getTargetPoisition(obj).y);
    return 1;
  }
  else if (name.size() > 5 && lname.substr(0, 5) == "objw:"){
    Object2D* obj = Engine::instance()->getObject(name.substr(5), false);
    if (obj == NULL)
      TR_BREAK("Object %s not found", name.substr(5).c_str());
    lua_pushinteger(L, obj->getSize().x);
    return 1;
  }
  else if (name.size() > 5 && lname.substr(0, 5) == "objh:"){
    Object2D* obj = Engine::instance()->getObject(name.substr(5), false);
    if (obj == NULL)
      TR_BREAK("Object %s not found", name.substr(5).c_str());
    lua_pushinteger(L, obj->getSize().y);
    return 1;
  }
  else if (lname == "actiontext"){
    lua_pushstring(L, Engine::instance()->getActionText().c_str());
    return 1;
  }
  else if (lname == "empty"){
    lua_pushstring(L, "");
    return 1;
  }
  else if (lname == "leftbracket"){
    lua_pushstring(L, "(");
    return 1;
  }
  else if (lname == "rightbracket"){
    lua_pushstring(L, ")");
    return 1;
  }
  else if (name.size() > 7 && lname.substr(0, 7) == "gamepad"){
    TR_BREAK("Implement me");
  }
  //transformed raw lookup
  lua_pushstring(L, lname.removeAll(' ').c_str());
  lua_rawget(L, 1);
  return 1;
}

int PcdkScript::getDataVar(lua_State* L){
  //create a new table, store and return it
  TR_USE(ADV_Script);
  String name = lua_tostring(L, 2);
  String lname = name.toLower();
  lua_newtable(L);

  lua_pushstring(L, lname.removeAll(' ').c_str());
  lua_setfield(L, -2, "name");

  lua_pushstring(L, lname.removeAll(' ').c_str());
  lua_pushvalue(L, -2);
  lua_rawset(L, 1);

  lua_newtable(L);
  lua_pushcfunction(L, specialVarAccessor);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  return 1;
}

int PcdkScript::specialVarAccessor(lua_State* L){
  String name = lua_tostring(L, 2);
  lua_getfield(L, 1, "name");
  String obj = lua_tostring(L, -1);
  lua_pop(L, 1);
  lua_getglobal(L, "var");
  lua_getfield(L, -1, (name+":"+obj).c_str());
  lua_remove(L, -2);
  return 1;
}

StackData PcdkScript::getVariable(const String& name){
  lua_getglobal(mL, "var");
  lua_getfield(mL, -1, name.c_str());
  StackData ret = StackData::fromStack(mL, -1);
  lua_pop(mL, 2);
  return ret;
}

int PcdkScript::setSpecialVar(lua_State* L){
  String name = lua_tostring(L, 2);
  String lname = name.toLower();
  if (lname == "mousex"){
    int x = (int)lua_tointeger(L, 3);
    Engine::instance()->setMousePosition(x, Engine::instance()->getCursorPos().y);
    return 0;
  }
  else if (lname == "mousey"){
    int y = (int)lua_tointeger(L, 3);
    Engine::instance()->setMousePosition(Engine::instance()->getCursorPos().x, y);
    return 0;
  }
  //transformed raw set
  lua_pushstring(L, lname.removeAll(' ').c_str());
  lua_pushnil(L);
  lua_copy(L, 3, -1);
  lua_rawset(L, 1);
  return 0;
}

void PcdkScript::setVariable(const String& name, const StackData& value){
  lua_getglobal(mL, "var");
  StackData::pushStack(mL, value);
  lua_setfield(mL, -2, name.c_str());
  lua_pop(mL, 1);
}

bool PcdkScript::isVariable(const String& name){
  lua_getglobal(mL, "var");
  lua_getfield(mL, -1, name.toLower().removeAll(' ').c_str());
  bool ret = lua_isnil(mL, -1) == 0;
  lua_pop(mL, 2);
  return ret;
}

void PcdkScript::deleteVariable(const String& name){
  lua_getglobal(mL, "var");
  lua_pushnil(mL);
  lua_setfield(mL, -2, name.toLower().removeAll(' ').c_str());
  lua_pop(mL, 1);
}

void PcdkScript::setPrevState(Object2D* trigger, Object2D* target){
  mPrevState.insert(std::make_pair(trigger, std::make_pair(target,target->getState())));
}

bool PcdkScript::applyPrevState(Object2D* obj){
  if (obj == NULL){
    std::map<Object2D*, std::pair<Object2D*,int> >::iterator deliter = mPrevState.end();
    Vec2i cursor = Engine::instance()->getCursorPos();
    for (std::map<Object2D*, std::pair<Object2D*,int> >::iterator iter = mPrevState.begin(); iter != mPrevState.end(); ++iter){
      if (iter->second.second == -1){
        deliter = iter;
        continue;
      }
      if (!iter->first->isHit(cursor-iter->first->getScrollOffset())){
        iter->second.first->setState(iter->second.second);
        iter->second.second = -1;
        deliter = iter;
      }
    }
    if (deliter != mPrevState.end())
      mPrevState.erase(deliter);
  }
  std::map<Object2D*, std::pair<Object2D*,int> >::iterator iter = mPrevState.find(obj);
  if (iter != mPrevState.end()){
    iter->second.first->setState(iter->second.second);
    mPrevState.erase(iter);
    return true;
  }
  return false;
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

ExecutionContext* PcdkScript::removeScript(const std::string& name, bool onlyFromFunctions){
  std::map<std::string,ExecutionContext*>::iterator iter = mScriptFunctions.find(name);
  if (iter != mScriptFunctions.end()){
    ExecutionContext* scr = iter->second;
    if (!onlyFromFunctions)
      remove(scr);
    mScriptFunctions.erase(iter);
    return scr;
  }
  return NULL;
}

void PcdkScript::addTimer(ExecutionContext* timer){
  mTimers.insert(timer);
  execute(timer, true);
}

ASTNode* parseLangArg(const char* funcname, int argnum, int strindex){
  ASTNode* node = Engine::instance()->getInterpreter()->parseLangArg(funcname, argnum, strindex);
  return node;
}

ASTNode* PcdkScript::parseLangArg(const char* funcname, int argnum, int strindex){
  Language::Section sec = mData->getLanguageSection(funcname, argnum);
  std::string text = mData->getLanguageString(mLanguage, sec, strindex);
  if (text.size() == 1 && text[0] == ' ')
    return NULL;
  text += ";";

  pANTLR3_INPUT_STREAM input;
  ppcdkLexer lexer;
  pANTLR3_COMMON_TOKEN_STREAM tokStream;
  ppcdkParser parser;
  pcdkParser_arg_return pcdkAST;
  input = antlr3NewAsciiStringInPlaceStream(((pANTLR3_UINT8)text.c_str()), (ANTLR3_UINT32)text.size(), (pANTLR3_UINT8)"Script");
  lexer = pcdkLexerNew(input);
  tokStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
  parser = pcdkParserNew(tokStream);
  parser->pParser->super = parser;
  parser->pParser->rec->reportError = reportParseError;
  parser->pParser->rec->recoverFromMismatchedToken = recoverFromMismatchedToken;
  pcdkAST = parser->arg(parser);
  ASTNode* node = pcdkAST.value;
  if (parser->pParser->rec->state->errorCount > 0){
    TR_USE(ADV_Script)
    //CGE::Engine::instance()->messageBox("Error parsing argument", "Error");
    TR_ERROR("Error parsing argument %s", text.c_str());
#undef free
    parser->free(parser);
    tokStream->free(tokStream);
    lexer->free(lexer);
    input->free(input);
    delete node;
    return NULL;
  }
  parser->free(parser);
  tokStream->free(tokStream);
  lexer->free(lexer);
  input->free(input);
#ifdef _CRTDBG_MAP_ALLOC
#define free _free_dbg
#endif
  return node;
}

ObjectGroup* PcdkScript::getGroup(const std::string& name){
  for (std::vector<ObjectGroup*>::iterator iter = mGroups.begin(); iter != mGroups.end(); ++iter){
    if ((*iter)->getName() == name)
      return *iter;
  }
  return NULL;
}

int PcdkScript::getItemState(const String& name){
  mMutex.lock(); //TODO really mMutex and not mScriptMutex???
  std::map<String,int>::iterator iter = mItemStates.find(name.toLower());
  if (iter == mItemStates.end()){
    mMutex.unlock();
    return 1;
  }
  mMutex.unlock();
  return iter->second;
}

lua_State* PcdkScript::allocNewState(ExecutionContext* ctx){
  mExecMutex.lock();
  lua_State* ret = lua_newthread(mL);
  lua_newtable(mL);
  lua_pushlightuserdata(mL, ctx);
  lua_setfield(mL, -2, "ec");
  lua_settable(mL, LUA_REGISTRYINDEX);
  mExecMutex.unlock();
  return ret;
}

StackData StackData::fromStack(lua_State* L, int idx){
  int type = lua_type(L, idx);
  if (type == LUA_TNUMBER){
    double num = lua_tonumber(L, idx);
    StackData ret;
    if ((double)(int)num == num)
      ret = StackData((int)num);
    else
      ret = StackData((float)num);
    return ret;
  }
  else if (type == LUA_TSTRING){
    const char* str = lua_tostring(L, idx);
    StackData ret = StackData(String(str));
    return ret;
  }
  else if (type == LUA_TBOOLEAN){
    bool b = lua_toboolean(L, idx) != 0;
    StackData ret = StackData(b);
    return ret;
  }
  else if (type == LUA_TLIGHTUSERDATA){
    ExecutionContext* ctx = (ExecutionContext*)lua_touserdata(L, idx);
    StackData ret = StackData(ctx);
    return ret;
  }
  else if (type == LUA_TFUNCTION){
    lua_pushvalue(L, idx);
    ExecutionContext* ctx = new ExecutionContext(L);
    StackData ret = StackData(ctx, true);
    return ret;
  }
  return StackData(0);
}

void StackData::pushStack(lua_State* L, StackData const& value){
  if (value.isInt())
    lua_pushinteger(L, value.getInt());
  else if (value.isFloat())
    lua_pushnumber(L, value.getFloat());
  else if (value.isString())
    lua_pushstring(L, value.getString().c_str());
  else if (value.isBool())
    lua_pushboolean(L, value.getBool());
  else if (value.isEC())
    lua_pushlightuserdata(L, value.getEC());
  else
    lua_pushnil(L);
}

int PcdkScript::varToString(lua_State* L){
  String name = lua_tostring(L, 2);
  if (name == "_state"){
    lua_pushthread(L);
    lua_gettable(L, LUA_REGISTRYINDEX);
  }
  else if (name == "on"){
    lua_pushthread(L);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_getfield(L, -1, "event");
    lua_remove(L, -2);
  }
  else{
    lua_pushnil(L);
    lua_copy(L, 2, -1);
  }
  return 1;
}

int PcdkScript::eventStrToEventId(lua_State* L){
  const char* ev = lua_tostring(L, 1);
  EngineEvent event = Engine::instance()->getInterpreter()->getEngineEvent(ev);
  lua_pushinteger(L, event);
  return 1;
}

int PcdkScript::eventFired(lua_State* L){
  lua_getfield(L, 1, "ec");
  ExecutionContext* ctx = (ExecutionContext*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  EngineEvent event = (EngineEvent)lua_tointeger(L, 2);
  bool ret = ctx->isEventSet(event);
  lua_pushboolean(L, ret ? 1 : 0);
  return 1;
}

int PcdkScript::eventHandled(lua_State* L){
  lua_getfield(L, 1, "ec");
  ExecutionContext* ctx = (ExecutionContext*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  ctx->setEventHandled();
  return 0;
}

int PcdkScript::setIdle(lua_State* L){
  if (lua_gettop(L) < 2)
    return luaL_error(L, "2 arguments expected");
  lua_getfield(L, 1, "ec");
  ExecutionContext* ctx = (ExecutionContext*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  bool idle = lua_toboolean(L, 2) != 0 ? true : false;
  ctx->setIdle(idle);
  return 0;
}

void PcdkScript::initLuaContext(ExecutionContext* ctx){
  lua_State* L = ctx->getState();
  lua_pushthread(L);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua_pushvalue(L, -2);
  lua_setfield(L, -2, "script");
  lua_newtable(L);
  lua_newtable(L);
  lua_getglobal(L, "on_newidx");
  //lua_pushcfunction(L, setEventHandler);
  lua_setfield(L, -2, "__newindex");
  lua_setmetatable(L, -2);
  //lua_pushnil(L);
  //lua_copy(L, -2, -1);
  lua_setfield(L, -2, "event");
  lua_pushcfunction(L, eventFired);
  lua_setfield(L, -2, "eventFired");
  lua_pushcfunction(L, eventHandled);
  lua_setfield(L, -2, "eventHandled");
  lua_pushcfunction(L, setIdle);
  lua_setfield(L, -2, "setIdle");
  lua_pop(L, 2);
}
