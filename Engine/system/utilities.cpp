#include "utilities.h"

using namespace CGE;

void Utilities::replaceWith(std::string& text, char oldCh, char newCh){
  for (size_t i = 0; i < text.size(); i++){
    if (text[i] == oldCh)
      text[i] = newCh;
  }
}

void Utilities::replaceWith(std::string& text, const std::string& oldSub, const std::string& newSub){
  for (size_t i = 0; i < text.size(); i++){
    size_t cmp;
    for (cmp = 0; cmp < text.size(); ++cmp){
      if (text[i] != oldSub[cmp])
        break;
    }
    if (cmp == oldSub.size()){
      //correct place found
      text.erase(i, cmp);
      text.insert(i, newSub);
      i += cmp;
    }
  }
}

