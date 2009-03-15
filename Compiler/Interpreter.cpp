#include "Interpreter.h"

Interpreter::Interpreter(){

}

Interpreter::~Interpreter(){

}

void Interpreter::execute(std::string classname){
  unsigned classidx = mMethodArea.getClassIndex(classname);
  unsigned pc = mMethodArea.prepareMethod("main", "([Ljava/lang/String;)V", classidx);
  return;
}