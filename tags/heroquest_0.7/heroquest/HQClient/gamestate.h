//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: gamestate.hh                                               |
//  ==================================================================

#ifndef GAMESTATE_HH
#define GAMESTATE_HH

#include <queue>

namespace Gui{
class DropDownButton;
}

//! The states the game can be in
enum State{
  INIT,
  PREPARE,
  RUN
};

//! Manages the state of the game
class GameState{
public:
  //! Constructor
  GameState();
  //! Destructor
  ~GameState();
  //! sets the state of the game
  inline void setState(State status) {status_ = status;}
  //! returns the gamestate
  inline State getState() const {return status_;}
  //! sets the number of moves a player has
  inline void setMoves(short move){moves_ = move;}
  //! returns the number of moves
  inline const short getMoves() {return moves_;}
  //! sets how many damage is pending
  inline void setPendingDamage(short damage) { pending_damage_ = damage;}
  //! gets the pending damage
  inline short getPendingDamage() {return pending_damage_;}
  //! inits the game
  void init();
  //! starts the game
  bool start();
  //! is called every frame to keep the game running
  void run();
  //! ends the game
  void end();
  //! gets the position of the next creature that is controlled by you
  Vector2D getNextCreaturePos();
  //! choose the packages and levels that are available
  void choosePackage(Gui::DropDownButton* pack, Gui::DropDownButton* level);
  //! add a random number to repository
  void addRandom(int number);
  //! get a cached random number
  inline int getRandomNumber() {int res = randoms_.front(); randoms_.pop(); return res;}
private:
  //! the game state
  State status_;
  //! number of moves
  short moves_;
  //! if damage is pending
  short pending_damage_;
  //! the cached random numbers
  std::queue<int> randoms_;
};

extern class GameState game;

#endif
