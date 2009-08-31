#ifndef PCDK_SCRIPT_H
#define PCDK_SCRIPT_H

#include <map>
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
  protected:
    std::vector<CCode*> mCodes;
  };

  class ExecutionContext{
    CodeSegment* mCode;
    Stack mStack;
    unsigned mPC;
  };

  PcdkScript(AdvDocument* data);
  ~PcdkScript();
  CodeSegment* parseProgram(std::string program);
  void registerFunction(std::string name, ScriptFunc func);
  void execute(CodeSegment* segment);

  static int loadRoom(Stack& s, unsigned numArgs);
  static int setFocus(Stack& s, unsigned numArgs);
protected:
  unsigned transform(NodeList* program, CodeSegment* codes, TrMode mode);
  unsigned transform(ASTNode* node, CodeSegment* codes);
  EngineEvent getEngineEvent(const std::string eventname);
  std::map<std::string, ScriptFunc> mFunctions;
  Stack mStack;
  unsigned mPc;
  AdvDocument* mData;
};

#endif
