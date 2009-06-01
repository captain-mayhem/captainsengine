#include <system/engine.h>
#include <renderer/renderer.h>
#include <renderer/camera.h>
#include <input/keyboard.h>
#include <physics/Simulator.h>
#include <physics/CollisionPlane.h>

#include "OSMReader.h"
#include "MapChunk.h"
#include "OSMMapAdapter.h"
#include "SimpleMapAdapter.h"
#include "Utilities.h"
#include "GeoGen.h"
#include "Vehicle.h"
#include "StreetGenerator.h"
#include "FollowCam.h"
#include "Terrain.h"
#include <renderer/forms.h>

using namespace CGE;

CGE::Camera* activeCam;
CGE::Camera cam;
FollowCam followCam;

Terrain test(256,256,4.0f,4.0f,NULL,1.0f);
Vehicle* car;
MapChunk map;

void init(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->setClearColor(Vec3f(0.2f,0.3f,1.0f));
  //rend->renderMode(CGE::Wireframe);
  rend->enableBackFaceCulling(true);
  //Vec3f ep = Vec3f(map.getCenter().x,map.getCenter().y,map.getCenter().z);
  //ep.normalize();
  cam.project(60,1,0.1f,5000);
  cam.lookAt(Vec3f(0,5,0), Vec3f(), Vec3f(0,0,-1));
  followCam.project(60,1,0.1f,5000);
  followCam.lookAt(Vec3f(0,5,0), Vec3f(), Vec3f(0,0,-1));
  //cam.lookAt(Vec3f(ep*200),Vec3f(),Vec3f(0,1,0));
  rend->enableLight(0, true);
}

void render(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->clear(ZBUFFER | COLORBUFFER);
  
  rend->enableTexturing(false);
  rend->resetModelView();
  rend->enableLighting(true);

  activeCam->activate();
  rend->switchFromViewToModelTransform();
  CGE::Light lit(CGE::Light::Directional, Vec3f(0.5,-1,0));
  rend->setLight(0, lit);
  test.render(*rend, *activeCam);

  car->render(*activeCam);
  map.render(activeCam);
  rend->enableLighting(false);
  
}

float acceleration = 0;
float steer = 0;

void keyPress(int key, float timeInterval){
  switch(key){
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
    case KEY_I:
      cam.pitch(timeInterval);
      break;
    case KEY_K:
      cam.pitch(-timeInterval);
      break;
    case KEY_J:
      cam.yaw(timeInterval);
      break;
    case KEY_L:
      cam.yaw(-timeInterval);
      break;
    case KEY_UP:
      if (acceleration < 100)
        acceleration += 5.0f*timeInterval;
      break;
    case KEY_DOWN:
      if (acceleration > -100)
        acceleration -= 5.0f*timeInterval;
      break;
    case KEY_LEFT:
      if (steer > -1.0)
        steer -= 2.0f*timeInterval;
      break;
    case KEY_RIGHT:
      if (steer < 1.0)
        steer += 2.0f*timeInterval;
      break;
  }
}

void keyUp(int key){
  switch(key){
    case KEY_UP:
      //acceleration = 0;
      break;
    case KEY_DOWN:
      //acceleration = 0;
      break;
    case KEY_LEFT:
      steer = 0;
      break;
    case KEY_RIGHT:
      steer = 0;
      break;
  }
}

void keyDown(int key){
  switch(key){
    case KEY_ESCAPE:
      EXIT();
      break;
    case KEY_1:
      activeCam = &followCam;
      break;
    case KEY_2:
      activeCam = &cam;
      break;
  }
}

void simulate(double time){
  car->simulate(acceleration, steer);
}

bool collide(CollisionObject* a, CollisionObject* b, CollisionResponse& collision){
  bool handleCollision = false;
  if (a->getType() == CollisionBase::Heightfield && b->getType() == CollisionBase::Cylinder)
    handleCollision = true;
  if (a->getType() == CollisionBase::Cylinder && b->getType() == CollisionBase::Heightfield)
    handleCollision = true;
  collision.setMu((float)HUGE_VAL);
  collision.setSlip1(0.0001f);
  collision.setSlip2(0.00003f);
  collision.setSoftERP(0.99f);
  collision.setSoftCFM(0.001f);
  collision.setBounce(0.1f);
  return handleCollision;
}

void engineMain(int argc, char** argv){

  //render callbacks
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->setRenderCB(render);
  rend->setInitCB(init);

  //physics callbacks
  CGE::Simulator* sim = CGE::Engine::instance()->getSimulator();
  sim->setSimulationCallback(simulate);
  sim->setCollisionCallback(collide);
  sim->setGravitation(Vec3f(0,-9.81f,0));

  //input callbacks
  Input::Keyboard::instance()->setKeyPressedCB(keyPress);
  Input::Keyboard::instance()->setKeyUpCB(keyUp);
  Input::Keyboard::instance()->setKeyDownCB(keyDown);
/*Input::Mouse::instance()->setButtonUpCB(Editor::mouseUp);
  Input::Mouse::instance()->setButtonDownCB(Editor::mouseDown);*/

  car = new Vehicle(*sim);

  test.generateTerrainChunks(32);
  sim->getRootSpace()->add(&test);
  
  OSMMapAdapter mapAdapter;
  mapAdapter.load("map2.osm", &map);
  //SimpleMapAdapter mapAdapter;
  //mapAdapter.load("testmaps/crossing.map", &map);
  
  StreetGenerator strgen(&map);
  strgen.buildStreets(map.getStreets());
  CGE::Log.flush();

  activeCam = &cam;
  followCam.setTarget(car);
}

