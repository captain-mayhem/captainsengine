#ifndef CGE_USERDIRS_H
#define CGE_USERDIRS_H

#include <cge_export.h>

#include <string>

namespace CGE{

//! The folders where an application keeps the user's data and configuration
/*! | Platform | data                      | config                      |
    |----------|---------------------------|-----------------------------|
    | Windows  | %APPDATA%\<app>           | %APPDATA%\<app>             |
    | Linux    | $XDG_DATA_HOME/<app>      | $XDG_CONFIG_HOME/<app>      |
    |          | (~/.local/share/<app>)    | (~/.config/<app>)           |
    | Android  | the app's internalDataPath (both)                       |
    | Web      | /IDBFS/<app>, kept in the browser's IndexedDB (both)    |

    <app> is appName from engine.ini, by default the executable's name. The
    folders are created when they're asked for. The paths are absolute, use
    the platform's native encoding for narrow strings (the ANSI code page on
    Windows) and have no trailing separator.

    On the web, files written there only reach IndexedDB with commit(), so
    applications call it after saving. It does nothing on the other platforms. */
class CGE_API UserDirs{
public:
  //! sets the application name used in the folder names; call it before the first query
  static void setAppName(const std::string& name);
  //! the application name used in the folder names
  static const std::string& getAppName();
  //! the folder for saved games, profiles and other user data
  static std::string getDataDir();
  //! the folder for configuration files (the settings store)
  static std::string getConfigDir();
  //! makes written files persistent (web: syncs the IDBFS mount to IndexedDB)
  static void commit();
};

}

#endif
