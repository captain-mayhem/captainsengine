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
    virtual void setPosition(const ::CGE::Vector2D& pos);
    //! process the click
    virtual void process();
    //! set the text in the message box
    void setMessage(const string& text);
  private:
    //! the dimensions of the background
    CGE::Vector2D surSpan_;
    //! the position of the box
    CGE::Vector2D surPos_;
    //! the background color
    Graphics::Color surBgColor_;
    //! the foreground color
    Graphics::Color surFgColor_;
    //! the message to be displayed
    list<string> msg_;
};

}

#endif
