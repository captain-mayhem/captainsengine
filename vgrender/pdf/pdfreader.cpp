#include "pdfreader.h"

#include <io/BinFileReader.h>
#include <io/Tracing.h>

TR_CHANNEL(PDF_Reader);

#define ASSERT_TOKEN(tok) do{const char* read = getNextToken(); if (strcmp(tok, read) != 0) TR_ERROR("Token %s unexpected, should be %s", read, tok);} while(0)

PdfReader::PdfReader(const std::string& file){
  mReader = new CGE::BinFileReader(file);
}

PdfDocument* PdfReader::readDocument(){
  TR_USE(PDF_Reader);
  uint8 version[9];
  mReader->readBytes(version, 8);
  mReader->gotoEnd();
  //read trailer section
  int numObjects = 0;
  pdf::Reference root;
  findBackwards("trailer");
  ASSERT_TOKEN("trailer");
  ASSERT_TOKEN("<<");
  const char* key = getNextDictEntry();
  while(key != NULL){
    if (strcmp(key, "Size") == 0){
      numObjects = getNextInt();
    }
    else if (strcmp(key, "Root") == 0){
      root = getNextReference();
    }
    else
      TR_WARN("%s unexpected in trailer", key);
    key = getNextDictEntry();
  }
  ASSERT_TOKEN("startxref");
  unsigned crtOffset = (unsigned)getNextInt();
  //ASSERT_TOKEN("%%EOF");
  PdfDocument* doc = new PdfDocument(this, root, numObjects, crtOffset);
  return doc;
}

#define CHUNKSIZE 512

void PdfReader::findBackwards(const std::string what){
  uint8 chunk[CHUNKSIZE];
  while(true){
    mReader->skip(-CHUNKSIZE);
    mReader->readBytes(chunk, CHUNKSIZE);
    for (int i = CHUNKSIZE-1; i >= 0; --i){
      bool matches = true;
      for (unsigned j = 0 ; j < what.size(); ++j){
        if (chunk[i-j] != what[what.size()-1-j]){
          matches = false;
          break;
        }
      }
      if (matches){
        mReader->skip(-CHUNKSIZE+i-what.size());
       return;
      }
    }
    mReader->skip(-CHUNKSIZE);
  }
}

const char* PdfReader::getNextToken(bool breakAtSlash){
  char c;
  do {
    c = mReader->readChar();
  } while(isspace(c));
  mCurrent = mBuffer;
  while (true){
    if (!mReader->isWorking())
      break;
    if (isspace(c) || (breakAtSlash && c == '/' && mCurrent != mBuffer))
      break;
    *mCurrent = c;
    ++mCurrent;
    c = mReader->readChar();
  }
  *mCurrent = '\0';
  return mBuffer;
}

int PdfReader::getNextInt(){
  const char* tok = getNextToken();
  return atoi(tok);
}

const char* PdfReader::getNextDictEntry(){
  const char* tok;
  while (true){
    tok = getNextToken();
    //no more entries
    if (tok[0] == '>' && tok[1] == '>')
      return NULL;
    if (tok[0] == '/')
      return tok+1;
  }
}

pdf::Reference PdfReader::getNextReference(){
  TR_USE(PDF_Reader);
  pdf::Reference ret;
  ret.object = getNextInt();
  ret.generation = getNextInt();
  ASSERT_TOKEN("R");
  return ret;
}

void PdfReader::readCrt(unsigned crtOffset, pdf::CrossReferenceTable& crt){
  TR_USE(PDF_Reader);
  mReader->jumpTo(crtOffset);
  ASSERT_TOKEN("xref");
  return;
}
