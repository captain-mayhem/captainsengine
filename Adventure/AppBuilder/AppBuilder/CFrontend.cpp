#include <system/engine.h>
#include <io/TraceManager.h>
#include <SDL/SDL.h>
#include <emscripten.h>
#include <input/keyboard.h>

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"
#include "Renderer.h"
#include "CmdReceiver.h"

#define CEXPORT __attribute__ ((visibility ("default")))

TR_CHANNEL_LVL(Frontend, TRACE_DEBUG);

using namespace adv;
extern "C"{

AdvDocument* adoc;
CommandReceiver receiver;
SDL_Surface* screen;

static int winwidth = 0;
static int winheight = 0;
static int advwidth = 0;
static int advheight = 0;
static int realwidth = 0;
static int realheight = 0;
static int lastTime = 0;

class ConsoleOutputter : public CGE::TraceOutputter{
  virtual bool init() {return true;}
  virtual void trace(unsigned channel, int level, const char* function, const char* message){
	printf("%i-%s: %s\n", level, function, message);
  }
};

void quit(){
}

void setMouse(int x, int y){
}

CEXPORT int advLoad(const char* filename){
	ConsoleOutputter* putty = new ConsoleOutputter;
    CGE::TraceManager::instance()->setTraceOutputter(putty);
	//CGE::TraceManager::instance()->setTraceOutputter(alo);
	//TR_INFO("native lib init, trying to load %s", str);
	TR_USE(Frontend);
	TR_INFO("trying to load adventure %s", filename);
	adoc = new AdvDocument();
	if (!adoc->loadDocument(filename)){
		TR_ERROR("failed to load adventure");
		return 0;
	}
	advwidth = adoc->getProjectSettings()->resolution.x;
	advheight = adoc->getProjectSettings()->resolution.y;
	screen = SDL_SetVideoMode(advwidth, advheight, 0, SDL_OPENGL);
	//setAdventureDims(env, advwidth, advheight);
	
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

	/*realwidth = winwidth;
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
	}*/
	realwidth = advwidth;
	realheight = advheight;
	TR_INFO("Setting viewort to %i x %i", realwidth, realheight);
	glViewport(0, 0, realwidth, realheight);

	//TR_DEBUG("init remote server");
	//receiver.start();
	TR_DEBUG("init game");
	Engine::instance()->initGame(quit, setMouse);
	return 1;
}

void render(){
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

  //receiver.processCommands();
  //int time = (unsigned)(CGE::Engine::instance()->getFrameInterval()*1000);
  int time = (int)(emscripten_get_now());
  Engine::instance()->render(time-lastTime);
  lastTime = time;

  SoundEngine::instance()->update(time);
  
  SDL_GL_SwapBuffers();
}

void mainloop(){
	render();

	SDL_Event event;
	int key = 0;
	while (SDL_PollEvent(&event)){
		switch (event.type) 
		{
			case SDL_MOUSEMOTION:
				Engine::instance()->setCursorPos(Vec2i(event.motion.x,event.motion.y));
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT){
					Engine::instance()->leftClick(Vec2i(event.button.x,event.button.y));
				}
				else if (event.button.button == SDL_BUTTON_RIGHT){
					Engine::instance()->rightClick(Vec2i(event.button.x,event.button.y));
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT){
					Engine::instance()->leftRelease(Vec2i(event.button.x,event.button.y));
				}
				break;
			case SDL_KEYDOWN:
			  if (event.key.keysym.sym == SDLK_ESCAPE)
				key = KEY_ESCAPE;
			  else
				key = event.key.keysym.unicode;
			  Engine::instance()->keyPress(key);
			  break;
			case SDL_KEYUP:
			// If escape is pressed, return (and thus, quit)
			  if (event.key.keysym.sym == SDLK_ESCAPE)
				key = KEY_ESCAPE;
			  else
			    key = event.key.keysym.unicode;
			  Engine::instance()->keyRelease(key);
			  break;
			case SDL_QUIT:
			  exit(0);
		}
	}
}

int main(int argc, char** argv){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	advLoad("data/game.dat");
	emscripten_hide_mouse();
	lastTime = emscripten_get_now();
	emscripten_set_main_loop(mainloop, 0, 0);
	return 0;
}

int av_get_cpu_flags(){
	return 0;
}

ALenum alGetEnumValue(const ALchar* ename){
	return (ALenum)0;
}

}