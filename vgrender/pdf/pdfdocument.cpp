#include "pdfdocument.h"

#include "pdfreader.h"

PdfDocument::PdfDocument(PdfReader* rdr, pdf::Reference& root, unsigned numObjects, unsigned crtOffset)
: mReader(rdr)
{
  mReader->readCrt(crtOffset, mCrt);
}