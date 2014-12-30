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

#include "field.h"

//Constructor
Field::Field() : object(0), tryObject(0), overlay(0), script(0),
  treasure(false), items(0) {
    numModels = 0;
    models = NULL;
}

Field::Field(const Field& f){
  //std::cerr << "Copy ";
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
  numModels = f.numModels;
  //models = new std::vector<CGE::ModelInstance>();
  models = new MeshGeo::Model*[numModels];
  for (unsigned i = 0; i < numModels; i++)
    models[i] = f.models[i];
  //for (int i = 0; i < models->size(); i++)
  //  (*models)[i] = (*f.models)[i];
  tryObject = f.tryObject;
  overlay = f.overlay;
  script = f.script;
  
  treasure = f.treasure;
  items = f.items;
}

//Destructor
Field::~Field(){
  if (models)
    delete [] models;
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
