#ifndef CGE_SERVICES_H
#define CGE_SERVICES_H

#include <cge_export.h>

#include <cstdint>
#include <string>

namespace CGE{

//! Small operating system services
/*! What exists on which platform:

    | Service      | Windows | Linux (X11) | Android | Web     |
    |--------------|---------|-------------|---------|---------|
    | tick clock   | yes     | yes         | yes     | yes     |
    | clipboard    | yes     | yes (1)     | yes (2) | yes (3) |
    | cursors      | yes     | yes         | yes (2) | yes     |
    | text input   | -       | -           | yes (2) | yes     |
    | open URL     | yes     | xdg-open    | intent  | new tab |
    | open file    | yes     | xdg-open    | -       | -       |

    (1) The CLIPBOARD selection. The window owns the copied text, so it is
    gone when the application quits, unless a clipboard manager took it.
    (2) With the engine's CgeActivity (Engine/android/java); a plain
    NativeActivity has none of them. Cursors are for a mouse.
    (3) The web reads the last text copied or pasted in the page (Ctrl+V
    brings the system clipboard's text with it), not the clipboard itself.

    Where a service is missing, the functions do nothing and return false or
    an empty result. Text (clipboard, URLs) is UTF-8; file paths use the
    engine's narrow encoding (the ANSI code page on Windows). */
class CGE_API Platform{
public:
  //! milliseconds on a steady clock; only differences are meaningful
  static uint64_t getTickMs();

  //! replaces the clipboard's text
  static bool setClipboardText(const std::string& text);
  //! the clipboard's text, empty if it holds none
  static std::string getClipboardText();
  //! does the clipboard hold text (cheaper than getClipboardText where possible)
  static bool hasClipboardText();

  enum Cursor{
    CURSOR_ARROW,
    CURSOR_IBEAM,
    CURSOR_HAND,
    CURSOR_WAIT,
    CURSOR_CROSSHAIR,
    CURSOR_RESIZE_H,
    CURSOR_RESIZE_V,
    CURSOR_MOVE,
    CURSOR_FORBIDDEN,
  };
  //! sets the mouse cursor over the application's window, immediately
  /*! It takes effect at once, also during a long operation that blocks the
      main loop (the wait cursor). Hiding the mouse (Input::Mouse::showCursor)
      takes precedence. */
  static void setCursor(Cursor cursor);
  //! the cursor set last
  static Cursor getCursor();

  //! shows the on-screen keyboard, for typing into a text field
  /*! The typed text arrives as from a hardware keyboard: characters
      (IApplication::onChar) and the keys backspace, delete and enter. ImGui's
      platform backend calls it while one of its text fields is active, and
      again for a tap while it stays active, which brings back a keyboard the
      user closed (Android; on the web where the browser has the
      VirtualKeyboard API). On the web the keyboard only opens shortly after a
      tap or a click (a browser rule). Where there is no on-screen keyboard,
      only isTextInputActive changes. */
  static void startTextInput();
  //! hides the on-screen keyboard
  static void stopTextInput();
  //! between startTextInput and stopTextInput
  static bool isTextInputActive();

  //! opens a URL in the user's browser
  static bool openUrl(const std::string& url);
  //! opens a file with the application the system associates with it
  static bool openFile(const std::string& path);
};

}

#endif
