#include <system/engine.h>
#include <renderer/renderer.h>

#include "OSMReader.h"
#include "MapChunk.h"

MapChunk map;

void init(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->setClearColor(Vec3f(0.5,0.5,0.5));
}

void render(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->clear(ZBUFFER | COLORBUFFER);
  rend->projection(60,1.0f,0.1f,100000.0f);
  rend->enableTexturing(false);
  rend->resetModelView();
  rend->translate(0,0,-5);
  rend->setColor(1.0,1.0,0.0,1.0);
  Vec3f ep = Vec3f(555.24565f,2670.81660f,5757.44706f);
  ep.normalize();
  rend->lookAt(ep*6373,ep*6371,Vec3f(0,1,0));
  //rend->lookAt(Vec3f(0,0,-2000),Vec3f(555.24565f,2670.81660f,5757.44706f),Vec3f(0,1,0));
  map.render();
}

void engineMain(int argc, char** argv){
  //System::Engine::instance();
  //Editor::instance()->init();

  //render callbacks
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->setRenderCB(render);
  rend->setInitCB(init);
/*
  //input callbacks
  Input::Keyboard::instance()->setKeyDownCB(Editor::keypress);
  Input::Mouse::instance()->setButtonUpCB(Editor::mouseUp);
  Input::Mouse::instance()->setButtonDownCB(Editor::mouseDown);*/
  OSMReader rdr("map.osm");
  rdr.read(&map);
  System::Log.flush();
}