//  ==================================================================
// |                         Hero Engine                              |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: filedialog.cpp                                             |
//  ==================================================================

#include "filedialog.h"
#include "listbox.h"

#include "../system/engine.h"
#include "../system/file.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"

using namespace Gui;
using Graphics::Color;
using System::Filesystem;

#ifdef WIN32
#define SEPARATOR "\\"
#endif
#ifdef UNIX
#define SEPARATOR "/"
#endif

string FileDialog::cwd_ = Filesystem::getCwd();
string FileDialog::path_ = ".";

//! Constructor
FileDialog::FileDialog(){
  init();
}
  
//! Destructor
FileDialog::~FileDialog(){
}

void FileDialog::init(){
  //Directories
  ListBox* dir = new ListBox();
  dir->setPosition(Vector2D(50, 100));
  dir->setSpan(Vector2D(400, 600));
  dir->addEntries(Filesystem::getDirectories(cwd_+SEPARATOR+path_));
  dir->setCbFunc(&changeDir);
  addElement(dir);
  //Files
  ListBox* file = new ListBox();
  file->setPosition(Vector2D(564, 100));
  file->setSpan(Vector2D(400, 600));
  file->addEntries(Filesystem::getFiles(cwd_+SEPARATOR+path_));
  file->setCbFunc(&writeFilename);
  addElement(file);
  //Input field
  InputField* inp = new InputField();
  inp->setOpacity(0.8);
  inp->setPosition(Vector2D(50, 50));
  inp->setSpan(Vector2D(300,18));
  addUnscaledElement(inp);
  //OK button
  PDButton* ok = new PDButton();
  ok->setPosition(Vector2D(575, 50));
  ok->setSpan(Vector2D(75,18));
  ok->setText("  OK");
  addUnscaledElement(ok);
  //Abort button
  PDButton* abort = new PDButton();
  abort->setPosition(Vector2D(700, 50));
  abort->setSpan(Vector2D(75,18));
  abort->setText(" Abort");
  addUnscaledElement(abort);
}

//! Process an event
void FileDialog::process(){
}

//! The change directory callback
void FileDialog::changeDir(GuiElement* elem){
  Dialog* dia = dynamic_cast<Dialog*>(elem->getParent());
  ListBox* dirs = dynamic_cast<ListBox*>(dia->getElement(0));
  string dir = dirs->selectedItem();
  //collapse path when .. is chosen and the situation is appropriate
  if (dir == ".." && path_.length() > 1 && path_[path_.length()-1] != '.'){
    int n = path_.find_last_of(SEPARATOR);
    if (n != -1)
      path_.erase(n);
    else
      path_ += ".";
  }
  else
    path_ += SEPARATOR+dir;
  dirs->clear();
  dirs->addEntries(Filesystem::getDirectories(cwd_+SEPARATOR+path_));
  //update file list
  ListBox* files = dynamic_cast<ListBox*>(dia->getElement(1));
  files->clear();
  files->addEntries(Filesystem::getFiles(cwd_+SEPARATOR+path_));
}

//! The write filename callback
void FileDialog::writeFilename(GuiElement* elem){
  FileDialog* dia = dynamic_cast<FileDialog*>(elem->getParent());
  InputField* inp = dynamic_cast<InputField*>(dia->getElement(2));
  ListBox* files = dynamic_cast<ListBox*>(dia->getElement(1));
  string file = files->selectedItem();
  inp->setText(file);
}

//! Get the path and file (after being chosen)
std::string FileDialog::getFile() const{
  InputField* inp = dynamic_cast<InputField*>(getElement(2));
  return cwd_+SEPARATOR+path_+SEPARATOR+inp->getText();
}

//! Get the relative path to the file
std::string FileDialog::getRelFile() const{
  InputField* inp = dynamic_cast<InputField*>(getElement(2));
  return path_+SEPARATOR+inp->getText();
}

//! Set the callback that is executed when OK button is pressed
void FileDialog::setCbFunc(void (*click)(GuiElement*)){
  PDButton* ok = dynamic_cast<PDButton*>(getElement(3));
  ok->setCbFunc(click);
}

