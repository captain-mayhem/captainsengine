//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Stefan Ruff                                                      |
// | stefan.ruff@gmx.net                                              |
// |------------------------------------------------------------------|
// | File: furniture.cpp                                              |
//  ==================================================================

#ifndef FURNITURE_HH
#define FURNITURE_HH

#include "field.h"
#include <string>

using std::string;

//! The Furniture in the world
class Furniture : public GameObject{
public:
	//! Constructor
	Furniture(){
		active_ = true;
		rendered_ = false;
    mModel = NULL;
	}

	//! Copy Constructor
	Furniture(const Furniture& f);

	//! Destructor
	~Furniture(){}
		
	void render() const;
	void render2D() const;
	void update();
	
	//! set abbreviation
	inline void setBrev(string brev){brev_=brev;}
	//! get abbreviation
	inline string getBrev(){return brev_;}
	//! set width
	inline void setWidth(short w){width_ = w;}
	//! get width
	inline short getWidth(){return width_;}
	//! set height
	inline void setHeight(short h){height_ = h;}
	//! get height
	inline short getHeight(){return height_;}
	//! set orientation
	inline void setOrientation(Direction d){orientation_ = d;}
	//! get orientation
	inline Direction getOrientation(){return orientation_;}
	//! is the furniture already rendered?
	inline bool isRendered(){return rendered_;}
	//! reset the furniture state
	inline void reset(){rendered_ = false;}
  //! sets a model
  void setModel(CGE::Model* mdl) { mModel = mdl; }

private:
	//! the orientation of the piece of furniture
	Direction orientation_;
	//! the abbreviation
	string brev_;
	//! the furniture width
	short width_;
	//! the furniture depth
	short height_;
	//! is the furniture already rendered
	bool rendered_;
  //! the model of the furniture
  CGE::Model* mModel;
};

//! less operator for STL storage
inline bool operator<(Furniture a, Furniture b){
  return a.getId() < b.getId();
}
//! equal operator
inline bool operator==(Furniture a, Furniture b){
  return a.getId() == b.getId();
}

#endif
