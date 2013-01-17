#ifndef PDF_READER_H
#define PDF_READER_H

#include <system/types.h>

namespace pdf{

struct Reference{
  Reference() : object(0), generation(0) {}
  uint32 object;
  uint16 generation;
};

}

class PdfDocument{

};

#include <string>
#include <io/Reader.h>

class PdfReader{
public:
  PdfReader(const std::string& file);
  PdfDocument* readDocument();
protected:
  void findBackwards(const std::string what);
  
  const char* getNextToken(bool breakAtSlash = true);
  int getNextInt();
  const char* getNextDictEntry();
  pdf::Reference getNextReference();
  char mBuffer[512];
  char* mCurrent;

  CGE::Reader* mReader;
};

#endif
