//  ==================================================================
// |                         Captains Engine                          |
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
  virtual bool isClicked(const ::CGE::Vector2D& pos);
  //! Process the click
  virtual void process();
  //! Add an element
  /*! Position and Span of Dialog and GuiElement has to be set before adding elements
  */
  virtual void addElement(GuiElement* elem);
  //! Add an unscaled element
  /*! Position and Span of Dialog and GuiElement has to be set before adding elements
  */
  virtual void addUnscaledElement(GuiElement* elem);
  //! Add an unscaled element
  /*! Position and Span of Dialog and GuiElement has to be set before adding elements
  */
  virtual void addOriginalElement(GuiElement* elem);
  //! Prepares the position of a gui element
  virtual void preparePosition(GuiElement* elem) const;
  //! Get an element
  GuiElement* getElement(int idx) const;
  //! Get all elements
  inline std::list<GuiElement*>& getElements(){return elements_;}
protected:
  std::list<GuiElement*> elements_;
};

}

#endif

