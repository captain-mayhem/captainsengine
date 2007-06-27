//  ==================================================================
// |                         Hero Engine                              |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: filedialog.h                                               |
//  ==================================================================

#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include "dialog.h"

namespace Gui{

//! a dialog field
class FileDialog : public Dialog {
public:
  //! Constructor
  FileDialog();
  //! Destructor
  virtual ~FileDialog();
  //! Process the click
  virtual void process();
  //! The change directory callback
  static void changeDir(GuiElement* elem);
  //! The write filename callback
  static void writeFilename(GuiElement* elem);
  //! Get the absolute path to the file file (after being chosen)
  std::string getFile() const;
  //! Get the relative path to the file
  std::string getRelFile() const;
  //! Set the callback that is executed when OK button is pressed
  void setCbFunc(void (*click)(GuiElement*));
protected:
  //! Init the dialog with its elements
  void init();
  //! The working directory
  static std::string cwd_;
  //! The relative path
  static std::string path_;
  //! The callback
  void (*handleClicks_)(GuiElement*);

};

}

#endif

