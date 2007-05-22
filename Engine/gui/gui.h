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

#include "../math/vector.h"
#include "../renderer/font.h"

using std::string;
using std::list;
using std::ostringstream;

//using Math::Vector3D;
//using Math::Vector2D;

namespace Gui{
//! a common base class for gui elements
class GuiElement{
public:
  GuiElement(){}
  virtual ~GuiElement(){name_ = "none";}
  virtual void render() = 0;
  virtual bool isClicked(const ::Math::Vector2D& pos) = 0;
  virtual void process() = 0;
  //! sets the button text
  inline void setName(const string& text) { name_ = text; }
  //! gets the button text
  inline const string& getName() { return name_; }
protected:
  std::string name_;
};

//! a highly customizable text field for user input based on OpenGL
class InputField : public GuiElement{
  public:
    //! Constructor: Sets default parameters
    InputField();
    //! Copy constructor
    InputField(const InputField& i);
    //! Destructor
    virtual ~InputField() {}
    //! Add a character to the field
    inline void addChar(char ch) {field_.append(1,ch);}
    //! Remove a character from the field
    inline void removeChar() {if(field_.length() > 0) field_.erase(field_.length()-1);}
    //! Sets text in the input field
    inline void setText(const string text) {field_ = text;}
    //! Returns the text in the input field
    inline string& getText() {finished_ = false; if (field_.length() == 0) return field_; if(field_[field_.length()-1] == '_')
	    field_.erase(field_.length()-1,1); return field_;}
    //! Sets the position of the input field
    /*! The position is specified by the lower left corner of the field
     */
    inline void setPosition(::Math::Vector2D position) {pos_ = position;}
    //! Gets the position
    inline ::Math::Vector2D getPosition() {return pos_;}
    //! Sets the dimensions of the field
    /*! the x-component of span is the width, the y-component the height
     */
    inline void setSpan(::Math::Vector2D span) {span_ = span;}
    //! Gets the dimensions of the field
    inline ::Math::Vector2D getSpan() {return span_;}
    //! Sets the opacity of the field
    inline void setOpacity(float opaque) {opacity_ = opaque;}
    //! Sets the colors of the field
    /*! \param fgCol foreground (text) color of the field (RGB)
     *  \param bgCol background color (RGB)
     */
    inline void setColors(::Math::Vector3D fgCol, ::Graphics::Color bgCol)
      {fgColor_ = fgCol, bgColor_ = bgCol;}
    //! Says that the user input has finished
    inline void end() {finished_ = true;}
    //! Returns, if the user has finished input
    inline bool isFinished() {return finished_;}
    //! Clears the text in the field
    inline void clear() {field_.erase();}
    //! Sets the font to use
    inline void setFont(::Graphics::Font* fnt) {fnt_ = fnt;}
    //! Displays the field using OpenGL
    virtual void render();
    //! Returns if the position belongs to the input field
    virtual bool isClicked(const ::Math::Vector2D& pos);
    //! Hides typed input (for password fields);
    inline void setHidden(bool hide = true) {isHidden_ = hide;}
    //! overloaded process
    virtual void process() {};
  private:
    //! position
    ::Math::Vector2D pos_;
    //! dimensions of the field
    ::Math::Vector2D span_;
    //! background color
    ::Graphics::Color bgColor_;
    //! foreground color
    ::Math::Vector3D fgColor_;
    //! opacity
    float opacity_;
    //! The font to write the field text with
    ::Graphics::Font* fnt_;
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
class Button : public GuiElement{
  public:
    //! Constructor
    Button();
    //! Copy Constructor
    Button(const Button& b);
    //! Destructor
    virtual ~Button();
    //! Is it clicked upon?
    inline bool isClicked(const ::Math::Vector2D& pos) {return input_.isClicked(pos);}
    //! sets the button text
    inline void setText(const string& text) { input_.setText(text); }
    //! gets the button text
    inline const string& getText() { return input_.getText(); }
    //! sets the button position
    virtual void setPosition(const ::Math::Vector2D& pos) {input_.setPosition(pos);}
    //! sets the button dimesions
    void setSpan(const ::Math::Vector2D& span) {input_.setSpan(span);}
    //! sets the callback function
    inline void setCbFunc(void (*click)()) { handleClicks_ = click; }
    //! executes the callback function
    virtual void process() { (*handleClicks_)(); }
    //! renders the button
    virtual void render() {input_.render(); }
  protected:
    //! The InputField
    //! The button is nothing more than an input field with an additional callback
    // and some input field functions not being accessible.
    InputField input_;
    //! The callback function
    void (*handleClicks_)();
};
}
#endif
