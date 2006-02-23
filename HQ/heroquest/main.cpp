//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: main.cpp                                                   |
//  ==================================================================

#include "SDL.h"

#include <string>
#ifdef WIN32
#include <direct.h>
#ifdef _DEBUG
#include <windows.h>
#include <crtdbg.h>
#endif
#endif

#include "gamestate.hh"
#include "camera.hh"
#include "texture.hh"
#include "font.hh"
#include "world.hh"
#include "console.hh"
#include "message.hh"
#include "player.hh"
#include "clientsocket.hh"
#include "script.hh"

using std::string;

//some important global objects
Renderer intern_;
Renderer* gl;
GameState game;
Camera cam;
Texture tex;
Font text;
Font output;
Font line;
World wrld;
Console consol;
Player plyr;
Message msg;
Script scr;
ClientSocket* sock;
string path;

//wrapper for the getcwd function to get the current working directory
const string getCwd(){
  char cwd[2048];
#ifdef WIN32
  _getcwd(cwd, 2048);
#else
  getcwd(cwd, 2048);
#endif
  return string(cwd);
}

//callback for continuous updates
Uint32 rendercallback(Uint32 interval, void* param){
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 0;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);
  return interval;
}

//the program starts here
int main(int argc,char **argv)
{
#ifdef WIN32
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC 1
//int flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
//flag |= _CRTDBG_CHECK_ALWAYS_DF;
//_CrtSetDbgFlag(flag);
#endif
#endif

  //calculate correct directory, no matter from where the program is started
  string temp_path(argv[0]);
  int t = temp_path.find_last_of('/');
  if (t != -1 && temp_path[0] != '/'){
    temp_path.erase(t);
    path = getCwd()+"/"+temp_path+"/";
  }
  else
    path = getCwd()+"/";
	
  //atexit(SDL_Quit);


  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0){
    cerr << "SDL initialization failed: " << SDL_GetError();
    exit(EXIT_FAILURE);
  }

  //try to set a video mode supported by most graphic cards
  const SDL_VideoInfo* info = SDL_GetVideoInfo();
  int width = SCREENWIDTH;
  int height = SCREENHEIGHT;
  int bpp = info->vfmt->BitsPerPixel;

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  int flags = SDL_OPENGL | SDL_RESIZABLE | SDL_FULLSCREEN;
  
  if (SDL_SetVideoMode(width, height, bpp, flags) == 0){
    
    //try another setting
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  
    if (SDL_SetVideoMode(width, height, bpp, flags) == 0){
      //giving up
      cerr << "Required video mode not supported: "<< SDL_GetError();
      exit(EXIT_FAILURE);
    }
  }
  
  SDL_WM_GrabInput(SDL_GRAB_ON);
  
  gl = &intern_;
  gl->resizeGL(width,height);
  gl->initializeGL();

  SDL_EnableUNICODE(1);

  // init game
  game.init();
  
  // start game
  if(!game.start())
    return 0;

  SDL_Event event;
  
  while(!gl->isFinished()){
    while(SDL_PollEvent(&event)){
      switch(event.type){
        case SDL_KEYDOWN:
          gl->keyPressEvent(&event.key.keysym);
          break;
        case SDL_KEYUP:
          gl->keyReleaseEvent(&event.key.keysym);
          break;
        case SDL_MOUSEBUTTONDOWN:
          gl->mousePressEvent(&event.button);
          break;
        case SDL_MOUSEBUTTONUP:
          gl->mouseReleaseEvent(&event.button);
          break;
        case SDL_MOUSEMOTION:
          gl->mouseMoveEvent(&event.motion);
          break;
        case SDL_USEREVENT:
	        gl->ignoreNextMouse();
          break;
        case SDL_QUIT:
          exit(EXIT_SUCCESS);
          break;
        case SDL_VIDEORESIZE:
          break;
      }
    }
    game.run();
    gl->paintGL();
    gl->calculateFrameRate();
    SDL_GL_SwapBuffers();
  }

  SDL_ShowCursor(1);
  SDL_WM_GrabInput(SDL_GRAB_OFF);

#ifndef WIN32
  SDL_Quit();
#endif
  
  return 0;
}
