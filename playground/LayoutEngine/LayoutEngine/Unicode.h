#ifndef UNICODE_H
#define UNICODE_H

#include <vector>

typedef unsigned UnicodeChar;

class UnicodeString{
public:
  UnicodeString(const char* utf8) {setString(utf8);}
  bool setString(const char* utf8);
  unsigned size() const {return mChars.size();}
  UnicodeChar& operator[](int i) {return mChars[i];}
protected:
  unsigned getLength(unsigned char ch);
  std::vector<UnicodeChar> mChars;
};

#endif
