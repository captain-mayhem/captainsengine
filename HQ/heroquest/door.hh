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

#include "gameobject.hh"
struct ScriptInfo;

using std::ofstream;
using std::ifstream;

//! The doors in the level
class Door : public GameObject
{
public:
  //! Constructor
  Door();
  //! Copy Constructor
  Door(const Door& d);
  //! Destructor
  ~Door();
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
  void read(ifstream& in);
  //! provides vertices for collision detection
  inline Vector3D* getColVerts(){return verts_;}
  //! set the door script
  inline void setScript(ScriptInfo* si) {script_ = si;}
  //! get the door script
  inline ScriptInfo* getScript() {return script_;}
private:
  //! The second neighbouring field to the door
  /*! The first is stored in position from gameobject
   */
  Vector2D pos2_;
  //! collision vertices
  Vector3D verts_[8];
  //! door script
  ScriptInfo* script_;
};

//! less operator for STL storage
inline bool operator<(Door a, Door b){
  return a.getId() < b.getId();
}
//! equal operator
inline bool operator==(Door a, Door b){
  return a.getId() == b.getId();
}

#endif
