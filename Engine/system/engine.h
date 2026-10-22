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
#include <array>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <io/Tracing.h>


#define EXIT() (CGE::Engine::instance()->requestShutdown())
#define EXIT2(msg, ...) {TR_ERROR(msg, ##__VA_ARGS__); CGE::Engine::instance()->requestShutdown(); }
#define SAFE_DELETE(ptr)       { if(ptr) { delete (ptr); (ptr)=nullptr; } }
#define SAFE_DELETE_ARRAY(ptr) { if(ptr) { delete[] (ptr); (ptr)=nullptr; } }
#define SAFE_RELEASE(ptr)      { if(ptr) { (ptr)->Release(); (ptr)=nullptr; } }

#ifndef VERSION
#define VERSION 0.7
#define VERSTRING "0.7"
#endif


namespace CGE{
class AppWindow;
class Renderer;
class Font;
class Forms;
class Console;
namespace Gfx{
class DrawContext;
}
}

//! changes a string to an integer
inline const int toInt(const std::string& s){
  return atoi(s.c_str());
}

//! changes a string to a float
inline float toFloat(const std::string& s){
  return (float)atof(s.c_str());
}

//! changes a string to lowercase
inline const std::string toLower(std::string& s){
  for (unsigned i = 0; i < s.length(); i++){
    s.at(i) = tolower(s.at(i));
  }
  return s;
}

inline const std::string toLower(const std::string& s){
  std::string ret;
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
#include "../renderer/renderdevice.h"

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
    inline CGE::Renderer* getRenderer() {return rend_.get();}
    //! the render device, which owns the window's graphics context
    /*! The Null device when engine.ini says graphics = false. */
    inline Gfx::RenderDevice* getRenderDevice() {return device_.get();}
    //! the engine's drawing state on the render device, for Forms, Mesh, Scene and the render techniques
    inline Gfx::DrawContext* getDrawContext() {return draw_.get();}
    //! the virtual resolution of the GUI pass and the input events
    /*! Its window size is updated from the window on every call. */
    CGE::VirtualScreen& getVirtualScreen();
    inline CGE::Font* getFont(int idx) {return fnt_[idx].get();}
    //! Get the standard forms
    inline CGE::Forms* getForms() {return forms_.get();}
    //! get the window
    inline CGE::AppWindow* getWindow() {return win_.get();}
    //! get the console
    inline CGE::Console* getConsole() {return console_.get();}
    //! is Dear ImGui set up? (not without graphics)
    /*! The engine draws ImGui on top of each frame: applications call ImGui's
        functions in their render() (see ui/imgui_impl_cge_platform.h). */
    inline bool hasImGui() const {return imgui_;}
    inline double getFrameInterval() {return frameInterval_;}
    inline int getFPS() {return fps_;}
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
    ~Engine();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    //! processes the pending window events
    void processEvents();
    //! one frame, with or without updating the application
    void frame(bool update);
    //! renders the application and the GUI and presents the frame
    void renderFrame();
    //! creates the ImGui context and its backends for the window and the device
    void startupImGui();
    void shutdownImGui();
    //! draws the ImGui frame the application and the console built
    void renderImGui();
    //! sleeps until the frame limit (maxFramerate in engine.ini) is reached
    void limitFrame();

    //! The window
    std::unique_ptr<CGE::AppWindow> win_;
    //! the render device; destroyed after the renderer
    std::unique_ptr<Gfx::RenderDevice> device_;
    //! drawing on the render device; destroyed before it
    std::unique_ptr<Gfx::DrawContext> draw_;

    //! The renderer
    std::unique_ptr<CGE::Renderer> rend_;
    //! the virtual resolution
    CGE::VirtualScreen virtual_;
    //! The fonts
    std::array<std::unique_ptr<CGE::Font>, 3> fnt_;
    //! Stadard forms
    std::unique_ptr<CGE::Forms> forms_;
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

    //! the game console
    std::unique_ptr<CGE::Console> console_;
    //! is ImGui set up?
    bool imgui_;
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