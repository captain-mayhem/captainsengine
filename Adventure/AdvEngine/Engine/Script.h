#ifndef PCDK_SCRIPT_H
#define PCDK_SCRIPT_H

#include <map>
#include <set>
#include <list>
#include <antlr3.h>
#include "pcdkLexer.h"
#include "pcdkParser.h"
#include "AdvDoc.h"
#include "String.h"

#include "CIL.h"

extern ASTNode* stringify(ASTNode* tree);
typedef struct lua_State lua_State;

namespace adv{

class Object2D;
class ObjectGroup;

class PcdkScript{
  friend class ScriptFunctions;
  friend ASTNode* ::stringify(ASTNode* tree);
public:
  enum TrMode{
    START,
    ARGLIST,
    REVERSE,
  };

  PcdkScript(AdvDocument* data);
  ~PcdkScript();
  ExecutionContext* parseProgram(const std::string& program, ScriptLang type = DEFAULT_SCRIPT);
  void registerFunction(std::string name, ScriptFunc func);
  void registerConditional(std::string name, ScriptFunc func);
  ScriptFunc getFunction(const std::string name) {return mFunctions[name];}
  void registerRelVar(const std::string& function, int argnum, const std::string& prefix);
  void getArgumentAsExecutionContext(const std::string& function, int argnum) {mArgEC[function] = argnum;}
  void execute(ExecutionContext* script, bool executeOnce);
  bool executeImmediately(ExecutionContext* script, bool clearStackAfterExec = true);
  void executeCutscene(ExecutionContext* cutscene, bool looping);
  bool update(unsigned time);
  void remove(Object2D* object);
  void remove(ExecutionContext* script);
  StackData getVariable(const String& name);
  void setVariable(const String& name, const StackData& value);
  bool isVariable(const String& name);
  void deleteVariable(const String& name);
  CBRA* getBranchInstr(RelationalNode* relnode, bool negated);
  Vec2i& tsPos() {return mTSPos;}
  std::map<int, bool>& tsActive() {return mTSActive[mTSName][mTSLevel];}
  int getTSWidth() {return mTSWidth;}
  bool isTSTopToBottom() {return mTSTopToBottom;}
  void stop();
  std::ostream& save(std::ostream& out);
  std::istream& load(std::istream& in);
  bool isBlockingScriptRunning() {return mGlobalSuspend;}
  bool isTextScene() {return mCutScene != NULL && !mCutScene->isExecuteOnce();}
  void resumeBlockingScript() {mGlobalSuspend = false;}
  static void clickEndHandler(ExecutionContext& ctx);
  ExecutionContext* getCutscene() {return mCutScene;}
  ExecutionContext* getScript(const std::string& name);
  ExecutionContext* removeScript(const std::string& name, bool onlyFromFunctions);
  int getTextSpeed() {return mTextSpeed;}
  void setTextSpeed(int speed) {mTextSpeed = speed;}
  void addTimer(ExecutionContext* timer);
  ASTNode* parseLangArg(const char* funcname, int argnum, int strindex);
  void setLanguage(const std::string& lang) {mLanguage = lang;}
  const std::string& getLanguage() {return mLanguage;}
  bool willUpdate(unsigned interval);
  ObjectGroup* getGroup(const std::string& name);
  void setPrevState(Object2D* trigger, Object2D* target);
  bool applyPrevState(Object2D* obj);
  void setRunSpeed(float speed) {mRunSpeed = speed;}
  float getRunSpeed() {return mRunSpeed;}
  const Color& getOfftextColor() {return mOfftextColor;}
  void setOfftextColor(const Color& color) {mOfftextColor = color;}
  void cutsceneMode(bool hideUI) {mGlobalSuspend = true; mHideUI = hideUI;}
  bool isUIHidden() {return mHideUI;}
  void setItemState(const String& name, int state) {mMutex.lock(); mItemStates[name.toLower()] = state; mMutex.unlock();}
  int getItemState(const String& name);
  void clearState() {mMutex.lock(); mItemStates.clear(); mMutex.unlock();}
  lua_State* allocNewState(ExecutionContext* ctx);
  static int luaPcdkCall(lua_State* L);
  CGE::Mutex& getExecMutex() { return mExecMutex; }
protected:
  ExecutionContext* parseProgramPCDK(const std::string& program);
  ExecutionContext* parseProgramLUA(const std::string& program, bool checkLoop1 = true);
  unsigned transform(NodeList* program, CodeSegment* codes, TrMode mode, int seperateContext = -1);
  unsigned transform(ASTNode* node, CodeSegment* codes);
  std::string internal_stringify(ASTNode* node);
  std::string mObjectInfo;
  bool mIsGameObject;
  std::list<std::pair<CBRA*, unsigned> > mUnresolvedBranches;
  RelationalNode* mLastRelation;
  CFCPUSH* mUnresolvedBlockEnd;
  std::string mCurrFunc;
  unsigned mCurrArg;
  std::map<std::string, std::map<int, std::string> > mRelVars;
  std::map<std::string, int> mArgEC;
  std::set<EngineEvent> mEvents;
  ASTNode* mFirstArg;
  ExecutionContext* mLoop1;
  CGE::Mutex mMutex;
  bool mInRow;

  EngineEvent getEngineEvent(const std::string eventname);
  void update(ExecutionContext* ctx, unsigned time, bool execute=true);
  static int getSpecialVar(lua_State* L);
  static int setSpecialVar(lua_State* L);
  static int varToString(lua_State* L);
  static int eventStrToEventId(lua_State* L);
  static int eventFired(lua_State* L);
  static int eventHandled(lua_State* L);
  static int setIdle(lua_State* L);
  std::map<std::string, ScriptFunc> mFunctions;
  AdvDocument* mData;
  std::list<ExecutionContext*> mScripts;
  ExecutionContext* mCutScene;
  static bool mRemoveLinkObject;
  std::string mTSName;
  int mTSLevel;
  int mNextTSLevel;
  int mTSRow;
  Vec2i mTSPosOrig;
  Vec2i mTSPos;
  int mTSWidth;
  std::map<std::string, std::map<int, std::map<int, bool> > > mTSActive;
  bool mTSTopToBottom;
  bool mGlobalSuspend;
  std::string mPrevActiveCommand;
  std::map<Object2D*, std::pair<Object2D*,int> > mPrevState;
  std::map<std::string, int> mKeymap;
  std::vector<ObjectGroup*> mGroups;
  bool mACharacterAtScriptStart;
  std::map<std::string, ExecutionContext*> mScriptFunctions;
  std::set<ExecutionContext*> mTimers;
  int mTextSpeed;
  std::string mLanguage;
  unsigned mTimeAccu;
  float mRunSpeed;
  Color mOfftextColor;
  bool mHideUI;
  std::map<String, int> mItemStates;
  CGE::Mutex mScriptMutex;
  CGE::Mutex mExecMutex;
  lua_State* mL;
};

}

#endif
