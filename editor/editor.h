#ifndef EDITOR_H
#define EDITOR_H

#include "system/engine.h"

namespace Math{
class Arcball;
}

class Editor{
public:
  ~Editor();
  static void init();
  inline static Editor* instance() {return edi_;}
  inline static void release() {SAFE_DELETE(edi_);}
  static void keypress(int key);
  //! update editor
  void update();
protected:
  //! Constructor
  Editor();
  //! key press
  void _keypress(int key);
  //! the instance
  static Editor* edi_;
  //! the arcball
  Math::Arcball* arcball_;
};

#endif
