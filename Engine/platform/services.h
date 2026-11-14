#ifndef CGE_SERVICES_H
#define CGE_SERVICES_H

#include <cge_export.h>

#include <cstdint>
#include <string>
#include <vector>

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
    | languages    | yes     | yes (4)     | yes (5) | yes     |
    | file import  | dialog  | -           | yes (6) | picker  |
    | file export  | dialog  | -           | yes (6) | download|

    (1) The CLIPBOARD selection. The window owns the copied text, so it is
    gone when the application quits, unless a clipboard manager took it.
    (2) With the engine's CgeActivity (Engine/android/java); a plain
    NativeActivity has none of them. Cursors are for a mouse.
    (3) The web reads the last text copied or pasted in the page (Ctrl+V
    brings the system clipboard's text with it), not the clipboard itself.
    (4) From the environment: LANGUAGE (a list), then LC_ALL, LC_MESSAGES
    or LANG; nothing for the C and POSIX locales.
    (5) Only the first of the system's languages (AConfiguration).
    (6) The system's document picker (Storage Access Framework), through
    CgeActivity: the user's files, Downloads, and cloud drives.

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

  //! Moving files between the application's folders and the user's device
  /*! An import lets the user choose a file on their device and copies it
      into one of the application's folders (e.g. below
      UserDirs::getDataDir()); an export hands one of the application's
      files to the user. Where the application's folders aren't reachable
      by the user (the browser's IndexedDB, Android's internal storage),
      this is how files come in and go out.

      | Platform | import                       | export                        |
      |----------|------------------------------|-------------------------------|
      | Windows  | the open dialog              | the save dialog               |
      | Android  | ACTION_OPEN_DOCUMENT         | ACTION_CREATE_DOCUMENT        |
      | Web      | the browser's file picker    | a download                    |
      | Linux    | -                            | -                             |

      Both run asynchronously: they return at once, and pollTransfer()
      tells the outcome later (on Windows it is known when they return).
      One transfer runs at a time. The imported file gets the chosen file's
      name (importedName), and is committed (UserDirs::commit) before it is
      Done. A browser opens its file picker only shortly after a click, a
      tap or a key (its user activation): call importFile from that input's
      handler, or in the frame after it. On Android the application is
      paused while the system's picker is shown, like for any other
      activity in front of it. */
  enum TransferStatus{
    TRANSFER_NONE,       //!< no transfer was started
    TRANSFER_RUNNING,    //!< the user is choosing, or the file is copied
    TRANSFER_DONE,       //!< the file was imported or exported
    TRANSFER_CANCELLED,  //!< the user closed the picker or dialog
    TRANSFER_FAILED,     //!< reading or writing went wrong
  };
  //! whether importFile and exportFile can work here
  static bool canTransferFiles();
  //! lets the user choose a file and copies it into folder (made if it
  //! isn't there); accept lists the extensions that the picker offers
  //! (".zip,.txt"; empty: all). With replace, a file of the same name in
  //! the folder is replaced; otherwise the new one gets another name. False
  //! if it couldn't start (not here, another transfer runs, or the browser
  //! wants a new user activation).
  static bool importFile(const std::string& folder, const std::string& accept, bool replace);
  //! hands the file at path to the user, under its own name; false if it
  //! couldn't start
  static bool exportFile(const std::string& path);
  //! the last transfer's state; once it ended (DONE, CANCELLED, FAILED) it
  //! is told once, after that it is TRANSFER_NONE. For a DONE import, name
  //! gets the file's name in the folder (for an export, the file's name).
  static TransferStatus pollTransfer(std::string* name = nullptr);
  //! the name a file chosen as name gets in folder: '/', '\\' and control
  //! characters as '_' ("upload" if nothing is left); unless replace, a
  //! taken name gets " (2)", " (3)", ... before its extension
  static std::string importedName(const std::string& folder, const std::string& name, bool replace);

  //! opens a URL in the user's browser
  static bool openUrl(const std::string& url);
  //! opens a file with the application the system associates with it
  static bool openFile(const std::string& path);

  //! the user's languages, the preferred one first, as BCP 47 tags
  /*! A language and maybe a region, e.g. "de-DE", "en-US" or "fr"
      (Windows' and the browser's list, Linux's environment, Android's first
      language). Empty where the system says nothing (or only C). */
  static std::vector<std::string> getLanguages();
};

}

#endif
