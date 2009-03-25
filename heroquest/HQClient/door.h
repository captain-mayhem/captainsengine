//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: door.hh                                                    |
//  ==================================================================

#ifndef DOOR_HH
#define DOOR_HH

#include <iostream>
#include <fstream>

#include "gameobject.h"
struct ScriptInfo;

using std::ofstream;
using std::ifstream;

using ::CGE::Vector3D;

namespace Graphics{
class Scene;
}

//! The doors in the level
class Door{
public:
  //! Constructor
  Door();
  //! Copy Constructor
  Door(const Door& d);
  //! Destructor
  ~Door();
  //! Set second position
  inline void setPosition(const Vector2D& pos) {pos_ = pos;}
  //! Get second position
  inline const Vector2D getPosition() const  {return pos_;}
  //! Set second position
  inline void setPosition2(const Vector2D& pos) {pos2_ = pos;}
  //! Get second position
  inline const Vector2D getPosition2() const  {return pos2_;}
  //! Load door model
  /*! This is currently without functionality, but in the future,
   *  doors should be a model created with a modelling tool. Then, the
   *  model loading will be in here.
   */
  void load();
  //! Render door
  void render() const;
  //! Render door in 2D
  void render2D() const;
  //! Update collision vertices
  void update();
  //! writes door data
  void write(ofstream& out);
  //! reads door data
  void read(ifstream& in, Graphics::Scene& scene);
  //! provides vertices for collision detection
  inline Vector3D* getColVerts(){return verts_;}
  //! set the door script
  inline void setScript(ScriptInfo* si) {script_ = si;}
  //! get the door script
  inline ScriptInfo* getScript() {return script_;}
  //! open or close the door
  inline void setClosed(bool close) {closed_ = close;}
  //! is the door closed
  inline bool isClosed() const {return closed_;}
  //! set the type of the door
  inline void setType(short type) {type_ = type;}
  //! get the type of the door
  inline short getType() const {return type_;}
  //! get the model of the door
  inline MeshGeo::Model* getModel() {return model_;}
  //! set the model of the door
  inline void setModel(MeshGeo::Model* mdl) {model_ = mdl;}
private:
  //! The neighbouring field to the door
  Vector2D pos_;
  //! The second neighbouring field to the door
  /*! The first is stored in position from gameobject
   */
  Vector2D pos2_;
  //! the type of the door
  short type_;
  //! is the door open
  bool closed_;
  //! the door model
  MeshGeo::Model* model_;
  //! collision vertices
  Vector3D verts_[8];
  //! door script
  ScriptInfo* script_;
};

#endif

