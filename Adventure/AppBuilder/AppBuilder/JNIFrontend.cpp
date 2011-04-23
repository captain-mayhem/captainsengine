
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
}

AdvDocument* adoc;
static bool shouldQuit = false;
CommandReceiver receiver;

void quit(){
  shouldQuit = true;
}

JNIEXPORT void JNICALL Java_de_captain_online_AdventureLib_init(JNIEnv * env, jobject obj,  jstring filename){
	const char* str = env->GetStringUTFChars(filename, NULL);
	if (str == NULL)
		return;
		
	__android_log_print(ANDROID_LOG_INFO, "libadventure", "native lib init");
	
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
