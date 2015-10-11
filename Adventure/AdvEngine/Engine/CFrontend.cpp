#include <system/engine.h>
#include <io/TraceManager.h>
#include <SDL/SDL.h>
#include <emscripten.h>
#include <input/keyboard.h>
#include <system/file.h>

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"
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
  TR_USE(Frontend);
  TR_DEBUG("init core engine");
  CGE::Engine::init();

	ConsoleOutputter* putty = new ConsoleOutputter;
  CGE::TraceManager::instance()->setTraceOutputter(putty);

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

  TR_DEBUG("init renderer");
  CGE::Engine::instance()->startupRenderer("GL2");
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->initRendering();
  
	TR_DEBUG("init adventure engine");
	Engine::init();
	Engine::instance()->setData(adoc);
	SoundEngine::init();
	SoundEngine::instance()->setData(adoc);

  rend->switchMatrixStack(CGE::Projection);
  rend->resetModelView();
	rend->ortho(0, advwidth, advheight, 0, -1.0, 1.0);
	//glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  rend->switchMatrixStack(CGE::Modelview);
	rend->setClearColor(CGE::Vec4f(0.0,0.0,0.0,1.0));
  rend->setColor(1.0f, 1.0f, 1.0f, 1.0f);

  rend->enableDepthTest(false);
	//GL()enableClientState(ATTR_VERTEX_ARRAY);
	//GL()enableClientState(ATTR_TEXTURE_COORD_ARRAY);
  rend->enableTexturing(true);
	//glAlphaFunc(GL_GREATER, 0);
	//glEnable(GL_ALPHA_TEST);
	//glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableBlend(true);

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
	rend->viewport(0, 0, realwidth, realheight);

	//TR_DEBUG("init remote server");
	//receiver.start();
	TR_DEBUG("init game");
	Engine::instance()->initGame(quit, setMouse);
	return 1;
}

void render(){
  if (!initialized)
    return;
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->switchMatrixStack(CGE::Projection);
  rend->resetModelView();
  rend->ortho(0, advwidth, advheight, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  rend->switchMatrixStack(CGE::Modelview);
  rend->enableDepthTest(false);
  //glEnableClientState(GL_VERTEX_ARRAY);
  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableBlend(true);

  rend->clear(COLORBUFFER | ZBUFFER);
  rend->resetModelView();

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
    case SDLK_BACKQUOTE:
      return KEY_CIRCUMFLEX;
    case SDLK_PAGEUP:
      return KEY_PAGEUP;
    case SDLK_PAGEDOWN:
      return KEY_PAGEDOWN;
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