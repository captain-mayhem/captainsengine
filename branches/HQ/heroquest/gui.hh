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

#include "vector.hh"
#include "font.hh"
#include "message.hh"
#include "renderer.hh"

using std::string;
using std::list;
using std::ostringstream;

//! a highly customizable text field for user input based on OpenGL
class InputField{
  public:
    //! Constructor: Sets default parameters
    InputField();
    //! Copy constructor
    InputField(const InputField& i);
    //! Destructor
    ~InputField() {}
    //! Add a character to the field
    inline void addChar(char ch) {field_.append(1,ch);}
    //! Remove a character from the field
    inline void removeChar() {if(field_.length() > 0) field_.erase(field_.length()-1);}
    //! Sets text in the input field
    inline void setText(const string text) {field_ = text;}
    //! Returns the text in the input field
    inline string& getText() {finished_ = false; if(field_[field_.length()-1] == '_')
	    field_.erase(field_.length()-1,1); return field_;}
    //! Sets the position of the input field
    /*! The position is specified by the lower left corner of the field
     */
    inline void setPosition(Vector2D position) {pos_ = position;}
    //! Sets the dimensions of the field
    /*! the x-component of span is the width, the y-component the height
     */
    inline void setSpan(Vector2D span) {span_ = span;}
    //! Sets the opacity of the field
    inline void setOpacity(float opaque) {opacity_ = opaque;}
    //! Sets the colors of the field
    /*! \param fgCol foreground (text) color of the field (RGB)
     *  \param bgCol background color (RGB)
     */
    inline void setColors(Vector3D fgCol, Vector3D bgCol)
      {fgColor_ = fgCol, bgColor_ = bgCol;}
    //! Says that the user input has finished
    inline void end() {finished_ = true;}
    //! Returns, if the user has finished input
    inline bool isFinished() {return finished_;}
    //! Clears the text in the field
    inline void clear() {field_.erase();}
    //! Sets the display list base for the font
    inline void setFontBase(int base) {fnt_.setBase(base);}
    //! Displays the field using OpenGL
    void render();
    //! Returns if the position belongs to the input field
    bool isHit(const Vector2D& pos);
    //! Hides typed input (for password fields);
    inline void setHidden(bool hide = true) {isHidden_ = hide;}
  private:
    //! position
    Vector2D pos_;
    //! dimensions of the field
    Vector2D span_;
    //! background color
    Vector3D bgColor_;
    //! foreground color
    Vector3D fgColor_;
    //! opacity
    float opacity_;
    //! The font to write the field text with
    Font fnt_;
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
class Button{
  public:
    //! Constructor
    Button();
    //! Copy Constructor
    Button(const Button& b);
    //! Destructor
    ~Button();
    //! Is it clicked upon?
    inline bool isClicked(const Vector2D& pos) {return input_.isHit(pos);}
    //! sets the button text
    inline void setText(const string& text) { input_.setText(text); }
    //! gets the button text
    inline const string& getText() { return input_.getText(); }
    //! sets the button position
    inline void setPosition(const Vector2D& pos) {input_.setPosition(pos);}
    //! sets the callback function
    inline void setCbFunc(void (Renderer::*click)()) { handleClicks_ = click; }
    //! executes the callback function
    inline void process() { (gl->*handleClicks_)(); }
    //! renders the button
    inline void render() {input_.render(); }
  private:
    //! The InputField
    //! The button is nothing more than an input field with an additional callback
    // and some input field functions not being accessible.
    InputField input_;
    //! The callback function
    void (Renderer::*handleClicks_)();
};

#endif
