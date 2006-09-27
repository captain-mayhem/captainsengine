#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "gui.h"

//to compile on windows
#undef MessageBox

namespace Gui{
//! This class is a message box.
/*! only a message box
 */
class MessageBox : public Button {
  public:
    //! Constructor
    MessageBox();
    //! Copy Constructor
    MessageBox(const MessageBox& m);
    //! Destructor
    virtual ~MessageBox();
    //! renders the message box
    virtual void render();
    //! sets the message box position
    virtual void setPosition(const ::Math::Vector2D& pos);
    //! process the click
    virtual void process();
    //! set the text in the message box
    void setMessage(const string& text);
  private:
    //! the dimensions of the background
    Math::Vector2D span_;
    //! the position of the box
    Math::Vector2D pos_;
    //! the background color
    Graphics::Color bgColor_;
    //! the foreground color
    Graphics::Color fgColor_;
    //! the message to be displayed
    list<string> msg_;
};

}

#endif
