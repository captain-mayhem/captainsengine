#include "UnicodeProperties.h"

UnicodeProperties::UnicodeProperties(){
  FILE* f = fopen("data/UnicodeData.txt", "r");
  char tmp[512];
  while(fgets(tmp, 512, f)){
    char* val = strtok(tmp, ";");
    UnicodeChar chr = strtoul(val, NULL, 16);
    char* name = strtok(NULL, ";");
    char* u1 = strtok(NULL, ";");
    char* u2 = strtok(NULL, ";");
    char* bidi = strtok(NULL, ";");
    mBidiMap[chr] = strToBidi(bidi);
  }
  fclose(f);
}

UnicodeProperties::BidiClass UnicodeProperties::strToBidi(const char* bidiclass){
  if (strcmp(bidiclass, "L") == 0)
    return LEFT_TO_RIGHT;
  if (strcmp(bidiclass, "R") == 0)
    return RIGHT_TO_LEFT;
  if (strcmp(bidiclass, "AL") == 0)
    return RIGHT_TO_LEFT_ARABIC;
  if (strcmp(bidiclass, "EN") == 0)
    return EUROPEAN_NUMBER;
  if (strcmp(bidiclass, "ES") == 0)
    return EUROPEAN_NUMBER_SEPARATOR;
  if (strcmp(bidiclass, "ET") == 0)
    return EUROPEAN_NUMBER_TERMINATOR;
  if (strcmp(bidiclass, "AN") == 0)
    return ARABIC_NUMBER;
  if (strcmp(bidiclass, "CS") == 0)
    return COMMON_NUMBER_SEPARATOR;
  if (strcmp(bidiclass, "NSM") == 0)
    return NON_SPACING_MARK;
  if (strcmp(bidiclass, "BN") == 0)
    return BOUNDARY_NEUTRAL;
  if (strcmp(bidiclass, "B") == 0)
    return PARAGRAPH_SEPARATOR;
  if (strcmp(bidiclass, "S") == 0)
    return SEGMENT_SEPARATOR;
  if (strcmp(bidiclass, "WS") == 0)
    return WHITESPACE;
  if (strcmp(bidiclass, "ON") == 0)
    return OTHER_NEUTRALS;
  if (strcmp(bidiclass, "LRE") == 0)
    return LEFT_TO_RIGHT_EMBEDDING;
  if (strcmp(bidiclass, "LRO") == 0)
    return LEFT_TO_RIGHT_OVERRIDE;
  if (strcmp(bidiclass, "RLE") == 0)
    return RIGHT_TO_LEFT_EMBEDDING;
  if (strcmp(bidiclass, "RLO") == 0)
    return RIGHT_TO_LEFT_OVERRIDE;
  if (strcmp(bidiclass, "PDF") == 0)
    return POP_DIRECTIONAL_FORMAT;
  if (strcmp(bidiclass, "LRI") == 0)
    return LEFT_TO_RIGHT_ISOLATE;
  if (strcmp(bidiclass, "RLI") == 0)
    return RIGHT_TO_LEFT_ISOLATE;
  if (strcmp(bidiclass, "FSI") == 0)
    return FIRST_STRONG_ISOLATE;
  if (strcmp(bidiclass, "PDI") == 0)
    return POP_DIRECTIONAL_ISOLATE;
  return UNKNOWN;
}

UnicodeProperties::BidiClass UnicodeProperties::getBidiClass(const UnicodeChar& chr){
  return LEFT_TO_RIGHT;
}