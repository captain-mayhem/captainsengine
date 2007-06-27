#ifndef EDITOR_H
#define EDITOR_H

#include "system/engine.h"
#include "math/vector.h"

namespace Math{
class Arcball;
}

class Editor{
public:
  //The editing planes
  enum Plane{
    XZ,
    XY,
    YZ
  };
  enum Editmode{
    Translation,
    Rotation,
  };
  ~Editor();
  static void init();
  inline static Editor* instance() {return edi_;}
  inline static void release() {SAFE_DELETE(edi_);}
  static void keypress(int key);
  static void mouseDown(int x, int y, int button);
  static void mouseUp(int x, int y, int button);
  //! update editor
  void update();
  //! get grid offset
  inline float getGridOffset() const {return gridOffset_;}
  //! set the edit plane
  inline void setEditPlane(Plane p) {editPlane_ = p;}
  //! get the edit plane
  inline Plane getEditPlane() const {return editPlane_;}
protected:
  //! Constructor
  Editor();
  //! key press
  void _keypress(int key);
  //! mouse down
  void _mouseDown(int x, int y, int button);
  //! mouse up
  void _mouseUp(int x, int y, int button);
  //! the instance
  static Editor* edi_;
  //! the arcball
  Math::Arcball* arcball_;
  //! the last position
  Math::Vector2D lastPos_;
  //! the grid offset
  float gridOffset_;
  //! the grid step
  float gridStep_;
  //! the rotation step
  float rotationStep_;
  //! the editing plane
  Plane editPlane_;
  //! the edit mode
  Editmode editMode_;
};

#endif
