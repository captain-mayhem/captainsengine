
#include "menu.h"
#include "graphics.h"

#include "gui/gui.h"
#include "gui/filedialog.h"
#include "system/engine.h"

using namespace Gui;

//! The add mesh callback function
void Menu::addMesh(GuiElement* elem){
  FileDialog* fd = new FileDialog();
  fd->setCbFunc(loadMesh);
  System::Engine::instance()->addGuiListener(fd);
}

//! Load a mesh
void Menu::loadMesh(Gui::GuiElement* elem){
  FileDialog* fd = dynamic_cast<FileDialog*>(elem->getParent());
  Graphic::instance()->addMesh(fd->getFile());
}

