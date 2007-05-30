//  ==================================================================
// |                         Hero Engine                              |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: dialog.h                                                   |
//  ==================================================================

#ifndef DIALOG_H
#define DIALOG_H

#include "gui.h"

namespace Gui{

//! a dialog field
class Dialog : public GuiElement {
public:
  //! Constructor
  Dialog();
  //! Destructor
  virtual ~Dialog();
  //! The render function
  virtual void render();
  //! Is the dialog clicked
  virtual bool isClicked(const ::Math::Vector2D& pos);
  //! Process the click
  virtual void process();
  //! Add an element
  /*! Position and Span of Dialog and GuiElement has to be set before adding elements
  */
  virtual void addElement(GuiElement* elem);
protected:
  std::list<GuiElement*> elements_;
};

}

#endif

