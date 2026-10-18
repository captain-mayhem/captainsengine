#ifndef UTILITIES_H
#define UTILITIES_H

#include <cge_export.h>

#include <string>

namespace CGE{

class CGE_API Utilities{
  public:
    static void replaceWith(std::string& text, char oldCh, char newCh);
    static void replaceWith(std::string& text, const std::string& oldSub, const std::string& newSub);
};

}


#endif

