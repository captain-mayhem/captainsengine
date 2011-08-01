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

class Object2D;
class ObjectGroup;

class PcdkScript{
  friend class ScriptFunctions;
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
  void executeImmediately(ExecutionContext* script, bool clearStackAfterExec = true);
  void executeCutscene(ExecutionContext* cutscene, bool looping);
  void update(unsigned time);
  void remove(Object2D* object);
  void remove(ExecutionContext* script);
  StackData getVariable(const std::string& name);
  void setVariable(const std::string& name, const StackData& value);
  CBRA* getBranchInstr(RelationalNode* relnode, bool negated);
  Vec2i& tsPos() {return mTSPos;}
  std::map<int, bool>& tsActive() {return mTSActive[mTSName][mTSLevel];}
  int getTSWidth() {return mTSWidth;}
  void stop();
  std::ostream& save(std::ostream& out);
  std::istream& load(std::istream& in);
  bool isBlockingScriptRunning() {return mGlobalSuspend;}
  static void clickEndHandler(ExecutionContext& ctx);
  ExecutionContext* getCutscene() {return mCutScene;}
  void applyPrevState(Object2D* obj);
  ExecutionContext* getScript(const std::string& name);
  void removeScript(const std::string& name);
protected:
  unsigned transform(NodeList* program, CodeSegment* codes, TrMode mode, int seperateContext = -1);
  unsigned transform(ASTNode* node, CodeSegment* codes);
  std::string mObjectInfo;
  bool mIsGameObject;
  std::list<std::pair<CBRA*, unsigned> > mUnresolvedBranches;
  RelationalNode* mLastRelation;
  CLOAD* mUnresolvedLoad;

  EngineEvent getEngineEvent(const std::string eventname);
  void update(ExecutionContext* ctx, unsigned time);
  std::map<std::string, ScriptFunc> mFunctions;
  AdvDocument* mData;
  std::list<ExecutionContext*> mScripts;
  ExecutionContext* mCutScene;
  std::map<std::string,bool> mBooleans;
  std::map<std::string,StackData> mVariables;
  static bool mRemoveLinkObject;
  std::string mTSName;
  int mTSLevel;
  int mNextTSLevel;
  int mTSRow;
  Vec2i mTSPosOrig;
  Vec2i mTSPos;
  int mTSWidth;
  std::map<std::string, std::map<int, std::map<int, bool> > > mTSActive;
  bool mGlobalSuspend;
  std::string mPrevActiveCommand;
  std::map<Object2D*, int> mPrevState;
  std::map<std::string, int> mKeymap;
  std::vector<ObjectGroup*> mGroups;
  bool mACharacterAtScriptStart;
  std::map<std::string, ExecutionContext*> mScriptFunctions;
};

#endif
