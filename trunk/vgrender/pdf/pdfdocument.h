#ifndef PDF_DOCUMENT_H
#define PDF_DOCUMENT_H

#include <system/types.h>
#include <vector>
#include <map>

namespace pdf{

  struct Reference{
    Reference() : object(0), generation(0) {}
    bool operator<(const Reference& ref) const{
      if (object == ref.object)
        return generation < ref.generation;
      return object < ref.object;
    }
    uint32 object;
    uint16 generation;
  };

  struct CrtEntry{
    uint32 offset;
    uint16 generation;
    bool used;
  };

  struct CrossReferenceSection{
    uint32 object;
    uint32 numObjects;
    std::vector<CrtEntry> entries;
  };

  struct CrossReferenceTable{
    std::vector<CrossReferenceSection> sections;
    std::vector<CrtEntry> entries;
  };

  class Object{

  };
}

class PdfReader;

class PdfDocument{
public:
  PdfDocument(PdfReader* rdr, unsigned crtOffset);
private:
  unsigned getOffset(pdf::Reference& ref);
  pdf::Object* getObject(pdf::Reference& ref);
  PdfReader* mReader;
  pdf::CrossReferenceTable mCrt;
  std::map<pdf::Reference,pdf::Object*> mObjects;
};

#endif