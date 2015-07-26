#include <string>
#include <iostream>

#include "InputProcessor.h"

using namespace std;

int main(int argc, char** argv){
  InputProcessor processor;
  string input;
  cout << "Talk to me" << endl;
  while(cin){
    cout << endl << ">";
    getline(cin, input);
    if (input.empty())
      continue;
    if (input[0] == '#'){
      cout << processor.getData().directQuery(input.substr(1).c_str());
      continue;
    }
    if (input == "q")
      break;
    string ret = processor.process(input);
    cout << ret << endl;
  }
  return 0;
}