//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: camera.h                                                   |
//  ==================================================================

#ifndef CAMERA_HH
#define CAMERA_HH

#include "math/vector.h"
#include "field.h"

using ::Math::Vector3D;

//! Represents the camera in the game
/*! Because the camera is a ego-perspective camera,
 *  it provides methods for movement and collision detection
 */
class Camera{
public:
  //! default constructor
  Camera();  
  //! destructor
  ~Camera();
  //! Get camera position
  inline Vector3D position() {	return position_;}
  //! Get lookat
  inline Vector3D view() { return view_;}
  //! Get up vector
  inline Vector3D upVector() { return upVector_;}
  //! Get strafe vector
  inline Vector3D strafe() { return strafe_;}
  //! Get the map position in 2D of the camera
  inline Vector2D modelPos() { return modelPos_;}
  //! Get the camera movement speed
  inline float getSpeed() { return speed_;}
  //! Set the current camera rotation around the x-axis
  inline void setCurrRotX(float rot){ currRotX_ = rot;}
  //! Get the current camera rotation around the x-axis
  inline float getCurrRotX(){ return currRotX_;}
  //! Set the last camera rotation around the x-axis
  inline void setLastRotX(float rot){ lastRotX_ = rot;}
  //! Get the last camera rotation around the x-axis
  inline float getLastRotX(){ return lastRotX_;}
  //! intialize the camera's bounding sphere radius
  inline void setCameraRadius(float radius) { radius_ = radius;}
  //! change camera parameters
  void positionCamera(const Vector3D position, const Vector3D view, const Vector3D up);
  //! rotate the camera view around the position
  void rotateView(float angle, Vector3D v);
  //! strafe the camera left or right
  void strafeCamera(float speed);
  //! move the camera forward or backward
  void moveCamera(float speed);
  //! check for camera collision
  /*! checks, if the camera's bounding sphere collided 
   * with any triangle specified with pVertices
   * \param pVertices The vertices with which the camera could collide
   * \param numOfVerts the number of vertices that were given in pVertices
   */
  void checkCameraCollision(Vector3D **pVertices, int numOfVerts);
  //! This updates the camera's view and calls framerate calculation
  void update();
  //! Gives the lookAt parameters to OpenGL
  void look();
  //! Determines the 2D looking direction
  Direction getLookDirection(); 

private:
  //! updates the map, if the camera moved in map coordinates
  void update2D();
  //! The camera's position
  Vector3D position_;	
  //! The camera's view
  Vector3D view_;						
  //! The camera's up vector
  Vector3D upVector_;		
  //! The camera's strafe vector
  Vector3D strafe_;						
  //! The camera's bounding sphere radius
  float radius_;
  //! moving speed of the camera
  float speed_;
  //! The camera in the 2D map world
  Vector2D modelPos_;
  //! The old position of the camera
  Vector2D oldPos_;
  //! The current x-axis rotation
  float currRotX_;
  //! The last x-axis rotation
  float lastRotX_;
};

extern class Camera cam;

#endif
