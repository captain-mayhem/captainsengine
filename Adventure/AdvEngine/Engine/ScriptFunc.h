#ifndef SCRIPT_FUNC_H
#define SCRIPT_FUNC_H

#include <vector>
#include <string>

#include "Ids.h"

typedef struct lua_State lua_State;

namespace adv{

class ExecutionContext;
class PcdkScript;
struct Character;
class StackData;

class ScriptFunctions{
public:
  static void registerFunctions(PcdkScript* interpreter);
  /**
  \param textscene name of the textscene to activate
  \param level level of the textscene
  \param row row to deactivate
  **/
  static int activate(lua_State* L);
  /**
  \param character name of the character to whose inventory the item should be added
  special values off
  \param item the item to add
  special values givelink
  \param num [opt] the inventory number
  **/
  static int addItem(lua_State* L);
  /**
  \param character name of the character to beam
  \param room room to beam to
  \param x x-position to beam to
  \param y y-position to beam to
  \param dir [opt] look direction
  **/
  static int beamTo(lua_State* L);
  /**
    \param number identifies the textout line
    \param room the room to which the text should be bound
  **/
  static int bindText(lua_State* L);
  /**
  **/
  static int breakExec(lua_State* L);
  /**
  \param char the character to simulate click on
  **/
  static int charSimClick(lua_State* L);
  /**
    \param character the character whose zoom should be changed
    \param size the size of the character
    \param fade [opt] fade to the new size
  **/
  static int charZoom(lua_State* L);
  /**
  **/
  static int coinReturn(lua_State* L);
  /**
  \param command name of the command to set
  **/
  static int command(lua_State* L);
  /**
  \param cutscene name of the cutscene to start
  \param donthide [opt] leaves anywhere-objects and user panel there
  **/
  static int cutScene(lua_State* L);
  /**
  \param textscene name of the textscene to deactivate
  \param level level of the textscene
  \param row row to deactivate
  **/
  static int deactivate(lua_State* L);
  /**
  \param character name of the character from whose inventory the item should be removed
  special values off
  \param item the item to remove
  special values givelink
  \param num [opt] the inventory number
  **/
  static int delItem(lua_State* L);
    /**
  \param enable if FX shapes are enabled
  **/
  static int enableFXShape(lua_State* L);
  /**
  \param enable if menu is enabled
  **/
  static int enableMenu(lua_State* L);
  /**
  \param show enable the mouse
  **/
  static int enableMouse(lua_State* L);
  /**
  **/
  static int endScene(lua_State* L);
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
  static int enterText(lua_State* L);
  /**
  \param char1 the first character whose inventories should be exchanged
  \param char2 the second character
  **/
  static int exchange(lua_State* L);
  /**
  \param speed 1-15 or time in ms
  **/
  static int fadeSpeed(lua_State* L);
  /**
  \param char1 name of the character to follow char2
  \param char2 name of the character to be followed by char1
  \param dontwait [opt] continue script execution immediately
  **/
  static int follow(lua_State* L);
  /**
  \param scriptname name of the script to execute as function
  \param loops [opt] number of loops to execute (also "*" or "infinity" possible)
  **/
  static int function(lua_State* L);
  /**
  \param name [opt] name of the object to link for giving
  **/
  static int giveLink(lua_State* L);
  /**
  \param level set the level of a textscene
  **/
  static int gotoLevel(lua_State* L);
  /**
  \param object name of the group
  \param state first object of the group
  \param ... [opt] additional objects to group
  **/
  static int group(lua_State* L);
  /*
  */
  static int hideAllTexts(lua_State* L);
  /**
  \param icon mouse icon number
  **/
  static int instMouse(lua_State* L);
  /**
  \param object name of the object instance
  \param state state of the object
  \param ... [opt] additional states TODO
  **/
  static int instObj(lua_State* L);
  /**
  \param number scrolls the inventory number steps down
  **/
  static int invDown(lua_State* L);
  /**
  \param number scrolls the inventory number steps up
  **/
  static int invUp(lua_State* L);
  /**
  \param item the item to simulate click on
  **/
  static int itemSimClick(lua_State* L);
  /**
  \param seconds time to jiggle
  \param power how strong the jiggling is
  **/
  static int jiggle(lua_State* L);
  /**
  \param name name of the object to link
  **/
  static int link(lua_State* L);
  /**
  \param character the character to be linked to an object
  \param object the object to link to
  **/
  static int linkChar(lua_State* L);
  /**
  \param character preload the character character
  **/
  static int loadChar(lua_State* L);
  /**
  \param slot the save slot to be used
  **/
  static int loadGame(lua_State* L);
  /**
  \param name the name of the variable to load
  **/
  static int loadNum(lua_State* L);
  /**
  \param room name of the room
  \param effect [opt] blending effect
  **/
  static int loadRoom(lua_State* L);
  /**
  \param name the name of the variable to load
  **/
  static int loadString(lua_State* L);
  /**
  \param char name of the character to change look direction
  \param dir direction number or character name
  **/
  static int lookTo(lua_State* L);
  /**
  \param sound the sound name which should be played in loop
  \param volume [opt] the volume of the sound
  **/
  static int loopSound(lua_State* L);
  /**
  \param sound the looping sound name which should be stopped
  **/
  static int loopStop(lua_State* L);
  /**
  \param donthide [opt] don't hide the taskbar / anywhere objects
  **/
  static int miniCut(lua_State* L);
  /**
    Engine internal
  **/
  static int miniCutEnd(lua_State* L);
  /**
  \param instance name of the object instance
  \param x the x-position the object should be moved to
  \param y the y-position the object should be moved to
  \param speed/time the moving speed (9-1, 0) or time (x ms)
  \param wait [opt] if the script should be suspended
  **/
  static int moveObj(lua_State* L);
  /**
  \param id id of the textout instance
  \param x the x-position the text should be moved to
  \param y the y-position the text should be moved to
  \param speed/time the moving speed (9-1, 0) or time (x ms)
  \param wait [opt] if the script should be suspended
  **/
  static int moveText(lua_State* L);
  /**
  \param volume the volume of the music
  **/
  static int musicVolume(lua_State* L);
  /**
  \param alignment
  **/
  static int offAlign(lua_State* L);
  /**
  \param x x-coord of text (middle)
  \param y y-coord of text (bottom)
  \param text the text to be displayed
  \param sample [opt] sound file to be played TODO
  \param dontwait [opt] continue script execution immediately
  **/
  static int offSpeech(lua_State* L);
  /**
  \param red
  \param green
  \param blue
  **/
  static int offTextColor(lua_State* L);
  /**
  \param view 1: in front of all, 2: behind front objects, 3: behind middle objects, 4: behind background
  **/
  static int particleView(lua_State* L);
  /**
  \param character the name of the character to execute the pickup action
  **/
  static int pickup(lua_State* L);
  /**
  \param prefix prefix name of all animation graphics
  \param fps frames per second
  \param x x position
  \param y y position
  \param width width of animation
  \param height heigt of animation
  \param wait [opt] wait for end of animation
  **/
  static int playAnimation(lua_State* L);
  /**
  \param name the music file which should be played
  \param position [opt] start position (as pattern for module track)
  **/
  static int playMusic(lua_State* L);
  /**
  \param sound name of the sound file to play
  \param volume [opt] the volume
  **/
  static int playSound(lua_State* L);
  /**
  \param movie name of the swf movie file to play
  \param x [opt] x-position of the movie
  \param y [opt] y-position of the movie
  \param width [opt] width of the movie
  \param height [opt] height of the movie
  **/
  static int playSwf(lua_State* L);
  /**
  \param movie name of the swf movie file to play
  \param suspend suspend script execution
  \param x [opt] x-position of the movie
  \param y [opt] y-position of the movie
  \param width [opt] width of the movie
  \param height [opt] height of the movie
  **/
  static int playVideo(lua_State* L);
  /**
  **/
  static int popupCoin(lua_State* L);
  /**
  **/
  static int popupMenu(lua_State* L);
  /**
  **/
  static int quit(lua_State* L);
  /**
  \param name the variable to be filled with a random number
  \param limit limits the number to 1 .. limit
  **/
  static int randomNum(lua_State* L);
  /**
  \param realtime true/false
  **/
  static int realTime(lua_State* L);
  /**
  **/
  static int restart(lua_State* L);
  /**
  \param style (intelligent/classic/nothing)
  **/
  static int rightClickStyle(lua_State* L);
  /**
  \param row the row number
  \param text the text to be displayed
  \param active whether row is active from the beginning
  **/
  static int row(lua_State* L);
  /**
  \param speed 50 - 300 %
  **/
  static int runSpeed(lua_State* L);
  /**
  \param character name of the character to walk
  \param x x-position to walk to
  \param y y-position to walk to
  \param dir [opt] look direction
  \param dontwait [opt] continue script execution immediately
  **/
  static int runTo(lua_State* L);
  /**
  \param slot the save slot to be used
  **/
  static int saveGame(lua_State* L);
  /**
  \param name of the variable to be saved
  **/
  static int saveNum(lua_State* L);
  /**
  \param name of the variable to be saved
  **/
  static int saveString(lua_State* L);
  /**
  \param speed the scroll speed 1-20 (+100 for don't follow)
  **/
  static int scrollSpeed(lua_State* L);
  /**
  \param bool name of the boolean
  \param value true/false
  **/
  static int setBool(lua_State* L);
  /**
  \param char name of the character instance
  \param state extra state of the character (number or name)
  \param ... [opt] additional states
  **/
  static int setChar(lua_State* L);
  /**
  \param char name of the character instance
  \param a alpha value
  \param fade [opt] do a fading to the color
  **/
  static int setCharAlpha(lua_State* L);
  /**
  \param char name of the character instance
  \param r red component
  \param g green component
  \param b blue component
  \param fade [opt] do a fading to the color
  **/
  static int setCharLight(lua_State* L);
  /**
  \param effect sets the dsp effect
  **/
  static int setDSP(lua_State* L);
  /**
  \param effect sets the eax effect
  **/
  static int setEAX(lua_State* L);
  /**
  \param item the item
  \param state the new state
  \param ... [opt] additional states
  **/
  static int setItem(lua_State* L);
  /**
  \param character name of the character to focus
  special values none/last
  **/
  static int setFocus(lua_State* L);
  /**
  \param number number of the font to activate
  \param character [opt] set the font for a character
  **/
  static int setFont(lua_State* L);
  /**
  \param language choose the language for the game
  **/
  static int setLanguage(lua_State* L);
  /**
  \param room name of the room
  \param r red-component
  \param g green-component
  \param b blue-component
  \param fade [opt] fading effect TODO
  **/
  static int setLight(lua_State* L);
  /**
  \param name the name of the variable
  \param value the value to set
  **/
  static int setNum(lua_State* L);
  /**
  \param object name of the object instance
  \param state state of the object
  \param ... [opt] additional states
  **/
  static int setObj(lua_State* L);
  /**
  \param obj name of the object instance
  \param a alpha value
  \param fade [opt] do a fading to the color
  **/
  static int setObjAlpha(lua_State* L);
    /**
  \param obj name of the object instance
  \param r red component
  \param g green component
  \param b blue component
  \param fade [opt] do a fading to the color
  **/
  static int setObjLight(lua_State* L);
  /**
  \param object name of the particle object
  \param speed the speed of the particle
  \param amount the number of particles
  \param direction the direction of the particles (0 down, 90 right, ...)
  \param rotation the rotation angle speed of the particle (clockwise)
  \param variation the variation in speed of the particles in percent
  **/
  static int setParticles(lua_State* L);
  /**
  \param room the room where the camera position should be set
  \param x the x-position of the camera
  \param y the y-position of the camera
  \param immediately set the camera position immediately
  \param dir [opt] up/down/left/right moves the new position graphically to the old one
  **/
  static int setPos(lua_State* L);
  /**
  \param room the room for which its walkmap should be modified
  \param x1 start x-position of the walkmap rectangle
  \param y1 start y-position of the walkmap rectangle
  \param x2 end x-position of the walkmap rectangle
  \param y2 end y-position of the walkmap rectangle
  \param walkable if the field should be made walkable
  **/
  static int setRectWalkmap(lua_State* L);
  /**
  \param change changes style of room changes
  **/
  static int setScreenchange(lua_State* L);
  /**
  \param name the name of the variable
  \param value the value to set
  **/
  static int setString(lua_State* L);
  /**
  \param transparency the transparency of overall and taskbar room
  **/
  static int setTransparency(lua_State* L);
  /**
  \param room the room for which its walkmap should be modified
  \param x x-position of the walkmap field
  \param y y-position of the walkmap field
  \param walkable if the field should be made walkable
  **/
  static int setWalkmap(lua_State* L);
  /**
  \param character the name of the character
  \param soundname the name of the new walk sound
  **/
  static int setWalkSound(lua_State* L);
  /**
  **/
  static int showAllText(lua_State* L);
  /**
  \param text information about the object
  \param show boolean if the text should be displayed on the mouse cursor
  **/
  static int showInfo(lua_State* L);
  /**
  \param inventory intentory to show (1-5)
  **/
  static int showInventory(lua_State* L);
  /**
  \param show show the mouse cursor
  **/
  static int showMouse(lua_State* L);
  /**
  \param the object being (sim) clicked
  **/
  static int simClick(lua_State* L);
  /**
  \param volume the master volume
  **/
  static int soundVolume(lua_State* L);
  /**
  \param character name of the character to speak
  \param text the text to be displayed
  \param sample [opt] sound file to be played TODO
  \param dontwait [opt] continue script execution immediately
  **/
  static int speech(lua_State* L);
  /**
  \param volume the volume of speech
  **/
  static int speechVolume(lua_State* L);
  /**
  \param name the name of the variable a square root should be calculated of
  **/
  static int sqrt(lua_State* L);
  /**
  \param effect name of the effect
  \param args [opt] additional effect parameters
  **/
  static int startEffect(lua_State* L);
  /**
  \param fast [opt] show particles immediately
  **/
  static int startParticles(lua_State* L);
  /**
  \param character name of the character
  \param direction makes a step into the given direction
  **/
  static int stepTo(lua_State* L);
  /**
  \param effect the effect to stop
  **/
  static int stopEffect(lua_State* L);
  /**
  \param scriptname the function to stop
  **/
  static int stopFunction(lua_State* L);
   /**
  **/
  static int stopMusic(lua_State* L);
  /**
  \param fast [opt] show particles immediately
  **/
  static int stopParticles(lua_State* L);
  /*
  */
  static int stopSkip(lua_State* L);
  /**
  **/
  static int stopSwf(lua_State* L);
  /**
  \param character character to change zoom behaviour
  \param doit stop zooming
  **/
  static int stopZooming(lua_State* L);
  /**
  \param room name of the subroom to load
  \param fadingtime [opt] time in milliseconds for fading effect
  **/
  static int subRoom(lua_State* L);
  /**
  **/
  static int subRoomReturn(lua_State* L);
  /**
  **/
  static int subRoomReturnImmediate(lua_State* L);
  /**
  \param char1 first character to switch position and inventory
  \param char2 second character to switch position and inventory
  **/
  static int switchCharacter(lua_State* L);
  /**
  \param num the number of the text
  \param alignment left, right, middle
  **/
  static int textAlign(lua_State* L);
  /**
  \param num the number of the text
  **/
  static int textHide(lua_State* L);
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
  static int textOut(lua_State* L);
  /**
  \param textscene name of the textscene to start
  \param posx [opt] x-position of the textscene
  \param posy [opt] y-position of the textscene
  \param width [opt] width if the textscene
  **/
  static int textScene(lua_State* L);
  /**
  \param speed fast/normal/slow
  **/
  static int textSpeed(lua_State* L);
  /**
  \param show true/false
  **/
  static int taskbar(lua_State* L);
  /**
  \param show true/false
  **/
  static int textEnabled(lua_State* L);
  /**
  \param time the time when the function should be triggered
  \param func the function to be triggered
  **/
  static int timer(lua_State* L);
  /**
  \param character unlink character
  **/
  static int unlinkChar(lua_State* L);
  /**
  **/
  static int unloadRoom(lua_State* L);
  /**
  \param time time in seconds
  **/
  static int wait(lua_State* L);
  /**
  \param character name of the character to walk
  \param x x-position to walk to
  \param y y-position to walk to
  \param dir [opt] look direction
  \param dontwait [opt] continue script execution immediately
  **/
  static int walkTo(lua_State* L);

  static int dummy(lua_State* L);

  static int isBoolEqual(lua_State* L);
  static int isCharFocussed(lua_State* L);
  static int isCharInRoom(lua_State* L);
  static int isCharPossessingItem(lua_State* L);
  static int isCharTriggering(lua_State* L);
  static int isCommandSet(lua_State* L);
  static int isCurrentRoom(lua_State* L);
  static int isObjectInState(lua_State* L);
  static int isLinkedObject(lua_State* L);
  static int isGiveLinkedObject(lua_State* L);
  static int isNumEqual(lua_State* L);
  static int isKeyDownEqual(lua_State* L);
  static int isKeyPressedEqual(lua_State* L);
  static int isMouseWheelEqual(lua_State* L);
  static int isStringEqual(lua_State* L);
  static int isObjXPosEqual(lua_State* L);
  static int isObjYPosEqual(lua_State* L);
  static int isItemInState(lua_State* L);
  
  //debug functions
  static int print(lua_State* L);
 private:
  static void setObjInternal(std::vector<std::string> objects, std::vector<int> states, bool skip);
  static int moveTo(lua_State* L, float speedFactor);
  static int getRequestedState(Character* cclass, const StackData& data);
  static ScreenChange getScreenChange(const std::string& name);
};

}

#endif
