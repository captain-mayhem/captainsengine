
#include "menu.h"
#include "graphics.h"
#include "editor.h"

#include "gui/gui.h"
#include "gui/filedialog.h"
#include "gui/listbox.h"
#include "gui/dropdown.h"
#include "system/engine.h"
#include "mesh/mesh.h"
#include "mesh/model.h"
#include "renderer/texture.h"

using namespace Gui;
using Math::Vector2D;
using MeshGeo::Mesh;
using MeshGeo::Model;
using Graphics::Texture;

//! The add mesh callback function
void Menu::addMesh(GuiElement* elem){
  FileDialog* fd = new FileDialog();
  fd->setCbFunc(loadMesh);
  System::Engine::instance()->addGuiListener(fd);
}

//! Load a mesh
void Menu::loadMesh(Gui::GuiElement* elem){
  FileDialog* fd = dynamic_cast<FileDialog*>(elem->getParent());
  Graphic::instance()->addMesh(fd->getRelFile());
}

//! The add model callback
void Menu::addModel(Gui::GuiElement* elem){
  //Dialog
  Dialog* dia = new Dialog();
  //Meshes
  ListBox* lb = new ListBox();
  lb->setPosition(Vector2D(50, 100));
  lb->setSpan(Vector2D(400,600));
  vector<Mesh*> meshes = Graphic::instance()->getScene().getMeshes();
  for (unsigned i = 0; i < meshes.size(); i++){
    lb->addEntry(meshes[i]->getName());
  }
  dia->addElement(lb);
  //OK button
  PDButton* ok = new PDButton();
  ok->setPosition(Vector2D(575, 50));
  ok->setSpan(Vector2D(75,18));
  ok->setText("  OK");
  ok->setCbFunc(setModel);
  dia->addUnscaledElement(ok);
  System::Engine::instance()->addGuiListener(dia);
}

//! The set model callback
void Menu::setModel(Gui::GuiElement* elem){
  Dialog* dia = dynamic_cast<Dialog*>(elem->getParent());
  ListBox* lb = dynamic_cast<ListBox*>(dia->getElement(0));
  int sel = lb->selected();
  if (sel >= 0){
    Graphic::instance()->addModel(sel);
  }
}

//! The save scene callback
void Menu::save(Gui::GuiElement* elem){
  FileDialog* fd = new FileDialog();
  fd->setCbFunc(saveScene);
  System::Engine::instance()->addGuiListener(fd);
}

//! The save scene callback
void Menu::saveScene(Gui::GuiElement* elem){
  FileDialog* fd = dynamic_cast<FileDialog*>(elem->getParent());
  Graphic::instance()->getScene().save(fd->getFile());
}

//! The load scene callback
void Menu::load(Gui::GuiElement* elem){
  FileDialog* fd = new FileDialog();
  fd->setCbFunc(loadScene);
  System::Engine::instance()->addGuiListener(fd);
}
  
//! The load scene callback
void Menu::loadScene(Gui::GuiElement* elem){
  FileDialog* fd = dynamic_cast<FileDialog*>(elem->getParent());
  Graphic::instance()->getScene().load(fd->getFile());
}

//! The add texture callback
void Menu::addTexture(Gui::GuiElement* elem){
  FileDialog* fd = new FileDialog();
  fd->setCbFunc(loadTexture);
  System::Engine::instance()->addGuiListener(fd);
}
  
//! The set texture callback
void Menu::loadTexture(Gui::GuiElement* elem){
  FileDialog* fd = dynamic_cast<FileDialog*>(elem->getParent());
  Graphic::instance()->addTexture(fd->getRelFile());
}

//! the model properties callback
void Menu::properties(Gui::GuiElement* elem){
  //Dialog
  Dialog* dia = new Dialog();
  //Assign texture button
  PDButton* assTex = new PDButton();
  assTex->setPosition(Vector2D(20, 730));
  //assTex->setSpan(Vector2D(75,18));
  assTex->setText("Assign Texture");
  assTex->setCbFunc(assignTexture);
  dia->addUnscaledElement(assTex);
  
  /*
  ListBox* lb = new ListBox();
  lb->setPosition(Vector2D(50, 100));
  lb->setSpan(Vector2D(400,600));
  vector<Mesh*> meshes = Graphic::instance()->getScene().getMeshes();
  for (unsigned i = 0; i < meshes.size(); i++){
    lb->addEntry(meshes[i]->getName());
  }
  dia->addElement(lb);
  */
  //Close button
  PDButton* close = new PDButton();
  close->setPosition(Vector2D(575, 50));
  close->setSpan(Vector2D(75,18));
  close->setText(" Close");
  dia->addUnscaledElement(close);
  System::Engine::instance()->addGuiListener(dia);
}

