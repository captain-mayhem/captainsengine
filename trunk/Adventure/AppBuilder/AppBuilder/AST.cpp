#include "AST.h"

void IdentNode::append(const char* str) {
  std::string tmp(str);
  if (!mValue.empty() || tmp != " "){
    if (!mValue.empty()){
      char test = mValue[mValue.size()-1];
      //erase trailing whitespace
      if ((tmp == "-" || tmp == ":") && test == ' '){
        mValue.erase(mValue.size()-1);
        if (mValue.empty()){
          mValue = tmp;
          return;
        }
      }
      //do not allow adding of whitespace
      if (tmp == " "){
        if (test == '-')
          return;
      }
    }
    mValue += str;
  }
}
