//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: field.hh                                                   |
//  ==================================================================

#ifndef FIELD_HH
#define FIELD_HH

#include "math/vector.h"
//#include "renderer/model.h"
#include "mesh/model.h"
#include "bitset.h"
#include "gameobject.h"
//#include "script.hh"
//#include "overlay.hh"
class Overlay;
struct ScriptInfo;
class Inventory;

using ::Math::Vector3D;
using ::Math::Vec2f;

#define QUADSIZE 8
#define WALLHEIGHT 16
#define THICKNESS 1
// WALLHEIGHT/QUADSIZE
#define RATIO 2
#define DOORTHICK 0.9990
#define DOORHEIGHT 2.0/3*WALLHEIGHT
#define DOORTEX 2.0/3*RATIO

//! specifies direction of walls, doors, etc.
enum Direction{
  TOP,
  RIGHT,
  BOTTOM,
  LEFT
};

//! the field is the base unit of the map
class Field : public GameObject
{
public:
  //! Constructor
  Field();
  //! Copy Constructor
  Field(const Field& f);
  //! Destructor
  ~Field();
  //! Renders a field
  virtual void render() const;
  //! Renders in 2D.
  virtual void render2D() const;
  //! Updates status
  virtual void update();
  // the following variables are public. Although this violates somehow strict
  // object oriented design, in this case, it is more useful than harmful to the
  // clarity of the code, cause this class in its use is similiar to a struct.
  // And because all of this variables must be able to be set and to be read, 
  // you gain little with making them private.
  //! stores which walls exits
  Bitset wallbits;
  //! stores which doors exists
  Bitset doorbits;
  //! door indices
  short dooridx[4];
  //! the gameobject on the field
  GameObject* object;
  //! a helper for 2d and 3d visiblility calculations
  Bitset tempVis;
  //! number of vertices
  //unsigned numVertices;
  //! the vertices
  //Vector3D *vertices;
  //! number of models
  //unsigned numModels;
  //! number of used models
  //unsigned usedModels;
  //! the models
  //Graphics::ModelInstance* models;
  //! the texture coordinates
  //Vec2f *texCoords;
  //! number of models on the field
  unsigned numModels;
  //! the ids of the models
  MeshGeo::Model** models;
  //! object storage, if two creatures try to cross
  /*! If two creatures are on the same field, the first
   * is stored here
   */
  GameObject* tryObject;
  //! the overlay of the field
  Overlay* overlay;
  //! the hooked in lua script
  ScriptInfo* script;
  //! if already searched for treasure
  bool treasure;
  //! lost items
  Inventory* items;
};

#endif
