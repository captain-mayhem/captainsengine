
#include <jni.h>
#include <stdlib.h>
#include <android/log.h>

#include <system/engine.h>

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"
#include "Renderer.h"
#include "CmdReceiver.h"

extern "C"{
JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_init(JNIEnv * env, jobject obj,  jstring filename);
JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_render(JNIEnv* env, jobject obj, int time);
JNIEXPORT jstring JNICALL Java_com_example_hellojni_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz );
}

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
	const char* str = env->GetStringUTFChars(filename, NULL);
	if (str == NULL)
		return;
		
	__android_log_print(ANDROID_LOG_INFO, "libadventure", "native lib init, trying to load %s", str);
	
	adoc = new AdvDocument();
	if (!adoc->loadDocument(str)){
	return;
	}
	Engine::init();
	Engine::instance()->setData(adoc);
	SoundEngine::init();
	SoundEngine::instance()->setData(adoc);

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

	glViewport(0, 0, 640, 480);

	receiver.start();
	Engine::instance()->initGame(quit);
	
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

  SoundEngine::instance()->update();
}
