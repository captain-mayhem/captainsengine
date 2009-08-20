#ifndef PCDK_SCRIPT_H
#define PCDK_SCRIPT_H

#include <antlr3.h>
#include "pcdkLexer.h"
#include "pcdkParser.h"

class PcdkScript{
public:
  PcdkScript();
  ~PcdkScript();
  void parseProgram();
protected:
};

#endif
