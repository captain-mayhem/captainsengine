//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Stefan Ruff                                                      |
// | stefan.ruff@gmx.net                                              |
// |------------------------------------------------------------------|
// | File: monster.hh                                                 |
//  ==================================================================

#ifndef MONSTER_HH
#define MONSTER_HH

#include <string>
#include <vector>
#include <queue>
#include "common.hh"
#include "creature.hh"
#include "field.hh"
#include "vector.hh"

using std::string;
using std::vector;
using std::queue;

//! The monster in the game with all ist attributes
class Monster : public Creature{
public:
  //! Default constructor
  Monster();
  
	//! Constructor which initializes all values
  /*! \param player The name of the player
   * \param name the name of the monster
   * \param type of the monster
   * \param body the strenght
   * \param mind the intelligence
   * \param attack the attack strength
   * \param defend the defence capabilities
   * \param movement the number of fields the monster can move
   * \param id the character class id
   * \param position the position of the monster
  */
  Monster(const string& player, const string& name, const string& type, const char body, 
	const char mind, const char attack, const char defend, const char movement, const short id, const Vector2D& position);

  //! Copy Constructor
  Monster(const Monster& m);

	//! Destructor
  ~Monster();

	//! Set abbreviation
	void setBrev(string brev){
		brev_ = brev;
	}

	//! get abbreviation
	string getBrev(){
		return brev_;
	}

  //! render monster
  virtual void render() const;

  //! render monster in 2D
  virtual void render2D() const;

  //! sets if a monster can move
  inline void canMove(bool flag=true){canMove_ = flag;}

  //! returns if a monster can move
  inline bool movable() {return canMove_;}

private:
  //! 
  bool canMove_;
	//!
	string brev_;
};

//! less operator for STL storage
inline bool operator<(Monster a, Monster b){
  return a.getId() < b.getId();
}
//! equal operator
inline bool operator==(Monster a, Monster b){
  return a.getId() == b.getId();
}

#endif
