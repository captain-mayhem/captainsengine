#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

namespace System{

class Utilities{
  public:
    static void replaceWith(std::string& text, char oldCh, char newCh);
};

}


#endif

