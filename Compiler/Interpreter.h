#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>

#include "MethodArea.h"

class Interpreter{
public:
  Interpreter();
  ~Interpreter();
  void execute(std::string classname);
protected:
  MethodArea mMethodArea;
};

#endif