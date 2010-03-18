#ifndef SCRIPT_FUNC_H
#define SCRIPT_FUNC_H

class ExecutionContext;
class PcdkScript;

class ScriptFunctions{
public:
  static void registerFunctions(PcdkScript* interpreter);
  /**
  \param textscene name of the textscene to activate
  \param level level of the textscene
  \param row row to deactivate
  **/
  static int activate(ExecutionContext& ctx, unsigned numArgs);
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
  \param object name of the group
  \param state first object of the group
  \param ... [opt] additional objects to group
  **/
  static int group(ExecutionContext& ctx, unsigned numArgs);
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
  \param seconds time to jiggle
  \param power how strong the jiggling is
  **/
  static int jiggle(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name name of the object to link
  **/
  static int link(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param slot the save slot to be used
  **/
  static int loadGame(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name the name of the variable to load
  **/
  static int loadNum(ExecutionContext& ctx, unsigned numArgs);
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
  /**
  \param name the music file which should be played
  \param position [opt] start position (as pattern for module track)
  **/
  static int playMusic(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param sound name of the sound file to play
  \param volume [opt] the volume
  **/
  static int playSound(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param movie name of the swf movie file to play
  \param x [opt] x-position of the movie
  \param y [opt] y-position of the movie
  \param width [opt] width of the movie
  \param height [opt] height of the movie
  **/
  static int playSwf(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name the variable to be filled with a random number
  \param limit limits the number to 1 .. limit
  **/
  static int randomNum(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param realtime true/false
  **/
  static int realTime(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int restart(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param slot the save slot to be used
  **/
  static int saveGame(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name of the variable to be saved
  **/
  static int saveNum(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param bool name of the boolean
  \param value true/false
  **/
  static int setBool(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param char name of the character instance
  \param state extra state of the character (number or name)
  \param ... [opt] additional states
  **/
  static int setChar(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param char name of the character instance
  \param r red component
  \param g green component
  \param b blue component
  \param fade [opt] do a fading to the color
  **/
  static int setCharLight(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param character name of the character to focus
  special values none/last
  **/
  static int setFocus(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param number number of the font to activate
  \param character [opt] set the font for a character
  **/
  static int setFont(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param room name of the room
  \param r red-component
  \param g green-component
  \param b blue-component
  \param fade [opt] fading effect TODO
  **/
  static int setLight(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name the name of the variable
  \param value the value to set
  **/
  static int setNum(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param object name of the object instance
  \param state state of the object
  \param ... [opt] additional states
  **/
  static int setObj(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param change changes style of room changes
  **/
  static int setScreenchange(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param name the name of the variable
  \param value the value to set
  **/
  static int setString(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param room the room for which its walkmap should be modified
  \param x x-position of the walkmap field
  \param y y-position of the walkmap field
  \param walkable if the field should be made walkable
  **/
  static int setWalkmap(ExecutionContext& ctx, unsigned numArgs);
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
  /*
  */
  static int stopSkip(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int stopSwf(ExecutionContext& ctx, unsigned numArgs);
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
  \param show true/false
  **/
  static int textEnabled(ExecutionContext& ctx, unsigned numArgs);
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

  static int dummy(ExecutionContext& ctx, unsigned numArgs);

  static int isBoolEqual(ExecutionContext& ctx, unsigned numArgs);
  static int isCharFocussed(ExecutionContext& cxt, unsigned numArgs);
  static int isCharInRoom(ExecutionContext& ctx, unsigned numArgs);
  static int isCharPossessingItem(ExecutionContext& ctx, unsigned numArgs);
  static int isCharTriggering(ExecutionContext& ctx, unsigned numArgs);
  static int isCommandSet(ExecutionContext& ctx, unsigned numArgs);
  static int isObjectInState(ExecutionContext& ctx, unsigned numArgs);
  static int isLinkedObject(ExecutionContext& ctx, unsigned numArgs);
  static int isGiveLinkedObject(ExecutionContext& ctx, unsigned numArgs);
  static int isNumEqual(ExecutionContext& ctx, unsigned numArgs);
};

#endif
