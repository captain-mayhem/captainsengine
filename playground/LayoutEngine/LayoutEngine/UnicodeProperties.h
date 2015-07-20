#ifndef UNICODE_PROPERTIES_H
#define UNICODE_PROPERTIES_H

#include <map>

#include "Unicode.h"

class UnicodeProperties{
public:
  enum BidiClass{
    UNKNOWN,

    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
    RIGHT_TO_LEFT_ARABIC,
    
    EUROPEAN_NUMBER,
    EUROPEAN_NUMBER_SEPARATOR,
    EUROPEAN_NUMBER_TERMINATOR,
    ARABIC_NUMBER,
    COMMON_NUMBER_SEPARATOR,
    NON_SPACING_MARK,
    BOUNDARY_NEUTRAL,

    PARAGRAPH_SEPARATOR,
    SEGMENT_SEPARATOR,
    WHITESPACE,
    OTHER_NEUTRALS,

    LEFT_TO_RIGHT_EMBEDDING,
    LEFT_TO_RIGHT_OVERRIDE,
    RIGHT_TO_LEFT_EMBEDDING,
    RIGHT_TO_LEFT_OVERRIDE,
    POP_DIRECTIONAL_FORMAT,
    LEFT_TO_RIGHT_ISOLATE,
    RIGHT_TO_LEFT_ISOLATE,
    FIRST_STRONG_ISOLATE,
    POP_DIRECTIONAL_ISOLATE,
  };

  UnicodeProperties();
  BidiClass getBidiClass(const UnicodeChar& chr);
protected:
  BidiClass strToBidi(const char* bidiclass);
  std::map<UnicodeChar, BidiClass> mBidiMap;
};

#endif
