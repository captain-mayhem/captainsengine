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
  //! the application went into the background (minimised; on Android also
  //! without a window or without the focus, e.g. under the notifications)
  /*! update and render aren't called until onResume. */
  virtual void onPause() {}
  //! the application came back from the background
  virtual void onResume() {}
  //! the render device lost its context and all resources (Android)
  /*! The application deletes everything it created on the device, and
      forgets the engine's draw context, sprite batch, fonts and forms: they
      are created again, too. The GL objects are already gone, so nothing is
      drawn until onDeviceRestored. Applications that run on Android must
      handle this; Engine::restartRenderDevice tests it on any platform. */
  virtual void onDeviceLost() {}
  //! the render device works again: the application creates its resources again
  virtual void onDeviceRestored() {}

  //! a key was pressed; the default quits on Escape
  virtual void onKeyDown(int key);
  //! a key was released
  virtual void onKeyUp(int key) {}
  //! a printable character was typed, as a Unicode code point
  /*! Typed characters, not keys: they come with the keyboard layout, dead
      keys and the input method applied. */
  virtual void onChar(unsigned int ch) {}
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

  //! a finger touched the screen at (x,y) (Android and the web)
  /*! id is the finger's until it is lifted; positions are virtual
      coordinates. With Input::Touch's mouse emulation (the default) the first
      finger also makes mouse events, so applications that only need clicks
      can ignore the touch events. */
  virtual void onTouchDown(int id, int x, int y) {}
  //! a finger moved to (x,y)
  virtual void onTouchMove(int id, int x, int y) {}
  //! a finger was lifted at (x,y)
  virtual void onTouchUp(int id, int x, int y) {}
  //! the system took a finger over (e.g. for a gesture): it's gone, but it
  //! wasn't lifted, so it shouldn't count as a tap
  virtual void onTouchCancel(int id) {}
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