//! the assign texture button
void Menu::assignTexture(Gui::GuiElement* elem){
  //Dialog
  Dialog* dia = new Dialog();
  //Textures
  ListBox* lb = new ListBox();
  lb->setPosition(Vector2D(50, 100));
  lb->setSpan(Vector2D(400,600));
  vector<Texture*> textures = Graphic::instance()->getScene().getTextures();
  for (unsigned i = 0; i < textures.size(); i++){
    lb->addEntry(textures[i]->getName());
  }
  dia->addElement(lb);
  //Texture stages
  DropDownButton* stages = new DropDownButton();
  stages->setPosition(Vector2D(500, 680));
  dia->addUnscaledElement(stages);
  stages->calcDDPos(1);
  stages->setText("Stage 0");
  stages->addEntry("Stage 0");
  stages->addEntry("Stage 1");
  stages->addEntry("Stage 2");
  stages->addEntry("Stage 3");
  stages->addEntry("Stage 4");
  stages->addEntry("Stage 5");
  stages->addEntry("Stage 6");
  stages->addEntry("Stage 7");
  //OK button
  PDButton* ok = new PDButton();
  ok->setPosition(Vector2D(575, 50));
  ok->setSpan(Vector2D(75,18));
  ok->setText("  OK");
  ok->setCbFunc(assignTextureDialog);
  dia->addUnscaledElement(ok);
  System::Engine::instance()->addGuiListener(dia);
}

//! the assign texture dialog handler
void Menu::assignTextureDialog(Gui::GuiElement* elem){
  //current model needed
  Model* mdl = Graphic::instance()->getCurrModel();
  if (!mdl)
    return;
  //get the chosen texture
  Texture* tex = NULL;
  Dialog* dia = dynamic_cast<Dialog*>(elem->getParent());
  ListBox* lb = dynamic_cast<ListBox*>(dia->getElement(0));
  int sel = lb->selected();
  if (sel >= 0){
    tex = Graphic::instance()->getScene().getTextures()[sel];
  }
  if (!tex)
    return;
  //get the texture stage
  DropDownButton* dd = dynamic_cast<DropDownButton*>(dia->getElement(1));
  int stage = dd->getSelection();
  //assign the texture
  mdl->assignTexture(tex,stage);
}

void Menu::options(Gui::GuiElement* elem){
  //Dialog
  Dialog* dia = new Dialog();
  System::Engine::instance()->addGuiListener(dia);

  //The plane chooser
  DropDownButton* planes = new DropDownButton();
  planes->setPosition(Vector2D(20, 680));
  dia->addUnscaledElement(planes);
  planes->calcDDPos(1);
  Editor::Plane plane = Editor::instance()->getEditPlane();
  string label;
  if (plane == Editor::XZ)
    label = "XZ Plane";
  else if (plane == Editor::XY)
    label = "XY Plane";
  else if (plane == Editor::YZ)
    label = "YZ Plane";
  planes->setText(label);
  planes->addEntry("XZ Plane");
  planes->addEntry("XY Plane");
  planes->addEntry("YZ Plane");

  //OK button
  PDButton* ok = new PDButton();
  ok->setPosition(Vector2D(575, 50));
  ok->setSpan(Vector2D(75,18));
  ok->setText("  OK");
  ok->setCbFunc(changeOptions);
  dia->addUnscaledElement(ok);
}

void Menu::changeOptions(Gui::GuiElement* elem){
  Dialog* dia = dynamic_cast<Dialog*>(elem->getParent());
  //The edit plane
  DropDownButton* dd = dynamic_cast<DropDownButton*>(dia->getElement(0));
  Editor::Plane plane = (Editor::Plane)dd->getSelection();
  Editor::instance()->setEditPlane(plane);
}


