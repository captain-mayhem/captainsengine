#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <fstream>

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
}

namespace System{

extern ofstream Log;

class Engine{
  public:
    static void init();
    inline static Engine* instance() {return eng;}
    void startup();
    void run();
    void shutdown();
    inline ::Graphics::Renderer* getRenderer() {return rend_;}
    inline ::Windows::AppWindow* getWindow() {return win_;}
  private:
    static Engine* eng;
    Engine();
    ::Windows::AppWindow* win_;
    ::Graphics::Renderer* rend_;
    //! is the engine initialized and running?
    bool isUp_;
};
}

#endif
