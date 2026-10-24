#ifndef NATIVE_ANDROID_H
#define NATIVE_ANDROID_H

#include <cge_export.h>

#if defined(ANDROID)

#include "window.h"

struct android_app;
struct AInputEvent;

namespace CGE{

//! The activity's window (NativeActivity)
/*! The window comes and goes with the activity: the render device releases
    it when the application goes into the background and attaches the next
    one (see runApplication). Its size is checked every frame, since a
    rotation or a resize of a split screen changes it without an event the
    engine waits for. Touches and the mouse go to the engine's input and
    ImGui like the other platforms' events (the touches to Input::Touch).
    The back button is KEY_ESCAPE. The on-screen keyboard (CgeActivity's
    text input view) types as a hardware keyboard does. */
class CGE_API AndroidWindow : public AppWindow{
public:
  AndroidWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc);
  ~AndroidWindow() {}
  //! takes the activity's window
  void init(const std::string& name) override;
  //! kill the window
  void kill() override;
  //! the activity's current window
  Gfx::NativeWindow getNativeWindow() const override;
  //! the screen's density relative to 160 dpi
  float getContentScale() const override;
  //! takes the size of the activity's current window
  /*! Returns true if it changed. */
  bool updateSize();

  //! the input events of the activity (android_app::onInputEvent)
  /*! Returns 1 for the events the engine handled, 0 for those the system
      should handle (e.g. the volume keys). */
  static int32_t handleInput(android_app* app, AInputEvent* event);
  //! what CgeActivity reported on its UI thread since the last call: the
  //! back gestures and buttons (Android 13 and later, where back isn't a key
  //! event) as KEY_ESCAPE, and the on-screen keyboard's text and keys
  static void dispatchActivityEvents();
  //! lets CgeActivity (Engine/android/java) report back gestures and the
  //! on-screen keyboard to the native code; nothing happens for a plain
  //! NativeActivity
  static void registerNatives(android_app* app);
};

//! sends the engine's tracing to logcat (tag CGE)
CGE_API void installLogcatTracing();

}

#endif

#endif
