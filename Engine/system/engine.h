#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>

using std::string;
using std::ofstream;
using std::cerr;
using std::list;
using std::vector;

#define EXIT() (::System::Engine::instance()->shutdown())
#define EXIT2(msg) {::System::Log << (msg) << "\n"; ::System::Engine::instance()->shutdown(); }
#define SAFE_DELETE(ptr)       { if(ptr) { delete (ptr); (ptr)=NULL; } }
#define SAFE_DELETE_ARRAY(ptr) { if(ptr) { delete[] (ptr); (ptr)=NULL; } }
#define SAFE_RELEASE(ptr)      { if(ptr) { (ptr)->Release(); (ptr)=NULL; } }

#define SCREENWIDTH 1024
#define SCREENHEIGHT 768

#ifndef VERSION
#define VERSION 0.6
#define VERSTRING "0.6"
#endif

namespace Windows{
class AppWindow;
}

namespace Graphics{
class Renderer;
class Font;
class Forms;
}

namespace Gui{
class InputField;
class Button;
class Console;
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

//! changes a string to lowercase
inline const string toLower(string& s){
  for (unsigned i = 0; i < s.length(); i++){
    s.at(i) = tolower(s.at(i));
  }
  return s;
}

#ifdef UNIX
#include <unistd.h>
#include <sys/times.h>

long GetTickCount();
#endif

namespace System{

extern ofstream Log;

class Engine{
  public:
    static void init();
    inline static Engine* instance() {return eng;}
    void startup(int argc, char** argv);
    void run();
    void shutdown();
    inline ::Graphics::Renderer* getRenderer() {return rend_;}
    inline ::Graphics::Font* getFont() {return fnt_;}
    //! Get the standard forms
    inline ::Graphics::Forms* getForms() {return forms_;}
    //! get the window
    inline ::Windows::AppWindow* getWindow() {return win_;}
    //! get the console
    inline ::Gui::Console* getConsole() {return console_;}
    inline double getFrameInterval() {return frameInterval_;}
    inline int getFPS() {return fps_;}
   
    //! get the input field list
    inline list< ::Gui::InputField*>& getInputFields() {return listeners_;}
    //! get the button list
    inline list< ::Gui::Button*>& getButtons() {return buttons_;}
    //! adds an input field for mouse click checking
    void addInputListener(::Gui::InputField* in, bool immediate=true){if (immediate)listeners_.push_back(in);
      else newIn_.push_back(in);}
    //! adds a button for mouse click checking
    void addButtonListener(::Gui::Button* button, bool immediate=true){if (immediate)buttons_.push_back(button);
      else newBut_.push_back(button);}
    //! set the current input field that should be active
    void setActiveInput(::Gui::InputField* field);
    //! get the current input field that should be active
    inline ::Gui::InputField* getActiveInput() {return input_;}
    //! clears the input and button listeners
    void clearListeners(bool immediate=true);
    //! remove the input listener at position idx
    void removeInputListener(int idx);
    //! remove the button listener at position idx
    void removeButtonListener(int idx, bool immediate=true);
    //! remove all button listeners beginning at idx
    void removeButtonListeners(int idx);
  private:
    static Engine* eng;
    Engine();
    //! The window
    ::Windows::AppWindow* win_;
    //! The renderer
    ::Graphics::Renderer* rend_;
    //! The font
    ::Graphics::Font* fnt_;
    //! Stadard forms
    ::Graphics::Forms* forms_;
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
    //! waiting input fields
    list< ::Gui::InputField*> listeners_;
    //! waiting buttons
    list< ::Gui::Button*> buttons_;
    //! the new input fields
    list< ::Gui::InputField*> newIn_;
    //! the new buttons
    list< ::Gui::Button*> newBut_;
    //! if the input listeners should be cleared
    bool clear_;
    //! which buttons should be removed
    std::vector<short> remBut_;
    //! where should the keyboard input go to?
    ::Gui::InputField* input_;
    //! the game console
    ::Gui::Console* console_;
};
}

#endif
