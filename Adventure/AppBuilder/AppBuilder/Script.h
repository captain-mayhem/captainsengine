#ifndef PCDK_SCRIPT_H
#define PCDK_SCRIPT_H

#include <map>
#include <set>
#include <antlr3.h>
#include "pcdkLexer.h"
#include "pcdkParser.h"
#include "AdvDoc.h"

#include "CIL.h"

class PcdkScript{
public:
  enum TrMode{
    START,
    ARGLIST,
  };

  class CodeSegment{
  public:
    CodeSegment(){}
    ~CodeSegment(){
      for (unsigned i = 0; i < mCodes.size(); ++i){
        delete mCodes[i];
      }
    }
    void addCode(CCode* code){
      mCodes.push_back(code);
    }
    CCode* get(unsigned pc){
      if (pc < mCodes.size())
        return mCodes[pc];
      return NULL;
    }
    unsigned numInstructions(){
      return (unsigned)mCodes.size();
    }
  protected:
    std::vector<CCode*> mCodes;
  };

  class ExecutionContext{
    friend class PcdkScript;
  public:
    ExecutionContext(CodeSegment* segment);
    ~ExecutionContext();
  protected:
    CodeSegment* mCode;
    Stack mStack;
    unsigned mPC;
  };

  PcdkScript(AdvDocument* data);
  ~PcdkScript();
  ExecutionContext* parseProgram(std::string program);
  void registerFunction(std::string name, ScriptFunc func);
  void execute(ExecutionContext* script);
  void setEvent(EngineEvent evt);
  void resetEvent(EngineEvent evt);
  bool isEventSet(EngineEvent evt);

  static int loadRoom(Stack& s, unsigned numArgs);
  static int setFocus(Stack& s, unsigned numArgs);
  static int showInfo(Stack& s, unsigned numArgs);
  static int walkTo(Stack& s, unsigned numArgs);
  static int speech(Stack& s, unsigned numArgs);
  static int pickup(Stack& s, unsigned numArgs);
  static int playSound(Stack& s, unsigned numArgs);
protected:
  unsigned transform(NodeList* program, CodeSegment* codes, TrMode mode);
  unsigned transform(ASTNode* node, CodeSegment* codes);
  EngineEvent getEngineEvent(const std::string eventname);
  std::map<std::string, ScriptFunc> mFunctions;
  AdvDocument* mData;
  std::set<EngineEvent> mEvents;
};

#endif
