#include "system/engine.h"

#include "OSMReader.h"
#include "MapChunk.h"

void engineMain(int argc, char** argv){
  //System::Engine::instance();
  //Editor::instance()->init();

  //render callbacks
  /*Renderer* rend = Engine::instance()->getRenderer();
  rend->setRenderCB(Graphic::render);
  rend->setInitCB(Graphic::init);

  //input callbacks
  Input::Keyboard::instance()->setKeyDownCB(Editor::keypress);
  Input::Mouse::instance()->setButtonUpCB(Editor::mouseUp);
  Input::Mouse::instance()->setButtonDownCB(Editor::mouseDown);*/
  MapChunk map;
  OSMReader rdr("map.osm");
  rdr.read(&map);
  System::Log.flush();
}