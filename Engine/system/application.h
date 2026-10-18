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

//! the application's main function, called by the engine after the window is created
typedef void(*USERMAINFUNC)(int argc, char** argv);

//! starts the engine, calls func and runs the main loop until the application quits
/*! platformData is the android_app on Android and unused elsewhere.
    Applications don't call this directly, but use CGE_MAIN. */
CGE_API int runApplication(int argc, char** argv, USERMAINFUNC func, void* platformData = nullptr);

#ifdef _WIN32
//! the same, with the command line given to WinMain split into arguments
CGE_API int runApplication(const char* cmdline, USERMAINFUNC func);
#endif

}

//! Defines the platform's entry point (WinMain, main or android_main) of an
//! application in the application itself, which then starts the engine with
//! the given main function. Use it once in one source file of the application.
#if defined(_WIN32)
#define CGE_MAIN(func) \
  int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int){ \
    return CGE::runApplication(cmdline, func); \
  }
#elif defined(ANDROID)
#define CGE_MAIN(func) \
  extern "C" __attribute__((visibility("default"))) void android_main(struct android_app* app){ \
    static char arg0[] = "cge"; \
    char* argv[] = {arg0, nullptr}; \
    CGE::runApplication(1, argv, func, app); \
  }
#else
#define CGE_MAIN(func) \
  int main(int argc, char** argv){ \
    return CGE::runApplication(argc, argv, func); \
  }
#endif

#endif
