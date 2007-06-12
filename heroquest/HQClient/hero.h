//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: hero.hh                                                    |
//  ==================================================================

#ifndef HERO_HH
#define HERO_HH

#include <string>
#include <vector>
#include <queue>
#include "common.h"
#include "creature.h"
#include "field.h"
#include "math/vector.h"

using std::string;
using std::vector;
using std::queue;

//! The hero in the game with all ist attributes
class Hero : public Creature{
public:
  //! Default constructor
  Hero();
  //! Constructor which initializes all values
  /*! \param player The name of the player
   * \param name the name of the hero
   * \param type the character class of the hero
   * \param body the strenght
   * \param mind the intelligence
   * \param attack the attack strength
   * \param defend the defence capabilities
   * \param money the amount of money for the hero
   * \param movement the number of dice for moving
   * \param id the character class id
   * \param position the position of the hero
  */
  Hero(const string& player, const string& name, const string& type, const char body, 
	const char mind, const char attack, const char defend, const int money, 
	const char movement, const short id, const Vector2D& position, short spellClasses);
  //! Copy Constructor
  Hero(const Hero& h);
  //! Destructor
  ~Hero();
  //! change the amount of money the hero has
  inline void changeMoney(const int amount){(amount+money_ > 0) ? money_ += amount : money_ = 0;}
  //! returns the hero's money
  inline const int getMoney() const {return money_;}
  //! set number of spell classes
  inline void setSpellClasses(short number){spellClasses_ = number;}
  //! get the number of spell classes
  inline short getSpellClasses(){return spellClasses_;}
  //! creates a hero
  /*! \param path where the hero should be saved
   *  \param heros a vector with all available hero types and their attributes
   *  \param playerName the name of the player
   */
  static void Create(const string& path, const vector<Hero>& heros, const string& playerName);
  //! creates a hero from the gui
  static void createFromGui(Gui::GuiElement* elem);

  //! loads a hero from a file
  bool load(const string& filename);
  //! saves a hero to disk
  void write(const string& p) const;
  //! all hero attributes in one string
  const string data(short posIdx);
  //! renders a hero
  virtual void render() const;
  //! render a hero in 2D
  virtual void render2D() const;
  //! let the hero win the game
  void win();
private:
  //! amount of money
  int money_;
  //! the number of spell classes
  short spellClasses_;
};

//! less operator for STL storage
inline bool operator<(Hero a, Hero b){
  return a.getId() < b.getId();
}
//! equal operator
inline bool operator==(Hero a, Hero b){
  return a.getId() == b.getId();
}

#endif
