//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: field.cpp                                                  |
//  ==================================================================

#include "field.hh"

//Constructor
Field::Field() : object(0), numVertices(0), vertices(0), texCoords(0), tryObject(0), overlay(0), script(0),
  treasure(false), items(0) {
}

Field::Field(const Field& f){
  position_ = f.position_;
  id_ = f.id_;
  active_ = f.active_;
  interpolate_ = f.interpolate_;
  
  wallbits = Bitset(f.wallbits);
  doorbits = Bitset(f.doorbits);
  for (int i = 0; i < 4; i++){
    dooridx[i] =  f.dooridx[i];
  }
  object = f.object;
  tempVis = Bitset(f.tempVis);
  numVertices = f.numVertices;
  vertices = new Vector3D[numVertices];
  texCoords = new Vec2f[numVertices];
  for (unsigned i = 0; i < numVertices; i++){
    vertices[i] = f.vertices[i];
    texCoords[i] = f.texCoords[i];
  }
  tryObject = f.tryObject;
  overlay = f.overlay;
  script = f.script;
  
  treasure = f.treasure;
  items = f.items;
}

//Destructor
Field::~Field(){
  if (vertices)
    delete [] vertices;
  if (texCoords)
    delete [] texCoords;
}

void Field::render() const{
  //dummy function.
  // The fields are not rendered here to gain optimization possibilities
}

void Field::render2D() const{
  //dummy function.
  // The fields are not rendered here to gain optimization possibilities
}

void Field::update(){
  //also a dummy function
}
