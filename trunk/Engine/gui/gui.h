//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: gui.hh                                                     |
//  ==================================================================

#ifndef GUI_HH
#define GUI_HH

#include <string>
#include <list>
#include <sstream>
#include <cassert>

#include "../math/vector.h"
#include "../renderer/font.h"

using std::string;
using std::list;
using std::ostringstream;

//using CGE::Vector3D;
//using CGE::Vector2D;

namespace Gui{

enum GuiType{
  UnknownT,
  ButtonT,
  InputFieldT,
  MessageBoxT,
  DropDownButtonT,
  DialogT,
  ListBoxT,
};
  
//! a common base class for gui elements
class GuiElement{
public:
  GuiElement();
  virtual ~GuiElement(){}
  virtual void render() = 0;
  virtual bool isClicked(const ::CGE::Vector2D& pos);
  virtual void process() = 0;
  //! sets the button text
  inline void setName(const string& text) { name_ = text; }
  //! gets the button text
  inline const string& getName() const { return name_; }
  //! Sets the position of the input field
  /*! The position is specified by the lower left corner of the field
   */
  inline virtual void setPosition(::CGE::Vector2D position) {pos_ = position;}
  //! Gets the position
  inline ::CGE::Vector2D getPosition() const {return pos_;}
  //! Sets the dimensions of the field
  /*! the x-component of span is the width, the y-component the height
   */
  inline void setSpan(::CGE::Vector2D span) {span_ = span;}
  //! Gets the dimensions of the field
  inline ::CGE::Vector2D getSpan() const {return span_;}
  //! Sets the opacity of the field
  inline void setOpacity(float opaque) {opacity_ = opaque;}
  //! Sets the colors of the field
    /*! \param fgCol foreground (text) color of the field (RGB)
     *  \param bgCol background color (RGB)
     */
  inline void setColors(::CGE::Vector3D fgCol, ::CGE::Color bgCol)
      {fgColor_ = fgCol, bgColor_ = bgCol;}
  //! Get the gui type
  inline GuiType getType() const {return type_;}
  //! Set the parent
  inline void setParent(GuiElement* parent) {parent_ = parent;}
  //! Get the parent
  inline GuiElement* getParent() const {return parent_;}
protected:
  //! position
  CGE::Vector2D pos_;
  //! dimensions of the field
  CGE::Vector2D span_;
  //! background color
  CGE::Color bgColor_;
  //! foreground color
  CGE::Vector3D fgColor_;
  //! opacity
  float opacity_;
  //! name of the gui element
  std::string name_;
  //! the type of the gui element
  GuiType type_;
  //! the parent of the gui element
  GuiElement* parent_;
};

//! a highly customizable text field for user input based on OpenGL
class InputField : public GuiElement{
  public:
    //! Constructor: Sets default parameters
    InputField();
    //! Copy constructor
    InputField(const InputField& i);
    //! Destructor
    virtual ~InputField();
    //! Add a character to the field
    inline void addChar(char ch) {field_.append(1,ch);}
    //! Remove a character from the field
    inline void removeChar() {if(field_.length() > 0) field_.erase(field_.length()-1);}
    //! Sets text in the input field
    inline void setText(const string text) {field_ = text;}
    //! Returns the text in the input field
    inline string& getText() {finished_ = false; if (field_.length() == 0) return field_; if(field_[field_.length()-1] == '_')
	    field_.erase(field_.length()-1,1); return field_;}
    //! Says that the user input has finished
    inline void end() {finished_ = true;}
    //! Returns, if the user has finished input
    inline bool isFinished() {return finished_;}
    //! Clears the text in the field
    inline void clear() {field_.erase();}
    //! Sets the font to use
    inline void setFont(CGE::Font* fnt) {fnt_ = fnt;}
    //! Displays the field using OpenGL
    virtual void render();
    //! Hides typed input (for password fields);
    inline void setHidden(bool hide = true) {isHidden_ = hide;}
    //! overloaded process
    virtual void process();
  protected:
    //! The font to write the field text with
    CGE::Font* fnt_;
    //! The field text
    string field_;
    //! Input finished?
    bool finished_;
    //! Password field?
    bool isHidden_;
};

//! This class is a button (the thing you can click on).
/*! The button is based on OpenGL.
 */
class Button : public InputField{
  public:
    //! Constructor
    Button();
    //! Copy Constructor
    Button(const Button& b);
    //! Destructor
    virtual ~Button();
    //! sets the callback function
    inline void setCbFunc(void (*click)(GuiElement*)) { handleClicks_ = click; }
    //! executes the callback function
    virtual void process() { assert(handleClicks_ && "Button callback not set!"); (*handleClicks_)(this); }
  protected:
    //! The callback function
    void (*handleClicks_)(GuiElement*);
};

//! A button that destroys his parent if clicked upon
class PDButton : public Button{
  //! executes the callback function and destroys parent
  virtual void process();
};

}
#endif
