#include "pdfreader.h"
#include <io/TraceManager.h>
#include <io/Tracing.h>

int main(int argc, char** argv){
  CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
  if (argc <= 1){
    return -1;
  }
  PdfReader rdr(argv[1]);
  PdfDocument* doc = rdr.readDocument();
}