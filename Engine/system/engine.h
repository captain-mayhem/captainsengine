#ifndef CGE_ENGINE_H
#define CGE_ENGINE_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <io/Tracing.h>

using std::string;
using std::ofstream;
using std::cerr;
using std::list;
using std::vector;

#define EXIT() (CGE::Engine::instance()->requestShutdown())
#define EXIT2(msg, ...) {TR_ERROR(msg, ##__VA_ARGS__); CGE::Engine::instance()->requestShutdown(); }
#define SAFE_DELETE(ptr)       { if(ptr) { delete (ptr); (ptr)=NULL; } }
#define SAFE_DELETE_ARRAY(ptr) { if(ptr) { delete[] (ptr); (ptr)=NULL; } }
#define SAFE_RELEASE(ptr)      { if(ptr) { (ptr)->Release(); (ptr)=NULL; } }

#define SCREENWIDTH 1024
#define SCREENHEIGHT 768

#ifndef VERSION
#define VERSION 0.7
#define VERSTRING "0.7"
#endif


namespace CGE{
class AppWindow;
class Renderer;
class Font;
class Forms;
}

namespace Gui{
class GuiElement;
class InputField;
class Button;
class Console;
}

namespace CGE{
class Simulator;
}

//! changes an integer to a string
inline string toStr(const unsigned i){
  char tmp[16];
  memset(tmp, 0, 16);
  sprintf(tmp, "%d", i);
  return string(tmp);
}

//! changes an integer to a string
inline string toStr(const int i){
  char tmp[16];
  memset(tmp, 0, 16);
  sprintf(tmp, "%d", i);
  return string(tmp);
}

//! changes a string to an integer
inline const int toInt(const string& s){
  return atoi(s.c_str());
}

//! changed a float to a string
inline string toStr(const float f){
  char tmp[32];
  memset(tmp, 0, 32);
  sprintf(tmp, "%f", f);
  return string(tmp);
}

//! changes a string to a float
inline float toFloat(const string& s){
  return (float)atof(s.c_str());
}

//! changes a string to lowercase
inline const string toLower(string& s){
  for (unsigned i = 0; i < s.length(); i++){
    s.at(i) = tolower(s.at(i));
  }
  return s;
}

inline const string toLower(const string& s){
  string ret;
  for (unsigned i = 0; i < s.length(); i++){
    ret += tolower(s.at(i));
  }
  return ret;
}

#ifdef UNIX
#include <unistd.h>
#include <sys/times.h>

#endif
double getTime();

#include "thread.h"

namespace CGE{

typedef void(*USERMAINFUNC)(int argc, char** argv);

class Engine{
  public:
    static int mainLoop(int argc, char** argv, USERMAINFUNC func);
    static void init();
    inline static Engine* instance() {return eng;}
    void startup(int argc, char** argv);
    bool startupRenderer(const std::string& renderer);
    void run();
    void shutdown();
    inline CGE::Renderer* getRenderer() {return rend_;}
    inline CGE::Font* getFont(int idx) {return fnt_[idx];}
    //! Get the standard forms
    inline CGE::Forms* getForms() {return forms_;}
    //! get the window
    inline CGE::AppWindow* getWindow() {return win_;}
    //! get the console
    inline ::Gui::Console* getConsole() {return console_;}
    inline double getFrameInterval() {return frameInterval_;}
    inline int getFPS() {return fps_;}
   
    //! get the input field list
    //inline list< ::Gui::InputField*>& getInputFields() {return listeners_;}
    //! get the button list
    inline list< ::Gui::GuiElement*>& getGuiElements() {return guiElems_;}
    //! adds an input field for mouse click checking
    //void addInputListener(::Gui::InputField* in, bool immediate=true){if (immediate)listeners_.push_back(in);
    //  else newIn_.push_back(in);}
    //! adds a button for mouse click checking
    void addGuiListener(::Gui::GuiElement* element, bool immediate=true){if (immediate)guiElems_.push_back(element);
      else newBut_.push_back(element);}
    //! set the current input field that should be active
    void setActiveInput(::Gui::InputField* field);
    //! get the current input field that should be active
    inline ::Gui::InputField* getActiveInput() {return input_;}
    //! clears the input and button listeners
    void clearListeners(bool immediate=true);
    //! remove the input listener at position idx
    //void removeInputListener(int idx);
    //! remove the button listener at position idx
    void removeGuiListener(int idx, bool immediate=true);
    //! remove the button listener with a certain name
    void removeGuiListener(const string& name);
    //! remove all button listeners beginning at idx
    void removeGuiListeners(int idx);
    //! get a button by name
    ::Gui::GuiElement* getGuiListener(const string& name);
    //! get the gui mutex
    Mutex& getGuiMutex() {return guitex_;}
    inline CGE::Simulator* getSimulator() {return mSimulator;}
    void requestShutdown();
    inline bool isShutdownRequested() {return mShutdownRequested;}
    void messageBox(const std::string& message, const std::string& title);
  private:
    static Engine* eng;
    Engine();

    //! The window
    CGE::AppWindow* win_;
    
    //! The renderer
    CGE::Renderer* rend_;
    //! The fonts
    CGE::Font** fnt_;
    //! Stadard forms
    CGE::Forms* forms_;
    //! is the engine initialized and running?
    bool isUp_;
    //! graphics subsystem
    bool graphics_;
    //! max framerate
    /*! 0 means time based rendering,
        >0 is maximum frame rate*/
    int maxFramerate_;
    //! the frame interval
    double frameInterval_;
    //! the time of the previous frame
    double frameTime_;
    //! the time of the previous second
    double lastTime_;
    //! fps-counter
    int fpscounter_;
    //! fps
    int fps_;

    //! physics subsystem
    bool physics_;
    //! the simulator
    CGE::Simulator* mSimulator;

    //! waiting input fields
    //list< ::Gui::InputField*> listeners_;
    //! waiting buttons
    list< ::Gui::GuiElement*> guiElems_;
    //! the new input fields
    //list< ::Gui::InputField*> newIn_;
    //! the new buttons
    list< ::Gui::GuiElement*> newBut_;
    //! if the input listeners should be cleared
    bool clear_;
    //! which buttons should be removed
    std::vector<short> remBut_;
    //! where should the keyboard input go to?
    ::Gui::InputField* input_;
    //! the game console
    ::Gui::Console* console_;
    //! a gui lock mutex
    Mutex guitex_;
    bool mShutdownRequested;
};
}

#endif
