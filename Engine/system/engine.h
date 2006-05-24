#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <fstream>

using std::ofstream;
using std::cerr;

#define EXIT() (::Engine::Engine::instance()->shutdown())
#define EXIT2(msg) {::Engine::Log << (msg) << "\n"; ::Engine::Engine::instance()->shutdown(); }
#define SAFE_DELETE(ptr)       { if(ptr) { delete (ptr); (ptr)=NULL; } }
#define SAFE_DELETE_ARRAY(ptr) { if(ptr) { delete[] (ptr); (ptr)=NULL; } }
#define SAFE_RELEASE(ptr)      { if(ptr) { (ptr)->Release(); (ptr)=NULL; } }

namespace Windows{
class AppWindow;
}

namespace Renderer{
class Renderer;
}

namespace Engine{

extern ofstream Log;

class Engine{
  public:
    static void init();
    inline static Engine* instance() {return eng;}
    void startup();
    void run();
    void shutdown();
    inline ::Renderer::Renderer* getRenderer() {return rend_;}
    inline ::Windows::AppWindow* getWindow() {return win_;}
  private:
    static Engine* eng;
    Engine();
    ::Windows::AppWindow* win_;
    ::Renderer::Renderer* rend_;
};
}

#endif
