#include "utilities.h"

using namespace System;

void Utilities::replaceWith(std::string& text, char oldCh, char newCh){
  for (size_t i = 0; i < text.size(); i++){
    if (text[i] == oldCh)
      text[i] = newCh;
  }
}

