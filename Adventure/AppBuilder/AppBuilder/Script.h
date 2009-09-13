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

  /**
  \param character name of the character to beam
  \param room room to beam to
  \param x x-position to beam to
  \param y y-position to beam to
  \param dir [opt] look direction
  **/
  static int beamTo(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param room name of the room
    \param effect [opt] blending effect TODO
  **/
  static int loadRoom(ExecutionContext& ctx, unsigned numArgs);
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
    \param character the name of the character to execute the pickup action
  **/
  static int pickup(ExecutionContext& ctx, unsigned numArgs);
  /** TODO
    \param sound name of the sound file to play
    \param volume [opt] the volume
  **/
  static int playSound(ExecutionContext& ctx, unsigned numArgs);
  /**
    \param character name of the character to walk
    \param x x-position to walk to
    \param y y-position to walk to
    \param dir [opt] look direction
    \param dontwait [opt] continue script execution immediately
  **/
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
