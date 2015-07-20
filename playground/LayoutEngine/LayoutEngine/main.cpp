#include "Unicode.h"
#include "Bidir.h"
#include "UnicodeProperties.h"

int main(int argc, char** argv){
  UnicodeProperties props;
  UnicodeString us(argv[1]);
  BiDir bidi(&props);
  bidi.determineBidiLevels(us);
  return 0;
}
