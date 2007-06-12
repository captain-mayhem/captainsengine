#ifndef MENU_H
#define MENU_H

namespace Gui{
class GuiElement;
}

//! menu related callback functions
class Menu{
public:
  //! The add mesh callback function
  static void addMesh(Gui::GuiElement* elem);
  //! Load a mesh
  static void loadMesh(Gui::GuiElement* elem);
};

#endif

