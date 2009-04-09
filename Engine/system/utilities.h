#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

namespace CGE{

class Utilities{
  public:
    static void replaceWith(std::string& text, char oldCh, char newCh);
};

}


#endif

