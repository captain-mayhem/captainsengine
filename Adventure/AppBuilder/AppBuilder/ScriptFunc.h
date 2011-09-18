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
    \param number identifies the textout line
    \param room the room to which the text should be bound
  **/
  static int bindText(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int breakExec(ExecutionContext& ctx, unsigned numArgs);
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
  \param display the string to display before the prompt
  \param variable the variable where to save the user's input
  \param x the x position of the prompt
  \param y the y position of the prompt
  \param font [opt] the font to use
  \param maxletters [opt] the maximum number of characters allowed as input
  \param red [opt] the red component of the text's color
  \param green [opt] the green component of the text's color
  \param blue [opt] the blue component of the text's color
  **/
  static int enterText(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param speed 1-15 or time in ms
  **/
  static int fadeSpeed(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param char1 name of the character to follow char2
  \param char2 name of the character to be followed by char1
  \param dontwait [opt] continue script execution immediately
  **/
  static int follow(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param scriptname name of the script to execute as function
  \param loops [opt] number of loops to execute (also "*" or "infinity" possible)
  **/
  static int function(ExecutionContext& ctx, unsigned numArgs);
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
  \param donthide [opt] don't hide the taskbar / anywhere objects
  **/
  static int miniCut(ExecutionContext& ctx, unsigned numArgs);
  /**
    Engine internal
  **/
  static int miniCutEnd(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param instance name of the object instance
  \param x the x-position the object should be moved to
  \param y the y-position the object should be moved to
  \param speed/time the moving speed (9-1, 0) or time (x ms)
  \param wait [opt] if the script should be suspended
  **/
  static int moveObj(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param volume the volume of the music
  **/
  static int musicVolume(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param x x-coord of text (middle)
  \param y y-coord of text (bottom)
  \param text the text to be displayed
  \param sample [opt] sound file to be played TODO
  \param dontwait [opt] continue script execution immediately
  **/
  static int offSpeech(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param view 1: in front of all, 2: behind front objects, 3: behind middle objects, 4: behind background
  **/
  static int particleView(ExecutionContext& ctx, unsigned numArgs);
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
  \param movie name of the swf movie file to play
  \param suspend suspend script execution
  \param x [opt] x-position of the movie
  \param y [opt] y-position of the movie
  \param width [opt] width of the movie
  \param height [opt] height of the movie
  **/
  static int playVideo(ExecutionContext& ctx, unsigned numArgs);
  /**
  **/
  static int quit(ExecutionContext& ctx, unsigned numArgs);
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
  \param effect sets the eax effect
  **/
  static int setEAX(ExecutionContext& ctx, unsigned numArgs);
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
  \param language choose the language for the game
  **/
  static int setLanguage(ExecutionContext& ctx, unsigned numArgs);
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
  \param object name of the particle object
  \param speed the speed of the particle
  \param amount the number of particles
  \param direction the direction of the particles (0 down, 90 right, ...)
  \param rotation the rotation angle speed of the particle (clockwise)
  \param variation the variation in speed of the particles in percent
  **/
  static int setParticles(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param room the room where the camera position should be set
  \param x the x-position of the camera
  \param y the y-position of the camera
  \param immediately set the camera position immediately
  \param dir [opt] up/down/left/right moves the new position graphically to the old one
  **/
  static int setPos(ExecutionContext& ctx, unsigned numArgs);
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
  /**
  \param volume the volume of speech
  **/
  static int speechVolume(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param effect name of the effect
  \param args [opt] additional effect parameters
  **/
  static int startEffect(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param fast [opt] show particles immediately
  **/
  static int startParticles(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param character name of the character
  \param direction makes a step into the given direction
  **/
  static int stepTo(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param effect the effect to stop
  **/
  static int stopEffect(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param scriptname the function to stop
  **/
  static int stopFunction(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param fast [opt] show particles immediately
  **/
  static int stopParticles(ExecutionContext& ctx, unsigned numArgs);
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
  \param num the number of the text
  **/
  static int textHide(ExecutionContext& ctx, unsigned numArgs);
   /**
  \param num the number of the text
  \param text the text to display
  \param x the x position of the text
  \param y the y position of the text
  \param font [opt] the font to use
  \param red [opt] the red component of the text's color
  \param green [opt] the green component of the text's color
  \param blue [opt] the blue component of the text's color
  **/
  static int textOut(ExecutionContext& ctx, unsigned numArgs);
  /**
  /**
  \param textscene name of the textscene to start
  \param posx [opt] x-position of the textscene
  \param posy [opt] y-position of the textscene
  \param width [opt] width if the textscene
  **/
  static int textScene(ExecutionContext& ctx, unsigned numArgs);
  /**
  \param speed fast/normal/slow
  **/
  static int textSpeed(ExecutionContext& ctx, unsigned numArgs);
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
  static int isCurrentRoom(ExecutionContext& ctx, unsigned numArgs);
  static int isObjectInState(ExecutionContext& ctx, unsigned numArgs);
  static int isLinkedObject(ExecutionContext& ctx, unsigned numArgs);
  static int isGiveLinkedObject(ExecutionContext& ctx, unsigned numArgs);
  static int isNumEqual(ExecutionContext& ctx, unsigned numArgs);
  static int isKeyDownEqual(ExecutionContext& ctx, unsigned numArgs);
  static int isKeyPressedEqual(ExecutionContext& ctx, unsigned numArgs);
  static int isMouseWheelEqual(ExecutionContext& ctx, unsigned numArgs);
  static int isStringEqual(ExecutionContext& ctx, unsigned numArgs);
  static int isObjXPosEqual(ExecutionContext& ctx, unsigned numArgs);
};

#endif
