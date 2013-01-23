#ifndef PDF_DOCUMENT_H
#define PDF_DOCUMENT_H

#include <system/types.h>
#include <vector>

namespace pdf{

  struct Reference{
    Reference() : object(0), generation(0) {}
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
  };
}

class PdfReader;

class PdfDocument{
public:
  PdfDocument(PdfReader* rdr, pdf::Reference& root, unsigned numObjects, unsigned crtOffset);
private:
  PdfReader* mReader;
  pdf::CrossReferenceTable mCrt;
};

#endif