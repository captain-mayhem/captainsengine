#ifndef NATIVE_LINUX_H
#define NATIVE_LINUX_H

#include <cge_export.h>

#if defined(LINUX)

#include <X11/Xlib.h>
#include "window.h"

namespace CGE{

class CGE_API X11Window : public AppWindow{
public:
  X11Window(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc);
  ~X11Window() {}
  //! init the window
  void init(const std::string& name) override;
  //! get display
  inline Display* getDisplay() const {return disp_;}
  //! get window
  inline Window getWindow() const {return win_;}
  //! get screen
  inline int getScreen() const {return screen_;}
  //! the input context for typed characters (Xutf8LookupString), nullptr if
  //! there is no input method
  inline XIC getInputContext() const {return ic_;}
  Gfx::NativeWindow getNativeWindow() const override;
  //! the desktop's font DPI (Xft.dpi) / 96
  float getContentScale() const override;
  //! kill the window
  void kill() override;
  
  virtual void changeSize(int width, int height) override;
  //! asks the window manager for fullscreen (_NET_WM_STATE_FULLSCREEN)
  void setFullscreen(bool fullscreen) override;
  
  virtual void show(bool doit) override;
  void setTitle(const std::string& title) override;
  //! is the event the window manager's request to close the window?
  bool isCloseRequest(const XEvent& event) const;

  //! the clipboard (the CLIPBOARD selection): the window owns the text
  //! until another client takes the selection
  void setClipboardText(const std::string& text);
  //! the clipboard's text as UTF-8; asks its owner and waits up to a
  //! second for the answer. False if there is none.
  bool getClipboardText(std::string& text);
  //! does a client own the clipboard?
  bool hasClipboardText() const;
  //! answers other clients' requests for the clipboard that the window owns
  /*! Returns true if the event was one of the selection's events. */
  bool handleSelectionEvent(const XEvent& event);
protected:
  //! asks the clipboard's owner for its text as target and waits for it
  bool convertClipboard(Atom target, std::string& data, Atom& type);
  //! waits up to a second for an event of the window
  bool waitForEvent(int type, XEvent& event);
  //! the window's size in pixels for its windowed size, within the screen
  void windowedPixels(int& width, int& height) const;
  //! the window handle
  Window win_;
  //! the display
  Display* disp_;
  //! the screen number
  int screen_;
  //! the content scale, 0 until it is read
  mutable float contentScale_;
  //! the input method and the window's input context
  XIM im_;
  XIC ic_;
  //! WM_PROTOCOLS and WM_DELETE_WINDOW
  Atom wmProtocols_;
  Atom wmDelete_;
  //! CLIPBOARD, UTF8_STRING, TARGETS, TEXT, INCR and the property the
  //! clipboard's owner writes into
  Atom clipboard_;
  Atom utf8String_;
  Atom targets_;
  Atom text_;
  Atom incr_;
  Atom clipboardProperty_;
  //! the copied text while the window owns the clipboard
  std::string clipboardText_;
  bool ownsClipboard_;
};

}

#endif

#ifdef WIN32
#include "window.h"
namespace CGE{
class CGE_API X11Window : public AppWindow{
public:
  X11Window(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc) {}
  ~X11Window() {}
};
}
#endif

#endif

