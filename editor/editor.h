#ifndef EDITOR_H
#define EDITOR_H

#include "system/engine.h"

class Editor{
public:
  ~Editor();
  static void init();
  inline static Editor* instance() {return edi_;}
  inline static void release() {SAFE_DELETE(edi_);}
protected:
    //Constructor
    Editor();
    //the instance
    static Editor* edi_;
};

#endif
