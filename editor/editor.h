#ifndef EDITOR_H
#define EDITOR_H

#include <map>
#include "system/engine.h"
#include "math/vector.h"

namespace CGE{
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
  enum Attribs{
    GROUND = 1000,
    WALL = 1001,
    STARTPOS = 1002,
    DOOR = 1003,
    WALLPART = 1004,
    MONSTER = 1005,
    FURNITURE = 1006,
    OVERLAY = 1007,
    SCRIPT = 1008,
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
  //! get the string to an attribute
  std::string attribString(int attrib);
  //! get the attribute to a string
  int attribValue(const std::string& attrib);
  //! get grid step
  inline void setGridStep(float step) {gridStep_=step;}
  //! set grid step
  inline float getGridStep() {return gridStep_;}
  //! add model to load at startup
  inline void addStartupModel(std::string const& name){ mLoadModels.push_back(name); }
  //! startup after renderer is initialized
  void startup();
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
  CGE::Arcball* arcball_;
  //! the last position
  CGE::Vector2D lastPos_;
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
  //! convert value to attribute
  std::map <int,std::string> vToa_;
  //! convert attribute to value
  std::map <std::string,int> aTov_;
  //! models to load at startup
  std::vector<std::string> mLoadModels;
};

#endif
