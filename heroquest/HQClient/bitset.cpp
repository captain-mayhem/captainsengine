//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: bitset.cpp                                                 |
//  ==================================================================

#include <map>
#include "assert.h" 
#include "bitset.h"

using std::map;

//constructor
Bitset::Bitset(){
  //maps a position index to a specific bit
  idx_[0] = 0x01;
  idx_[1] = 0x02;
  idx_[2] = 0x04;
  idx_[3] = 0x08;
  idx_[4] = 0x10;
  idx_[5] = 0x20;
  idx_[6] = 0x40;
  idx_[7] = 0x80;
  data_ = '\0';
}

//copy
Bitset::Bitset(const Bitset& b){
  data_ = b.data_;
  idx_ = map<short,unsigned char>(b.idx_);
}

//destructor
Bitset::~Bitset(){
}

//set a bit
void Bitset::set(const short position){
  assert(position < 8 && position >= 0);
  data_ |= idx_[position];
}

// unset a bit
void Bitset::unset(const short position){
  assert(position < 8 && position >= 0);
  data_ &= !idx_[position];
}

// test, if bit is set
bool Bitset::test(const short position){
  assert(position < 8 && position >= 0);
  return (data_ & idx_[position]);
}

// number of set bits
short Bitset::numSetBits(){
  short counter = 0;
  for (int i = 0; i < 8; i++){
    if (test(i))
      counter++;
  }
  return counter;
}
