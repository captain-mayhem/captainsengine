#include "pdfdocument.h"

#include "pdfreader.h"
#include <io/Tracing.h>

TR_CHANNEL(PDF_Document);

PdfDocument::PdfDocument(PdfReader* rdr, unsigned crtOffset)
: mReader(rdr)
{
  pdf::Reference root = mReader->readCrt(crtOffset, mCrt);
  pdf::Object* obj = getObject(root);
}

pdf::Object* PdfDocument::getObject(pdf::Reference& ref){
  pdf::Object* ret = mObjects[ref];
  if (ret == NULL){
    unsigned offset = getOffset(ref);
    ret = mReader->readObject(offset, ref);
    mObjects[ref] = ret;
  }
  return ret;
}

unsigned PdfDocument::getOffset(pdf::Reference& ref){
  TR_USE(PDF_Document);
  pdf::CrtEntry& entry = mCrt.entries[ref.object];
  if (entry.generation != ref.generation)
    TR_ERROR("Generation mismatch for entry %i: epected %i got %i", ref.object, ref.generation, entry.generation);
  if (!entry.used)
    TR_ERROR("Unused entry %i gen %i referenced", ref.object, ref.generation);
  return entry.offset;
}
