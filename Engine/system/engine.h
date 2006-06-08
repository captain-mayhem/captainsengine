#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <fstream>
#include <string>

using std::string;
using std::ofstream;
using std::cerr;

#define EXIT() (::System::Engine::instance()->shutdown())
#define EXIT2(msg) {::System::Log << (msg) << "\n"; ::System::Engine::instance()->shutdown(); }
#define SAFE_DELETE(ptr)       { if(ptr) { delete (ptr); (ptr)=NULL; } }
#define SAFE_DELETE_ARRAY(ptr) { if(ptr) { delete[] (ptr); (ptr)=NULL; } }
#define SAFE_RELEASE(ptr)      { if(ptr) { (ptr)->Release(); (ptr)=NULL; } }

namespace Windows{
class AppWindow;
}

namespace Graphics{
class Renderer;
class Font;
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
    inline ::Windows::AppWindow* getWindow() {return win_;}
    inline double getFrameInterval() {return frameInterval_;}
    inline int getFPS() {return fps_;}
  private:
    static Engine* eng;
    Engine();
    ::Windows::AppWindow* win_;
    ::Graphics::Renderer* rend_;
    ::Graphics::Font* fnt_;
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
};
}

#endif
