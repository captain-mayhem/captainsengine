#include <system/engine.h>
#include <io/TraceManager.h>
#include <SDL/SDL.h>
#include <emscripten.h>
#include <input/keyboard.h>
#include <system/file.h>

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
static int initialized = 0;
static unsigned lastClick = 0;

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

void fileChanged(std::string const& file){
  TR_USE(Frontend);
  TR_INFO("file %s changed, syncing...", file.c_str());
  EM_ASM(     
          FS.syncfs(false,function (err) { // sync TO backing store
              assert(!err);
              ccall('fileWritten', 'v', [], []);
          });
      );
}

void fileWritten(){
  TR_USE(Frontend);
  TR_INFO("file written");
}

CEXPORT int advLoad(const char* filename){
	ConsoleOutputter* putty = new ConsoleOutputter;
    CGE::TraceManager::instance()->setTraceOutputter(putty);
	//CGE::TraceManager::instance()->setTraceOutputter(alo);
	//TR_INFO("native lib init, trying to load %s", str);
	TR_USE(Frontend);
	TR_INFO("trying to load adventure %s", filename);
	adoc = new AdvDocument();
  adoc->getProjectSettings()->savedir = "/IDBFS/adventure";
  CGE::Filesystem::createDir("/IDBFS/adventure");
  adoc->setFileChangedCB(fileChanged);
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
  if (!initialized)
    return;
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

int translateKey(SDL_keysym& sym){
  switch (sym.sym){
    case SDLK_ESCAPE:
      return KEY_ESCAPE;
    case SDLK_RETURN:
      return KEY_RETURN;
    case SDLK_BACKSPACE:
      return KEY_BACKSPACE;
    case SDLK_SPACE:
      return KEY_SPACE;
    case SDLK_UP:
      return KEY_UP;
    case SDLK_DOWN:
      return KEY_DOWN;
    case SDLK_LEFT:
      return KEY_LEFT;
    case SDLK_RIGHT:
      return KEY_RIGHT;
    case SDLK_DELETE:
      return KEY_DELETE;
    case SDLK_RCTRL:
    case SDLK_LCTRL:
      return KEY_CTRL;
    case SDLK_RALT:
    case SDLK_LALT:
      return KEY_ALT;
    case SDLK_F1:
      return KEY_F1;
    case SDLK_F2:
      return KEY_F2;
    default:
      return sym.unicode;
  }
}

void mainloop(){
	render();

	SDL_Event event;
	int key = 0;
	while (SDL_PollEvent(&event)){
    short ascii;
		switch (event.type) 
		{
			case SDL_MOUSEMOTION:
				Engine::instance()->setCursorPos(Vec2i(event.motion.x,event.motion.y));
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT){
          unsigned ticks = SDL_GetTicks();
          if (ticks - lastClick < 500)
            Engine::instance()->doubleClick(Vec2i(event.button.x,event.button.y));
          else
            Engine::instance()->leftClick(Vec2i(event.button.x,event.button.y));
          lastClick = ticks;
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
			  key = translateKey(event.key.keysym);
			  Engine::instance()->keyPress(key);
        ascii = event.key.keysym.unicode;
        if (ascii > 31 && ascii < 256){
          Engine::instance()->keyAscii((char)ascii);
        }
			  break;
			case SDL_KEYUP:
			// If escape is pressed, return (and thus, quit)
			  key = translateKey(event.key.keysym);
			  Engine::instance()->keyRelease(key);
			  break;
			case SDL_QUIT:
			  exit(0);
		}
	}
}

int main(int argc, char** argv){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
  EM_ASM(
    FS.mkdir('/IDBFS');
    FS.mount(IDBFS, {}, '/IDBFS');
    FS.syncfs(true, function (err) {
            assert(!err);
            ccall('fsMounted', 'v', [], []);
        });
  );
	//advLoad("data/game.dat");
	emscripten_hide_mouse();
	lastTime = emscripten_get_now();
	emscripten_set_main_loop(mainloop, 0, 0);
	return 0;
}

void fsMounted(){
  TR_USE(Frontend);
  advLoad("/data/game.dat");
  initialized = 1;
  TR_INFO("save path is %s", adoc->getProjectSettings()->savedir.c_str());
}

int av_get_cpu_flags(){
	return 0;
}

ALenum alGetEnumValue(const ALchar* ename){
	return (ALenum)0;
}

}