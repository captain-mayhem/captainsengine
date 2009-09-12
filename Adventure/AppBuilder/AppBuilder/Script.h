#ifndef PCDK_SCRIPT_H
#define PCDK_SCRIPT_H

#include <map>
#include <set>
#include <list>
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

  PcdkScript(AdvDocument* data);
  ~PcdkScript();
  ExecutionContext* parseProgram(std::string program);
  void registerFunction(std::string name, ScriptFunc func);
  void execute(ExecutionContext* script, bool executeOnce);
  void update();
  void remove(ExecutionContext* script);

  static int loadRoom(ExecutionContext& ctx, unsigned numArgs);
  static int setBool(ExecutionContext& ctx, unsigned numArgs);
  static int setFocus(ExecutionContext& ctx, unsigned numArgs);
  static int setLight(ExecutionContext& ctx, unsigned numArgs);
  static int setObj(ExecutionContext& ctx, unsigned numArgs);
  static int showInfo(ExecutionContext& ctx, unsigned numArgs);
  static int speech(ExecutionContext& ctx, unsigned numArgs);
  static int pickup(ExecutionContext& ctx, unsigned numArgs);
  static int playSound(ExecutionContext& ctx, unsigned numArgs);
  static int walkTo(ExecutionContext& ctx, unsigned numArgs);

  static int isBoolEqual(ExecutionContext& ctx, unsigned numArgs);
  static int isCommandSet(ExecutionContext& ctx, unsigned numArgs);
  static int isObjectInState(ExecutionContext& ctx, unsigned numArgs);

  static void clickEndHandler(ExecutionContext& ctx, void* data);
protected:
  void execute(ExecutionContext* script);
  unsigned transform(NodeList* program, CodeSegment* codes, TrMode mode);
  unsigned transform(ASTNode* node, CodeSegment* codes);
  std::list<std::pair<CBRA*, unsigned> > mUnresolvedBranches;
  EngineEvent getEngineEvent(const std::string eventname);
  std::map<std::string, ScriptFunc> mFunctions;
  AdvDocument* mData;
  std::list<ExecutionContext*> mScripts;
  std::map<std::string,bool> mBooleans;
};

#endif
