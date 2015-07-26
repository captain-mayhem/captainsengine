#ifndef BIDIR_H
#define BIDIR_H

#include "Unicode.h"

class UnicodeProperties;

class BiDir{
public:
  BiDir(UnicodeProperties* props) {mUniProps = props;}
  void determineBidiLevels(const UnicodeString& str);
protected:
  UnicodeProperties* mUniProps;
};

#endif
