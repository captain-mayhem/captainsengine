//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | Stefan Ruff                                                      |
// |------------------------------------------------------------------|
// | File: overlay.hh                                                 |
//  ==================================================================

#ifndef OVERLAY_HH
#define OVERLAY_HH

#include <string>

#include "gameobject.h"
#include "math/vector.h"
#include "field.h"
#include "world.h"

using std::string;

class Overlay : public GameObject {
public:
  //! Constructor
  Overlay();
  Overlay(const Overlay& o);
  //! Destructor
  virtual ~Overlay();
  //! sets the name of the overlay
  inline void setName(const string name){name_ = name;}
  //! returns the name of the overlay
  inline const string getName() const {return name_;}  
  //! sets the overlay shortcut
  inline void setShort(const string shortcut){short_ = shortcut;}
  //! returns the overlay shortcut
  inline const string getShort() const {return short_;}
  //! sets the width of the overlay
  inline void setWidth(const short width){width_ = width;}
  //! sets the width of the overlay
  inline const short getWidth() const {return width_;}
  //! sets the height of the overlay
  inline void setHeight(const short height){height_ = height;}
  //! returns the height of the overlay
  inline const short getHeight() const {return height_;}
  //! sets the description of the overlay
  inline void setDescription(const string desc){description_ = desc;}
  //! returns the description of the overlay
  inline const string getDescription() const {return description_;}  
  //! set orientation
  inline void setOrientation(Direction d){orientation_ = d;}
  //! get orientation
  inline Direction getOrientation(){return orientation_;}
  //! is the furniture already rendered?
  inline bool isRendered(){return rendered_;}
  
  //! renders the object in 3D
  virtual void render() const;
  //! renders the object in 2D
  virtual void render2D() const ;
  //! updates overlay status
  virtual void update() {rendered_ = true;};
  //! reset the overlay rendering state
  inline void reset(){rendered_ = false;}
  //! if overlay is found
  inline void find(bool found) {found_ = found;}
  //! is overlay found
  inline bool isFound() {return found_;}
  //! set walkability
  inline void setWalkable(bool what) {walkable_ = what;}
  //! is walkable
  inline bool isWalkable() {return walkable_;}
  //! set the 3D model
  inline void setModel(CGE::Model* model) { model_ = model; }
protected:
  //! the name
  string name_;
  //! the shortcut of the overlay
  string short_;
  //! the width of the overlay
  short width_;
  //! the height of the overlay
  short height_;
  //! the description of the overlay
  string description_;
  //! the orientation of the overlay
  Direction orientation_;
  //! is the overlay already rendered
  bool rendered_;
  //! is the overlay (trap) found
  bool found_;
  //! can someone walk on the overlay
  bool walkable_;
  //! the model of the creature
  CGE::Model* model_;
};
  
#endif