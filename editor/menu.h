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
  //! The add model callback
  static void addModel(Gui::GuiElement* elem);
  //! The set model callback
  static void setModel(Gui::GuiElement* elem);
  //! The save scene callback
  static void save(Gui::GuiElement* elem);
  //! The save scene callback
  static void saveScene(Gui::GuiElement* elem);
  //! The load scene callback
  static void load(Gui::GuiElement* elem);
  //! The load scene callback
  static void loadScene(Gui::GuiElement* elem);
  //! The add texture callback
  static void addTexture(Gui::GuiElement* elem);
  //! The set texture callback
  static void loadTexture(Gui::GuiElement* elem);
  //! the model properties callback
  static void properties(Gui::GuiElement* elem);
  //! the assign texture button
  static void assignTexture(Gui::GuiElement* elem);
  //! the assign texture dialog handler
  static void assignTextureDialog(Gui::GuiElement* elem);
  //! the options button
  static void options(Gui::GuiElement* elem);
  //! the change options button
  static void changeOptions(Gui::GuiElement* elem);
};

#endif

