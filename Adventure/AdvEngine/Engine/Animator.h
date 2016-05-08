#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <list>
#include <map>
#include "AdvDoc.h"

namespace adv{

class Object2D;
class RoomObject;
class DynamicAnimation;
class CharacterObject;
struct Color;

class Animator{
  struct ObjectAnim{
    //Object2D* object;
    Vec2i startpos;
    std::list<Vec2i> path;
    float speedfactor;
    float normalization;
    float factor;
  };
  struct RoomAnim{
    Vec2f dir;
    Vec2f currPos;
    Vec2i target;
    float speed;
  };
  struct CamAnim{
    CamAnim() : active(false){
    }
    bool active;
    Vec2f dir;
    Vec2f currPos;
    Vec2f target;
  };
public:
  Animator();
  ~Animator();
  bool add(Object2D* obj, const std::list<Vec2i>& targetpath, float speedfactor);
  void remove(Object2D* obj);
  void remove(RoomObject* room);
  void update(unsigned interval);
  void clear();
  void add(DynamicAnimation* anim);
  void add(Object2D* obj, const Color& targetcolor);
  void add(RoomObject* obj, Vec2i scrollpos, float speed);
  void add(RoomObject* obj, int fade_duration, bool fadein);
  void add(CharacterObject* obj, float sourcescale, float targetscale, bool isUserScale);
  Vec2i getTargetPoisition(Object2D* obj);
  void moveCameraViewport(LookDir dir);
  bool isCamViewportMoving() { return mCamera.active; }
protected:
  bool isPointOnLine(Vec2f from, Vec2f to, Vec2f pt, double epsilon);
  std::map<Object2D*, ObjectAnim> mObjects;
  std::list<DynamicAnimation*> mAnimations;
  std::map<RoomObject*, RoomAnim> mRooms;
  CamAnim mCamera;
};

}

#endif
