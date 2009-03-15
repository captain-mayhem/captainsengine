

#include "Interpreter.h"

int main(int argc, char* argv[])
{
  char* filename = "test/Test";
  if (argc > 1){
    filename = argv[1];
  }
  
  Interpreter interp;
  interp.execute(filename);
	return 0;
}

