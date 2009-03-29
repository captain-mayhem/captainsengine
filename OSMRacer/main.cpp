#include <system/engine.h>
#include <renderer/renderer.h>
#include <renderer/camera.h>
#include <input/keyboard.h>
#include <physics/Simulator.h>
#include <physics/CollisionPlane.h>

#include "OSMReader.h"
#include "MapChunk.h"
#include "Utilities.h"
#include "GeoGen.h"
#include "GeoCache.h"
#include "Vehicle.h"

//MapChunk map;
#include "TerrainChunk.h"
#include <renderer/forms.h>

using namespace CGE;

Graphics::Camera cam;
TerrainChunk test;
Vehicle* car;
CollisionPlane* ground;

void init(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->setClearColor(Vec3f(0.5,0.5,0.5));
  //rend->renderMode(Graphics::Wireframe);
  rend->enableBackFaceCulling(true);
  //Vec3f ep = Vec3f(map.getCenter().x,map.getCenter().y,map.getCenter().z);
  //ep.normalize();
  cam.project(60,1,0.1f,5000);
  cam.lookAt(Vec3f(0,5,0), Vec3f(), Vec3f(0,0,-1));
  //cam.lookAt(Vec3f(ep*200),Vec3f(),Vec3f(0,1,0));
}

void render(){
  //GeoGen::useGeometry();
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->clear(ZBUFFER | COLORBUFFER);
  //rend->projection(60,1.0f,0.1f,100000.0f);
  rend->enableTexturing(false);
  rend->resetModelView();
  //rend->translate(0,0,-5);
  rend->setColor(0.0,1.0,0.0,1.0);
  //cam.lookAt(ep*(Utility::SCALE+20),ep*Utility::SCALE,Vec3f(0,1,0));
  //rend->lookAt(Vec3f(ep*20),Vec3f(),Vec3f(0,1,0));
  //rend->lookAt(ep*(Utility::SCALE+20),ep*Utility::SCALE,Vec3f(0,1,0));
  //rend->lookAt(Vec3f(0,0,-2000),Vec3f(555.24565f,2670.81660f,5757.44706f),Vec3f(0,1,0));
  cam.activate();
  //map.render(&cam);
  test.render(cam);
  car->render(cam);
  
}

void keyPress(int key, float timeInterval){
  switch(key){
    //case KEY_ESCAPE:
    //  EXIT();
    //  break;
    case KEY_W:
      cam.walk(50*timeInterval);
      break;
    case KEY_S:
      cam.walk(-50*timeInterval);
      break;
    case KEY_A:
      cam.strafe(-50*timeInterval);
      break;
    case KEY_D:
      cam.strafe(50*timeInterval);
      break;
    case KEY_UP:
      cam.pitch(timeInterval);
      break;
    case KEY_DOWN:
      cam.pitch(-timeInterval);
      break;
    case KEY_LEFT:
      cam.yaw(timeInterval);
      break;
    case KEY_RIGHT:
      cam.yaw(-timeInterval);
      break;
  }
}

void simulate(double time){
  car->accelerate(0.3);
}

void engineMain(int argc, char** argv){
  //System::Engine::instance();
  //Editor::instance()->init();

  //render callbacks
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->setRenderCB(render);
  rend->setInitCB(init);

  //physics callbacks
  CGE::Simulator* sim = System::Engine::instance()->getSimulator();
  sim->setSimulationCallback(simulate);
  sim->setGravitation(Vec3f(0,-0.5,0));

  //input callbacks
  Input::Keyboard::instance()->setKeyPressedCB(keyPress);
/*Input::Mouse::instance()->setButtonUpCB(Editor::mouseUp);
  Input::Mouse::instance()->setButtonDownCB(Editor::mouseDown);*/

  /*GeoCache::init();

  //start geo gen thread
  System::Thread geogen;
  //geogen.create(GeoGen::generateGeometry,NULL);

  OSMReader rdr("map.osm");
  rdr.read(&map);
  System::Log.flush();*/

  car = new Vehicle(*sim);
  //car->setPosition(Vec3f(0,2,0));
  ground = new CollisionPlane(Vec3f(0,1,0),0);
  sim->getRootSpace()->add(ground);

  test.generate(17);
}

