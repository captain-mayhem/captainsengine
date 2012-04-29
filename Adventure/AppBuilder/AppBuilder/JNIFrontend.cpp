
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

static int winwidth = 0;
static int winheight = 0;
static int advwidth = 0;
static int advheight = 0;
static int realwidth = 0;
static int realheight = 0;

extern "C"{
//JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_init(JNIEnv * env, jobject obj,  jstring filename);
//JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_render(JNIEnv* env, jobject obj, int time);
//JNIEXPORT jstring JNICALL Java_com_example_hellojni_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz );

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
    __android_log_print(prio, function, "%s (channel %i)", message, channel);
  }
};

AdvDocument* adoc;
static bool shouldQuit = false;
CommandReceiver receiver;

void quit(){
  shouldQuit = true;
}

void setAdventureDims(JNIEnv* env, int x, int y){
	TR_USE(ADV_JNIFrontend);
	jclass cls = env->FindClass("de.captain.online.AdventureLib");
	if (cls == NULL){
		TR_ERROR("class de.captain.online.AdventureLib not found");
		return;
	}
	jmethodID func = env->GetStaticMethodID(cls, "adventureSize", "(II)V");
	if (func == 0){
		TR_ERROR("method adventureSize not found");
	}
	else
		env->CallStaticVoidMethod(cls, func, x, y);
}

JNIEXPORT jboolean JNICALL Java_de_captain_online_AdventureLib_init(JNIEnv * env, jobject obj,  jstring filename){
	TR_USE(ADV_JNIFrontend);
	const char* str = env->GetStringUTFChars(filename, NULL);
	if (str == NULL)
		return JNI_FALSE;
	
	AndroidLogOutputter* alo = new AndroidLogOutputter();
	CGE::TraceManager::instance()->setTraceOutputter(alo);
	TR_INFO("native lib init, trying to load %s", str);
	
	adoc = new AdvDocument();
	if (!adoc->loadDocument(str)){
		TR_ERROR("failed to load adventure");
		return JNI_FALSE;
	}
	advwidth = adoc->getProjectSettings()->resolution.x;
	advheight = adoc->getProjectSettings()->resolution.y;
	setAdventureDims(env, advwidth, advheight);
	
	TR_DEBUG("init adventure engine");
	Engine::init();
	Engine::instance()->setData(adoc);
	SoundEngine::init();
	SoundEngine::instance()->setData(adoc);

	TR_DEBUG("init renderer");
	AdvRenderer::init();

	GL()matrixMode(MM_PROJECTION);
	GL()loadIdentity();
	GL()ortho(0, advwidth, advheight, 0, -1.0, 1.0);
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

	realwidth = winwidth;
	realheight = winheight;
	float aspect = advwidth/(float)advheight;
	if (aspect > 1.0f){
		realheight = winwidth/aspect;
		if (realheight > winheight){
			realheight = winheight;
			realwidth = winheight*aspect;
		}
	}
	else if (aspect < 1.0f){
		realwidth = winheight*aspect;
		if (realwidth > winwidth){
			realwidth = winwidth;
			realheight = winwidth*aspect;
		}
	}
	TR_INFO("Setting viewort to %i x %i", realwidth, realheight);
	glViewport(0, 0, realwidth, realheight);

	TR_DEBUG("init remote server");
	receiver.start();
	TR_DEBUG("init game");
	Engine::instance()->initGame(quit);
	TR_DEBUG("init done");
	
	env->ReleaseStringUTFChars(filename, str);
	return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_render(JNIEnv* env, jobject obj, int time){
	//CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  GL()matrixMode(MM_PROJECTION);
  GL()loadIdentity();
  GL()ortho(0, advwidth, advheight, 0, -1.0, 1.0);
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
  //time = (unsigned)(CGE::Engine::instance()->getFrameInterval()*1000);
  Engine::instance()->render(time);

  SoundEngine::instance()->update(time);
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_move(JNIEnv* env, jobject obj, int x, int y){
	if (x >= 0 && x <= advwidth && y >= 0 && y <= advheight)
		Engine::instance()->setCursorPos(Vec2i(x,y));
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_leftclick(JNIEnv* env, jobject obj, int x, int y){
	Engine::instance()->leftClick(Vec2i(x,y));
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_leftrelease(JNIEnv* env, jobject obj, int x, int y){
	Engine::instance()->leftRelease(Vec2i(x,y));
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
