//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: bitset.hh                                                  |
//  ==================================================================

#ifndef BITSET_H
#define BITSET_H

#include <map>

using std::map;

namespace System {
//! stores up to eight bits in one byte. 
/*! It is a simple bitset class that has the following advantages: 
 * It takes less memory than the bitset class from STL. Moveover,
 * memory management overhead is lower than in the STL_bitset.
 */
class Bitset {
public: 
  //! Constructor
  Bitset();
  //! Copy
  Bitset(const Bitset& b);
  //! Destructor
  ~Bitset();
  //! Set a bit
  //! \param position The bit position to be set
  void set(const short position);
  //! Unset a bit
  //! \param position The bit position to be unset
  void unset(const short position);
  //! Tests, if a bit is set
  //! \param position the bit to be set
  bool test(const short position);
  //! Counts the number of the bits that are set
  short numSetBits();
  //! returns the whole byte
  inline char getData() const {return data_;}
  //! sets the whole byte
  inline void setData(char data) { data_ = data;}
private:
  //! Here are the bits stored
  char data_;
  //! Maps the specified bit position to the actual bit position
  map<short,unsigned char> idx_;
};

}

#endif
