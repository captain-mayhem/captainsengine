
#include <jni.h>
#include <stdlib.h>
#include <android/log.h>

#include <system/engine.h>
#include <io/TraceManager.h>

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"
#include "Renderer.h"
#include "CmdReceiver.h"

using namespace adv;

TR_CHANNEL(ADV_JNIFrontend);

static int winwidth = 640;
static int winheight = 480;

extern "C"{
JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_init(JNIEnv * env, jobject obj,  jstring filename);
JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_render(JNIEnv* env, jobject obj, int time);
JNIEXPORT jstring JNICALL Java_com_example_hellojni_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz );

class AndroidLogOutputter : public CGE::TraceOutputter{
  virtual bool init() {}
  virtual void trace(unsigned channel, int level, const char* function, const char* message){
	int prio;
	switch(level){
		case TRACE_DEBUG_DETAIL:
			prio = ANDROID_LOG_VERBOSE;
			break;
		case TRACE_DEBUG:
			prio = ANDROID_LOG_DEBUG;
			break;
		case TRACE_INFO:
			prio = ANDROID_LOG_INFO;
			break;
		case TRACE_WARNING:
			prio = ANDROID_LOG_WARN;
			break;
		case TRACE_ERROR:
			prio = ANDROID_LOG_ERROR;
			break;
	}
    __android_log_print(prio, "adventure", "%s (%s)", message, function);
  }
};

AdvDocument* adoc;
static bool shouldQuit = false;
CommandReceiver receiver;

JNIEXPORT jstring JNICALL Java_com_example_hellojni_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz ){
    return env->NewStringUTF("Hello from C++!");
}

void quit(){
  shouldQuit = true;
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_init(JNIEnv * env, jobject obj,  jstring filename){
	TR_USE(ADV_JNIFrontend);
	const char* str = env->GetStringUTFChars(filename, NULL);
	if (str == NULL)
		return;
	
	AndroidLogOutputter* alo = new AndroidLogOutputter();
	CGE::TraceManager::instance()->setTraceOutputter(alo);
	TR_INFO("native lib init, trying to load %s", str);
	
	adoc = new AdvDocument();
	if (!adoc->loadDocument(str)){
		TR_ERROR("failed to load adventure");
		return;
	}
	
	TR_DEBUG("init adventure engine");
	Engine::init();
	Engine::instance()->setData(adoc);
	SoundEngine::init();
	SoundEngine::instance()->setData(adoc);

	TR_DEBUG("init renderer");
	AdvRenderer::init();

	GL()matrixMode(MM_PROJECTION);
	GL()loadIdentity();
	GL()ortho(0, 640, 480, 0, -1.0, 1.0);
	//glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

	GL()matrixMode(MM_MODELVIEW);
	glClearColor(0.0,0.0,0.0,1.0);
	GL()color4ub(255,255,255,255);

	glDisable(GL_DEPTH_TEST);
	GL()enableClientState(ATTR_VERTEX_ARRAY);
	GL()enableClientState(ATTR_TEXTURE_COORD_ARRAY);
	GL()enable(GL_TEXTURE_2D);
	//glAlphaFunc(GL_GREATER, 0);
	//glEnable(GL_ALPHA_TEST);
	//glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	TR_INFO("Setting viewort to %i x %i", winwidth, winheight);
	glViewport(0, 0, winwidth, winheight);

	TR_DEBUG("init remote server");
	receiver.start();
	TR_DEBUG("init game");
	Engine::instance()->initGame(quit);
	TR_DEBUG("init done");
	
	env->ReleaseStringUTFChars(filename, str);
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_render(JNIEnv* env, jobject obj, int time){
	//CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  GL()matrixMode(MM_PROJECTION);
  GL()loadIdentity();
  GL()ortho(0, 640, 480, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  GL()matrixMode(MM_MODELVIEW);
  glDisable(GL_DEPTH_TEST);
  //glEnableClientState(GL_VERTEX_ARRAY);
  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  GL()enable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()loadIdentity();

  receiver.processCommands();
  Engine::instance()->render(time);

  SoundEngine::instance()->update(time);
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_move(JNIEnv* env, jobject obj, int x, int y){
	if (x >= 0 && x <= 640 && y >= 0 && y <= 480)
		Engine::instance()->setCursorPos(Vec2i(x,y));
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_leftclick(JNIEnv* env, jobject obj, int x, int y){
	Engine::instance()->leftClick(Vec2i(x,y));
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_setWindowDims(JNIEnv* env, jobject obj, int x, int y){
	winwidth = x;
	winheight = y;
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_keydown(JNIEnv* env, jobject obj, int keycode){
	Engine::instance()->keyPress(keycode);
	Engine::instance()->keyRelease(keycode);
}

}
