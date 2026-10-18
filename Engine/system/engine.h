#ifndef CGE_ENGINE_H
#define CGE_ENGINE_H

#include <cge_export.h>

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

#ifdef ANDROID
#include <android_native_app_glue.h>
#endif

CGE_API double getTime();

#include "thread.h"
#include "application.h"
#include "virtualscreen.h"

namespace CGE{

class CGE_API Engine{
  public:
    static void init();
    inline static Engine* instance() {return eng;}
    void startup(int argc, char** argv);
    bool startupRenderer(const std::string& renderer);
    //! runs one frame: processes the window events, calls the application's
    //! update as often as needed, renders and waits for the frame limit
    void runFrame();
    //! processes the window events and renders one frame without calling update
    /*! For applications that wait for input in a nested loop of their own,
        started from update or init. Must not be called from render or from an
        input handler. Returns false once a shutdown is requested. */
    bool pumpFrame();
    void shutdown();
    //! the application, or nullptr while it isn't initialised
    inline IApplication* getApplication() {return app_;}
    //! the window was resized to width x height pixels
    void resize(int width, int height);
    //! the application goes into the background: no updates and no rendering
    void pause();
    //! the application comes back from the background
    void resume();
    inline bool isPaused() {return paused_;}
    inline CGE::Renderer* getRenderer() {return rend_;}
    //! the virtual resolution of the GUI pass and the input events
    /*! Its window size is updated from the window on every call. */
    CGE::VirtualScreen& getVirtualScreen();
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
    void requestShutdown();
    inline bool isShutdownRequested() {return mShutdownRequested;}
    void messageBox(const std::string& message, const std::string& title);

#ifdef ANDROID
    android_app* getApp(){
      return m_app;
    }
#endif
  private:
    friend CGE_API int runApplication(int argc, char** argv, IApplication& app, void* platformData);
    static Engine* eng;
    Engine();
    //! processes the pending window events
    void processEvents();
    //! one frame, with or without updating the application
    void frame(bool update);
    //! renders the application and the GUI and swaps buffers
    void renderFrame();
    //! applies the deferred changes to the GUI element list
    void updateGuiElements();
    //! sleeps until the frame limit (maxFramerate in engine.ini) is reached
    void limitFrame();

    //! The window
    CGE::AppWindow* win_;
    
    //! The renderer
    CGE::Renderer* rend_;
    //! the virtual resolution
    CGE::VirtualScreen virtual_;
    //! The fonts
    CGE::Font** fnt_;
    //! Stadard forms
    CGE::Forms* forms_;
    //! is the engine initialized and running?
    bool isUp_;
    //! graphics subsystem
    bool graphics_;
    //! the application
    IApplication* app_;
    //! is the application in the background?
    bool paused_;
    //! is a frame being rendered? (pumpFrame isn't allowed then)
    bool rendering_;
    //! max framerate (0 means unlimited)
    int maxFramerate_;
    //! the fixed update interval
    double updateInterval_;
    //! the time that the updates still have to catch up with
    double updateLag_;
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
    
#ifdef LINUX
    //! the time of the last click, to detect double clicks
    double lastClick_;
#endif
#ifdef ANDROID
	static int32_t handleInput(struct android_app* app, AInputEvent* event);
	static void handleCmd(struct android_app* app, int32_t cmd);
    struct android_app* m_app;
#endif
};
}

#endif