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
  void executeImmediately(ExecutionContext* script);
  void executeCutscene(ExecutionContext* cutscene, bool looping);
  void update(unsigned time);
  void remove(ExecutionContext* script);
  std::map<std::string,StackData>& getVariables() {return mVariables;}
  CBRA* getBranchInstr(RelationalNode* relnode, bool negated);
  Vec2i& tsPos() {return mTSPos;}
  std::map<int, bool>& tsActive() {return mTSActive[mTSName][mTSLevel];}
  int getTSWidth() {return mTSWidth;}
  void stop();
  bool isBlockingScriptRunning() {return mGlobalSuspend;}

  /**
  \param character name of the character to whose inventory the item should be added
          special values off
  \param item the item to add
          special values givelink
  \param num [opt] the inventory number
  **/
  static int addItem(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param character name of the character to beam
  \param room room to beam to
  \param x x-position to beam to
  \param y y-position to beam to
  \param dir [opt] look direction
  **/
  static int beamTo(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param command name of the command to set
  **/
  static int command(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param cutscene name of the cutscene to start
    \param donthide [opt] leaves anywhere-objects and user panel there
   **/
  static int cutScene(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param textscene name of the textscene to deactivate
  \param level level of the textscene
  \param row row to deactivate
  **/
  static int deactivate(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param character name of the character from whose inventory the item should be removed
  special values off
  \param item the item to remove
  special values givelink
  \param num [opt] the inventory number
  **/
  static int delItem(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int endScene(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param char1 name of the character to follow char2
  \param char2 name of the character to be followed by char1
  \param dontwait [opt] continue script execution immediately
  **/
  static int follow(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name [opt] name of the object to link for giving
  **/
  static int giveLink(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param level set the level of a textscene
  **/
  static int gotoLevel(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param object name of the object instance
  \param state state of the object
  \param ... [opt] additional states TODO
  **/
  static int instObj(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param number scrolls the inventory number steps down
  **/
  static int invDown(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param number scrolls the inventory number steps up
  **/
  static int invUp(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name name of the object to link
  **/
  static int link(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param room name of the room
    \param effect [opt] blending effect TODO
  **/
  static int loadRoom(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param char name of the character to change look direction
  \param dir direction number or character name
  **/
  static int lookTo(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param sound the sound name which should be played in loop
  \param volume [opt] the volume of the sound
  **/
  static int loopSound(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param sound the looping sound name which should be stopped
  **/
  static int loopStop(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param x x-coord of text (middle)
  \param y y-coord of text (bottom)
  \param text the text to be displayed
  \param sample [opt] sound file to be played TODO
  \param dontwait [opt] continue script execution immediately
  **/
  static int offSpeech(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param character the name of the character to execute the pickup action
  **/
  static int pickup(ExecutionContext& ctx, unsigned numArgs);
  /** TODO
  \param name the music file which should be played
  \param position [opt] start position (as pattern for module track)
  **/
  static int playMusic(ExecutionContext& ctx, unsigned numArgs);
  /** TODO
  \param sound name of the sound file to play
  \param volume [opt] the volume
  **/
  static int playSound(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int restart(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param bool name of the boolean
    \param value true/false
  **/
  static int setBool(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param character name of the character to focus
           special values none/last TODO
  **/
  static int setFocus(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param room name of the room
    \param r red-component
    \param g green-component
    \param b blue-component
    \param fade [opt] fading effect TODO
  **/
  static int setLight(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param object name of the object instance
    \param state state of the object
    \param ... [opt] additional states TODO
  **/
  static int setObj(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param text information about the object
    \param show boolean if the text should be displayed on the mouse cursor
  **/
  static int showInfo(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param character name of the character to speak
    \param text the text to be displayed
    \param sample [opt] sound file to be played TODO
    \param dontwait [opt] continue script execution immediately
  **/
  static int speech(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param room name of the subroom to load
  \param fadingtime [opt] time in milliseconds for fading effect
  **/
  static int subRoom(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int subRoomReturn(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int stopMusic(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param textscene name of the textscene to start
  \param posx [opt] x-position of the textscene
  \param posy [opt] y-position of the textscene
  \param width [opt] width if the textscene
  **/
  static int textScene(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param show true/false
  **/
  static int taskbar(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int unloadRoom(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param time time in seconds
  **/
  static int wait(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param character name of the character to walk
    \param x x-position to walk to
    \param y y-position to walk to
    \param dir [opt] look direction
    \param dontwait [opt] continue script execution immediately
  **/
  static int walkTo(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name the name of the variable
  \param value the value to set
  **/
  static int setNum(ExecutionContext& ctx, unsigned numArgs);

  static int dummy(ExecutionContext& ctx, unsigned numArgs);

  static int isBoolEqual(ExecutionContext& ctx, unsigned numArgs);
  static int isCommandSet(ExecutionContext& ctx, unsigned numArgs);
  static int isObjectInState(ExecutionContext& ctx, unsigned numArgs);
  static int isLinkedObject(ExecutionContext& ctx, unsigned numArgs);
  static int isGiveLinkedObject(ExecutionContext& ctx, unsigned numArgs);
  static int isNumEqual(ExecutionContext& ctx, unsigned numArgs);

  static void clickEndHandler(ExecutionContext& ctx);
protected:
  unsigned transform(NodeList* program, CodeSegment* codes, TrMode mode);
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
  int mTSRow;
  Vec2i mTSPosOrig;
  Vec2i mTSPos;
  int mTSWidth;
  std::map<std::string, std::map<int, std::map<int, bool> > > mTSActive;
  bool mGlobalSuspend;
  std::string mPrevActiveCommand;
};

#endif
