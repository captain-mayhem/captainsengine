#ifndef CGE_CONSOLE_H
#define CGE_CONSOLE_H

#include <cge_export.h>

#include <deque>
#include <sstream>
#include <string>

#define consol (*CGE::Engine::instance()->getConsole())

namespace CGE{

//! A Quake-like console: an ImGui window at the top of the screen
/*! It shows the text added to it and an input line. Entered lines go to the
    main callback, or once to the special callback if one is set. The engine
    owns it (Engine::getConsole(), or the consol macro), toggles it with '~'
    (Input::Keyboard::enableBuitInConsole) and closes it with Escape. */
class CGE_API Console{
  public:
    Console();
    ~Console();

    //! toggles the console on or off
    void toggle();
    //! returns if the console is on
    inline bool isActive() const {return active_;}

    //! returns true if a line was entered since the last getText
    inline bool hasNewContent() const {return new_;}
    //! returns the line entered last
    inline std::string getText() {new_ = false; return lastInput_;}

    //! sets the main callback function
    /*! The main callback is called every time the user has entered a line.
     */
    inline void setCbFunc(void (*proc)(const char*)) {process_ = proc;}

    //! sets the special callback function
    /*! It is called once instead of the main callback for the next line.
     *  HeroQuest uses it for dialogs with several inputs.
     *  \param spec the function
     *  \param i an integer that gets passed to the special callback
     *  \param add a pointer that gets passed to the special callback
     */
    inline void setSpecialFunc(void (*spec)(const std::string&, int, void*), int i, void* add)
      {special_ = spec; mode_ = i; additional_ = add;}

    //! hides the input (for passwords): it is shown as stars and not added to the text or the history
    inline void setPasswordInput(bool password) {password_ = password;}

    //! Syntactic sugar for calling addText
    inline void operator<<(const std::string& text) {addText(text);}
    //! Calls also addText
    inline void operator<<(const std::ostringstream& text) {addText(text.str());}

    //! adds text to the console output; it is split at newlines
    void addText(const std::string& text);
    //! the output lines, oldest first
    inline const std::deque<std::string>& getLines() const {return lines_;}
    //! passes line on as if it was entered in the input line (at the next update)
    inline void enter(const std::string& line) {entered_.push_back(line);}

    //! passes the entered lines to the callbacks; the engine calls it every frame
    void update();
    //! shows the console as an ImGui window, if it is on; the engine calls it
    //! inside its ImGui frame, after the application's render
    void render();

  private:
    //! the output lines, oldest first
    std::deque<std::string> lines_;
    //! the entered lines, oldest first
    std::deque<std::string> history_;
    //! the history entry shown in the input line while going through it with up and down (-1: none)
    int historyPos_;
    //! the input line
    char input_[512];
    //! the lines entered but not yet passed to the callbacks
    std::deque<std::string> entered_;
    //! the line entered last
    std::string lastInput_;
    //! is the console on?
    bool active_;
    //! give the input line the keyboard focus in the next frame
    bool focusInput_;
    //! scroll the output to its end in the next frame
    bool scrollToEnd_;
    //! is a line entered since the last getText?
    bool new_;
    //! is the input a password?
    bool password_;
    //! main callback
    void (*process_)(const char*);
    //! special callback function
    void (*special_)(const std::string&, int, void*);
    //! integer parameter for special callback
    int mode_;
    //! pointer parameter for special callback
    void* additional_;

    friend struct ConsoleCallbacks;
};

}

#endif
