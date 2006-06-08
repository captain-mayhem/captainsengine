//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: camera.cpp                                                 |
//  ==================================================================

#include <math.h>
//#include "math.hh"
#include "renderer.h"
//#include "font.hh"
//#include "message.hh"
#include "world.h"
//#include "gamestate.hh"
#include "camera.h"

Camera::Camera(){
  //Set some standard values
  position_ = Vector3D();
  view_	= Vector3D(0.0,0.0,1.0);
  upVector_ = Vector3D(0.0,1.0,0.0);
  speed_ = 15.0;
  modelPos_ = Vector2D();
}

Camera::~Camera(){
}

//Set camera parameters
void Camera::positionCamera(const Vector3D position, const Vector3D view, const Vector3D up){
  position_ = position;
  view_ = view;
  upVector_ = up;
  oldPos_ = modelPos_;
  modelPos_ = wrld.realToModelPos(position_);
  wrld.updateCollisionVertices(modelPos_);
}

//rotate the view around the camera position
void Camera::rotateView(float angle, Vector3D v){

  Vector3D newView;
  // Get the view vector
  Vector3D vView = view_ - position_;

  float cosTheta = Math::Cosinus(angle);
  float sinTheta = Math::Sinus(angle);

  // find the new x position for the new rotated point
  newView.x  = (cosTheta + (1 - cosTheta) * v.x * v.x)	* vView.x;
  newView.x += ((1 - cosTheta) * v.x * v.y - v.z * sinTheta) * vView.y;
  newView.x += ((1 - cosTheta) * v.x * v.z + v.y * sinTheta) * vView.z;

  // find the new y position for the new rotated point
  newView.y  = ((1 - cosTheta) * v.x * v.y + v.z * sinTheta) * vView.x;
  newView.y += (cosTheta + (1 - cosTheta) * v.y * v.y)	* vView.y;
  newView.y += ((1 - cosTheta) * v.y * v.z - v.x * sinTheta) * vView.z;

  // Find the new z position for the new rotated point
  newView.z  = ((1 - cosTheta) * v.x * v.z - v.y * sinTheta) * vView.x;
  newView.z += ((1 - cosTheta) * v.y * v.z + v.x * sinTheta) * vView.y;
  newView.z += (cosTheta + (1 - cosTheta) * v.z * v.z)	* vView.z;

  view_ = position_ + newView;
}


//strafe the camera left and right depending on the speed
void Camera::strafeCamera(float speed){
  //update position and view
  position_.x += strafe_.x * speed;
  position_.z += strafe_.z * speed;
  view_.x += strafe_.x * speed;
  view_.z += strafe_.z * speed;

  //The position changed one field
  Vector2D temp = wrld.realToModelPos(position_);
  if (temp != modelPos_){
    game.setMoves(game.getMoves()-1);
    wrld.updateCollisionVertices(temp);
    game.setMoves(game.getMoves()+1);
    oldPos_ = modelPos_;
    modelPos_ = temp;
    update2D();
  }
}

//move the camera forward or backward depending on the speed
void Camera::moveCamera(float speed){
  // the current view vector
  Vector3D vVector = view_ - position_;
  vVector = vVector.normalized();

  position_.x += vVector.x * speed;
  position_.z += vVector.z * speed;
  view_.x += vVector.x * speed;
  view_.z += vVector.z * speed;

  //The position changed one field
  Vector2D temp = wrld.realToModelPos(position_);
  if (temp != modelPos_){
    game.setMoves(game.getMoves()-1);
    wrld.updateCollisionVertices(temp);
    game.setMoves(game.getMoves()+1);
    oldPos_ = modelPos_;
    modelPos_ = temp;
    update2D();
  }
}


//checks all the polygons in list and resets the camera if collided
void Camera::checkCameraCollision(Vector3D **pVertices, int numOfVerts){
  //Can happen directly after loading world, that this is NULL
  //because the world is loaded asynchronously within another thread
  if (pVertices == NULL)
    return;
  // go through all given triangles
  for(int i = 0; i < numOfVerts; i += 3){
    // the current triangle
    Vector3D triangle[3] = { *pVertices[i], *pVertices[i+1], *pVertices[i+2] };
    Vector3D normal = Math::Normal(triangle);
    
    float distance = 0.0f;
    int classification = Math::ClassifySphere(position_, normal, 
		  triangle[0], radius_, distance);
    // If the sphere intersects the polygon's plane, then check further
    if(classification == INTERSECTS){
      //offset to the plane
      Vector3D offset = normal * distance;
      Vector3D intersection = position_ - offset;

     //collision?
     if(Math::InsidePolygon(intersection, triangle, 3) ||
        Math::EdgeSphereCollision(position_, triangle, 3, radius_ / 2)){
      
        //correct camera position
        offset = Math::GetCollisionOffset(normal, radius_, distance);
        position_ = position_ + offset;
        view_ = view_ + offset;
      }
    }
  }
}

// update camera
void Camera::update(){
  //strafe vector
  Vector3D cross = (view_ - position_).cross(upVector_);
  strafe_ = cross.normalized();

  // Calculate frame rate for time based movement
  //gl->calculateFrameRate();
  
  //Print position
  char temp[100];
  sprintf(temp, "Coordinates(2D): x: %d y: %d", modelPos_.x, modelPos_.y);
  text.glPrint(20, 700, temp, 0);
}


// set lookat of the camera
void Camera::look(){
  gluLookAt(position_.x, position_.y, position_.z,
	    view_.x, view_.y, view_.z,
	    upVector_.x, upVector_.y, upVector_.z);
}

// update movement on the map
void Camera::update2D(){
  if (modelPos_.x > oldPos_.x){
    msg.process("d");
  }
  if (modelPos_.x < oldPos_.x){
    msg.process("a");
  }
  if (modelPos_.y < oldPos_.y){
    msg.process("w");
  }
  if (modelPos_.y > oldPos_.y){
    msg.process("s");
  }
}

// find looking direction of the camera
Direction Camera::getLookDirection(){
  Vector3D view = view_ - position_;
  Direction dir;
  //looking mainly in x-direction
  if (Math::Absolute(view.x) > Math::Absolute(view.z)){
    if (view.x > 0)
      dir = RIGHT;
    else
      dir = LEFT;
  }
  //... y-direction
  else{
    if (view.z > 0)
      dir = BOTTOM;
    else
      dir = TOP;
  }
  return dir;
}