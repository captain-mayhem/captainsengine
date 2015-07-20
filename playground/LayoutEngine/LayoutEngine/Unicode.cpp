#include "Unicode.h"

bool UnicodeString::setString(const char* utf8){
    unsigned char *p = (unsigned char*)utf8;
    
    unsigned ch;
    int x = 0;
    int l;
    while (*p){
      l = getLength(*p);
      switch (l){
      case 4:
        ch = (*p ^ 0xf0);
        break;
      case 3:
        ch = (*p ^ 0xe0);
        break;
      case 2:
        ch = (*p ^ 0xc0);
        break;
      case 1:
        ch = *p;
        break;
      default:
        return false;
      }
      ++p;
      int y;
      for (y = l; y > 1; y--)
      {
        ch <<= 6;
        ch |= (*p ^ 0x80);
        ++p;
      }
      mChars.push_back(ch);     
  }
  return false;
}

unsigned UnicodeString::getLength(unsigned char ch){
  ch >>= 3;
  // 6 => 0x7e
  // 5 => 0x3e
  if (ch == 0x1e)
    return 4;
  ch >>= 1;
  if (ch == 0xe)
    return 3;
  ch >>= 1;
  if (ch == 0x6)
    return 2;
  return 1;
}