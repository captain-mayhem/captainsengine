//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: creature.hh                                                |
//  ==================================================================

#ifndef CREATURE_HH
#define CREATURE_HH

#include <string>
#include <vector>
#include <queue>
#include "common.hh"
#include "gameobject.hh"
#include "field.hh"
#include "vector.hh"
#include "inventory.hh"

using std::string;
using std::vector;
using std::queue;

//! A creature in the game with all its attributes
//! A creature can be a hero or a monster
class Creature : public GameObject{
public:
  //! Default constructor
  Creature();
  //! Constructor which initializes all values
  /*! \param player The name of the player
   * \param name the name of the creature
   * \param type the character class of the creature
   * \param body the strenght
   * \param mind the intelligence
   * \param attack the attack strength
   * \param defend the defence capabilities
   * \param movement the number of dice for moving
   * \param id the character class id
   * \param position the position of the hero
  */
  Creature(const string& player, const string& name, const string& type, const char body, 
	const char mind, const char attack, const char defend, 
	const char movement, const short id, const Vector2D& position);
 
  //! Copy constructor
  Creature(const Creature& c);
  
	//! Destructor
  ~Creature();
  
	//! set creature name
  inline void setName(const string& name) {name_ = string(name);}
  
	//! get creature name
  inline const string getName() const {return name_;}
  
	//! set the type of the creature
  inline void setType(const string& type) {type_ = string(type);}
  
	//! get the type of the creature
  inline const string getType() const {return type_;}
  
	//! set the name of the player controlling the creature
  inline void setPlayer(const string& name) {player_ = string(name);}
  
	//! get the player's name
  inline const string getPlayer() const {return player_;}
  
	//! set attack strength
  inline void setAttack(const char points) {attack_ = points;}
  
	//! get attack value
  inline const short getAttack() const {return attack_;}
  
	//! set defence strength
  inline void setDefence(const char points) {defend_ = points;}
  
	//! get defence value
  inline const short getDefence() const {return defend_;}
  
	//! set body strength
  inline void setBody(const char points) {body_ = points;}
  
	//! get body points
  inline const short getBody() const {return body_;}

  //! set maximum body points
  inline void setMaxBody(const char points) {maxBody_ = points;}

  //! get maximum body points
  inline const short getMaxBody() const {return maxBody_;}
  
	//! set mind capabilities
  inline void setMind(const char points) {mind_ = points;}
  
	//! get mind points
  inline const short getMind() const {return mind_;}
 
  //! set maximum mind points
  inline void setMaxMind(const char points) {maxMind_ = points;}

  //! get maximum mind points
  inline const short getMaxMind() const {return maxMind_;}

	//! get number of movement dice
	inline const short getMove() const {return movement_;}
	
	//! set number of movement dice
	inline void setMove(const char dice) {movement_ = dice;}
	
	//! set a pointer to the last valid field position
  /*! if a creature trys to jump over another creature, this field
   *  would be an invalid resting position. So the backpointer
   *  has to be set to the last valid position
   */
  inline void setBackPointer(const Field* f) {bp_ = f;}
  
	//! returns the last valid resting field for the hero.
  inline const Field* getBackPointer() {return bp_;}
  
	//! returns all creature attributes as one long string
  /*! Only for client-server data transmission needed.
   */
  const string data(short posIdx);
  
	//! set the number of movement dice or fields the creature can use
  inline void setMovement(const char dice){movement_ = dice;}
  
	//! return the number of movement dice or fields
  inline const short getMovement() {return movement_;}
  
	//! inits a creature with the given data string.
  void init(const string& data);
  
	//! render creature
  virtual void render() const;
  
	//! render creature in 2D
  virtual void render2D() const;
  
	//! updates a creature's position
  //! interpolates a creature's position smoothly when moving
  virtual void update();
  
	//! adds a position to the positions that should be interpolated
  inline void addPosition(Vector3D pos){positions_.push(pos);}

	//! set look at vector
	inline void setLookAt(Vector3D look){lookAt_ = look;};

	//! get look at vector
	inline Vector3D getLookAt(){return lookAt_;};

	//! get x-rotation
	inline float getRot(){return xRot_;};
	
	//! set x-rotation
	inline void setRot(float rot){xRot_ = rot;};
  
  //! get a pointer to the inventory
  inline Inventory* getInventory(){return &items_;};

  //!set the creature sleeping
  inline void setSleeping(bool sleep){sleeping_ = sleep;}

  //! get the sleeping status
  inline bool isSleeping() {return sleeping_;}

  //! set the dice to move flag
  inline void setDiceToMove(bool doit) {diceToMove_ = doit;}

  //! must the creature dice to move
  inline bool diceToMove() {return diceToMove_;}
	
protected:
  //! the creature's name
  string name_;
  
	//! creature type
  string type_;
  
	//! the name of the creature
  string player_;
  
	//! number of movement dice/fields
  char movement_;
  
	//! attack value
  char attack_;
  
	//! defence value
  char defend_;
  
	//! body strength
  char body_;

  //! mamimum body strength
  char maxBody_;
  
	//! intelligence
  char mind_;

  //! mamimum intelligence
  char maxMind_;
  
	//! the backpointer to the last valid resting field
  const Field* bp_;
  
	//! the current 3D position
  Vector3D actPos_;
  
	//! over which fields it should interpolate movement
  queue<Vector3D> positions_;
	
	//! look at vector of the creature
	Vector3D lookAt_;

	//! camera rotations
	float xRot_;

  //! the inventory of the creature
  Inventory items_;

  //! is the creature sleeping
  bool sleeping_;

  //! dice to move
  bool diceToMove_;
};

//! less operator for STL storage
inline bool operator<(Creature a, Creature b){
  return a.getId() < b.getId();
}
//! equal operator
inline bool operator==(Creature a, Creature b){
  return a.getId() == b.getId();
}

#endif
