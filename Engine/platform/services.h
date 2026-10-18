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
    | clipboard    | yes     | Phase 1c    | Phase 1c| Phase 1c|
    | cursors      | yes     | yes         | -       | Phase 1c|
    | open URL     | yes     | xdg-open    | intent  | new tab |
    | open file    | yes     | xdg-open    | -       | -       |

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

  //! opens a URL in the user's browser
  static bool openUrl(const std::string& url);
  //! opens a file with the application the system associates with it
  static bool openFile(const std::string& path);
};

}

#endif
