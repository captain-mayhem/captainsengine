#ifndef CGE_APPLICATION_H
#define CGE_APPLICATION_H

#include <cge_export.h>

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef ANDROID
#include <android_native_app_glue.h>
#endif

namespace CGE{

//! The interface between the engine and an application
/*! An application derives from it, overrides what it needs and hands an
    instance to CGE_MAIN. The engine owns the main loop and calls these
    functions; all of them have empty defaults except onKeyDown. */
class CGE_API IApplication{
public:
  virtual ~IApplication();
  //! called once the window, the renderer, the fonts and the console exist
  /*! A failing application calls Engine::requestShutdown(). */
  virtual void init(int argc, char** argv) {}
  //! advances the application by dt seconds
  /*! Called with a fixed dt (updateRate in engine.ini) as often as needed to
      catch up with real time, before each render. */
  virtual void update(double dt) {}
  //! renders one frame; the engine renders its GUI on top and swaps buffers
  virtual void render() {}
  //! called once before the engine shuts down, while the renderer still exists
  virtual void shutdown() {}
  //! the window was resized to width x height pixels
  virtual void onResize(int width, int height) {}
  //! the application went into the background (minimised, or lost focus on Android)
  /*! update and render aren't called until onResume. */
  virtual void onPause() {}
  //! the application came back from the background
  virtual void onResume() {}

  //! a key was pressed; the default quits on Escape
  virtual void onKeyDown(int key);
  //! a key was released
  virtual void onKeyUp(int key) {}
  //! a printable character was typed
  virtual void onChar(unsigned char ch) {}
  //! a mouse button was pressed at (x,y)
  /*! All mouse positions are in virtual coordinates (Engine::getVirtualScreen()). */
  virtual void onMouseDown(int x, int y, int button) {}
  //! a mouse button was released at (x,y)
  virtual void onMouseUp(int x, int y, int button) {}
  //! a mouse button was double-clicked at (x,y)
  virtual void onDoubleClick(int x, int y, int button) {}
  //! the mouse moved to (x,y)
  virtual void onMouseMove(int x, int y, int buttons) {}
  //! the mouse wheel turned by delta notches at (x,y)
  virtual void onMouseWheel(int x, int y, int delta) {}
};

//! starts the engine with app and runs the main loop until the application quits
/*! platformData is the android_app on Android and unused elsewhere.
    Applications don't call this directly, but use CGE_MAIN. */
CGE_API int runApplication(int argc, char** argv, IApplication& app, void* platformData = nullptr);

#ifdef _WIN32
//! the same, with the command line given to WinMain split into arguments
CGE_API int runApplication(const char* cmdline, IApplication& app);
#endif

}

//! Defines the platform's entry point (WinMain, main or android_main) of an
//! application in the application itself, which then runs the engine with an
//! instance of AppClass (derived from CGE::IApplication). Use it once in one
//! source file of the application.
#if defined(_WIN32)
#define CGE_MAIN(AppClass) \
  int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int){ \
    AppClass app; \
    return CGE::runApplication(cmdline, app); \
  }
#elif defined(ANDROID)
#define CGE_MAIN(AppClass) \
  extern "C" __attribute__((visibility("default"))) void android_main(struct android_app* state){ \
    static char arg0[] = "cge"; \
    char* argv[] = {arg0, nullptr}; \
    AppClass app; \
    CGE::runApplication(1, argv, app, state); \
  }
#else
#define CGE_MAIN(AppClass) \
  int main(int argc, char** argv){ \
    AppClass app; \
    return CGE::runApplication(argc, argv, app); \
  }
#endif

#endif
