#ifndef CGE_PLATFORM_INTERNAL_H
#define CGE_PLATFORM_INTERNAL_H

// Engine-internal hooks of the platform services; not part of the public API.

#include <string>
#include <vector>

#ifdef ANDROID
struct AAssetManager;
#endif

namespace CGE{

namespace PlatformInternal{

//! called by runApplication before the engine starts; androidApp is the
//! android_app on Android and null elsewhere (services.cpp)
void init(int argc, char** argv, void* androidApp);
//! applies the asset folder and the application name from engine.ini, if set (services.cpp)
void applyEngineSettings(const std::string& assetDir, const std::string& appName);
//! the folder of the running executable, empty where there is none (Android, the web) (services.cpp)
std::string getExecutableDir();
//! sets the current cursor again (Windows: WM_SETCURSOR; X11: when the mouse is shown again) (services.cpp)
void applyCursor();
//! shows cursor (a Platform::Cursor) instead of the application's one, -1 shows the application's again (services.cpp)
/*! For the ImGui platform backend while the mouse is over an ImGui window. */
void setOverrideCursor(int cursor);

//! the application name used when engine.ini has none (userdirs.cpp)
void setDefaultAppName(const std::string& name);
#ifdef ANDROID
//! the app's internal data folder (userdirs.cpp)
void setAndroidDataPath(const char* path);
//! the APK's assets (assets.cpp)
void setAssetManager(AAssetManager* mgr);
//! the application's arguments from the activity's intent (CgeActivity.EXTRA_ARGUMENTS), without argv[0] (services.cpp)
std::vector<std::string> getActivityArguments();
#endif

}

}

#endif
