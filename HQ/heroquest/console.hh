//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: console.hh                                                 |
//  ==================================================================

#ifndef CONSOLE_HH
#define CONSOLE_HH

#include <string>
#include <list>
#include <sstream>

#include "vector.hh"
#include "font.hh"
#include "message.hh"
#include "gui.hh"

using std::string;
using std::list;
using std::ostringstream;

//! Provides a customizable quake-like game console
class Console{
  public:
    //! Constructor: Sets default values.
    Console();
   
    //! Copy constructor
    Console(const Console& c);
    
    //! Destructor
    ~Console();
    
    //! returns the input field where the text can be typed in
    inline InputField* GetInputField() {return &input_;}
    
    //! toggles the console on or off
    inline void toggle() {active_ = !active_;}
    
    //! returns if the console in on
    inline bool isActive() {return active_;}
    
    //! returns true if the input field contains new data
    inline bool hasNewContent() {return new_;}
    
    //! sets style parameters
    /*! changes the style of the input field in the console
     * \param fgCol foreground color for the input field
     * \param bgCol background color of the input field
     * \param opaque opacity of the input
     */
    inline void setParams(Vector3D fgCol, Vector3D bgCol, float opaque)
      {input_.setColors(fgCol, bgCol); input_.setOpacity(opaque);}
    
    //! returns the text of the input field
    inline string getText() {new_ = false; return history_.front();}
    
    //! sets the main callback function
    /*! the main callback is called every time the user has finished his input 
     * to the console. The function must belong to the message class and accept
     * a zero-terminated character array.
     */
    inline void setCbFunc(void (Message::*proc)(const char*)){process_ = proc;}
    
    //! sets the special callback function
    /*! this callback function is called only one time instead of the main callback 
     *  after being set with this method. It can be used in various ways. In HeroQuest,
     *  it is mainly used for realizing multiple input dialogs with the user.
     *  \param spec the function pointer to the special function
     *  \param i an integer that gets passed to the special callback when it is called
     *  \param add a void pointer that is handled like i and can point to everything.
     */
    inline void setSpecialFunc(void (Message::*spec)(const string&, int, void*), int i, void* add)
      {special_ = spec; mode_ = i; additional_ = add;}
    
    //! Syntactic sugar for calling addText
    inline void operator<<(const string& text) {addText(text);}
    
    //! Calls also addText
    inline void operator<<(const ostringstream& text) {addText(text.str());}
    
    //! Hides last input line.
    /*! This is useful for e.g. entering a password
     */
    inline void hideLastInput(){history_.pop_front();histIter_ = history_.begin();}
    
    //! Goes up in history
    inline void Up(){if(histIter_ != history_.end()){input_.setText(*histIter_);input_.addChar('_');histIter_++;}}
    
    //! Goes down in history
    inline void Down(){if(histIter_ != history_.begin()){histIter_--;input_.setText(*histIter_);input_.addChar('_');}}
    
    //! Adds text to the console output.
    void addText(const string& text);
     
    //! Displays the console using OpenGL
    void render();
    
    //! Updates the console contents
    void update();
    
private:
    //! The input field
    InputField input_;
    //! Here 
    list<string> history_;
    //! To iterate on the history
    list<string>::iterator histIter_;
    //! position of lower left corner
    Vector2D pos_;
    //! the console dimensions
    Vector2D span_;
    //! background color
    Vector3D bgColor_;
    //! foreground color
    Vector3D fgColor_;
    //! opacity
    float opacity_;
    //! the font for displaying text
    Font fnt_;
    //! is console active?
    bool active_;
    //! has it new contents?
    bool new_;
    //! main callback
    void (Message::*process_)(const char*);
    //! special callback function
    void (Message::*special_)(const string&, int, void*);
    //! integer parameter for special callback
    int mode_;
    //! void pointer for special callback
    void* additional_;
};

extern Console consol;

#endif