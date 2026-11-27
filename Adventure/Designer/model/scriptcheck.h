#ifndef SD_SCRIPTCHECK_H
#define SD_SCRIPTCHECK_H

#include <string>
#include <vector>

#include "project.h"

namespace sd{

//! a place in a script the game can't read
struct ScriptError{
  //! from 1
  int line = 0;
  //! from 0, in bytes
  int column = 0;
  std::string message;
};

//! checks a script (Windows-1252) with the game's own parser: the ANTLR
//! grammar of PCDK scripts, or Lua's compiler; empty if the game reads it
std::vector<ScriptError> checkScript(const std::string& text, ScriptLang language);

}

#endif
