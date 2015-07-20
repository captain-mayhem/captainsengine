#include "Bidir.h"
#include "UnicodeProperties.h"

struct BidiEntry{
  unsigned idx;
  UnicodeProperties::BidiClass bidi;
};

void BiDir::determineBidiLevels(const UnicodeString& str){
  std::vector<BidiEntry> mEntries;
  mEntries.resize(str.size());
  for (unsigned i = 0; i < str.size(); ++i){

  }
}