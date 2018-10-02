
#include "engine.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"

extern void engineMain(int argc, char** argv);

TR_CHANNEL(CGE_Main);

#ifdef WIN32

int WINAPI WinMain(HINSTANCE instance, HINSTANCE oldinstance, LPTSTR cmdline, int cmdShow){
  //_CrtSetBreakAlloc(729);
  std::vector<std::string> args;
  std::vector<const char*> argv;
  args.push_back("Captains Game Engine");
#ifdef UNDER_CE
  char tmp[1024];
  wcstombs(tmp, cmdline, 1024);
  args.push_back(tmp);
#else
  int len = (int)strlen(cmdline);
  std::string curr;
  bool acceptWs = false;
  for (int i = 0; i < len; ++i){
    if (cmdline[i] == '"'){
      acceptWs = !acceptWs;
      continue;
    }
    if (!acceptWs && cmdline[i] == ' ' && !curr.empty()){
      args.push_back(curr);
      curr.clear();
    }
    else
      curr.append(1, cmdline[i]);
  }
  if (!curr.empty()){
    args.push_back(curr);
    curr.clear();
  }
#endif
  argv.resize(args.size());
  for (unsigned i = 0; i < args.size(); ++i){
    argv[i] = args[i].c_str();
  }
  return CGE::Engine::mainLoop((int)argv.size(), (char**)&argv[0], engineMain);
}
#endif

/*#include "../window/nativeLinux.h"
#include "../input/keyboard.h"
#include "../renderer/renderer.h"
#include "../renderer/font.h"
#include "../gui/gui.h"*/

#if defined(LINUX) && !defined(NO_X11)
#include <X11/Xlib.h>
#include <GL/glx.h>

int main(int argc, char** argv){
  return CGE::Engine::mainLoop(argc, argv, engineMain);
}
#endif

#ifdef ANDROID
#include <android_native_app_glue.h>

__attribute__((visibility("default")))
void android_main(struct android_app* app){
	TR_USE(CGE_Main);
  app_dummy();
  
  /*JNIEnv *env;
app->activity->vm->AttachCurrentThread(&env, 0);

jobject me = app->activity->clazz;

jclass acl = env->GetObjectClass(me); //class pointer of NativeActivity
jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");
jobject intent = env->CallObjectMethod(me, giid); //Got our intent

jclass icl = env->GetObjectClass(intent); //class pointer of Intent
jmethodID gseid = env->GetMethodID(icl, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");

jstring jsParam1 = (jstring) env->CallObjectMethod(intent, gseid, env->NewStringUTF("arguments"));
const char *param = env->GetStringUTFChars(jsParam1, 0);
TR_INFO("args %s\n", param);
//When done with it, or when you've made a copy
env->ReleaseStringUTFChars(jsParam1, param);

//Same for Param2*/
  
  int argc = 1;
  char* arg = "cge";
  char** argv = &arg;
  CGE::Engine::mainLoop(argc, argv, engineMain, app);
}

#endif