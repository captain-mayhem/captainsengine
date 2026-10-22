#ifndef HQ_WIDGETS_H
#define HQ_WIDGETS_H

#include <list>
#include <string>
#include <vector>

#include "math/vector.h"
#include "system/thread.h"

//to compile on windows
#undef MessageBox

struct ImFont;

// HeroQuest's buttons, input fields, drop-down fields and message boxes,
// drawn with Dear ImGui. They were the engine's GUI (Engine/gui) until it was
// replaced by ImGui; the client's menus keep them as objects in a list
// (widgets), which they look up by name, position or type.
//
// Positions and sizes are virtual coordinates (Engine::getVirtualScreen())
// with the origin at the bottom left and y up, the position is the lower left
// corner.

namespace Gui{

enum GuiType{
  UnknownT,
  ButtonT,
  InputFieldT,
  MessageBoxT,
  DropDownButtonT,
};

//! a common base class for gui elements
class GuiElement{
public:
  GuiElement();
  virtual ~GuiElement(){}
  //! draws it with ImGui; true if it was clicked (process is called then)
  virtual bool render() = 0;
  //! draws what goes below the engine's font texts, through the engine's draw context
  virtual void renderBackground() {}
  //! reacts on a click
  virtual void process() {}
  //! sets the name, by which it can be found (Manager::get)
  inline void setName(const std::string& text) { name_ = text; }
  inline const std::string& getName() const { return name_; }
  //! sets the position of the lower left corner
  inline virtual void setPosition(CGE::Vector2D position) {pos_ = position;}
  inline CGE::Vector2D getPosition() const {return pos_;}
  //! sets the width (x) and height (y)
  inline void setSpan(CGE::Vector2D span) {span_ = span;}
  inline CGE::Vector2D getSpan() const {return span_;}
  //! sets the colors
  /*! \param fgCol foreground (text) color (RGB)
   *  \param bgCol background color (RGB)
   */
  inline void setColors(CGE::Vector3D fgCol, CGE::Vector3D bgCol) {fgColor_ = fgCol; bgColor_ = bgCol;}
  inline GuiType getType() const {return type_;}
protected:
  //! starts an ImGui window of the element at pos with the size span
  /*! Always followed by endWindow. An element with several windows gives
      each its own part. */
  bool beginWindow(const CGE::Vector2D& pos, const CGE::Vector2D& span, bool inputs = true, int part = 0);
  void endWindow();
  //! the window pixels per virtual unit
  static float scale();

  CGE::Vector2D pos_;
  CGE::Vector2D span_;
  CGE::Vector3D bgColor_;
  CGE::Vector3D fgColor_;
  std::string name_;
  GuiType type_;
};

//! a text field for user input
class InputField : public GuiElement{
  public:
    InputField();
    //! sets the text
    void setText(const std::string& text);
    //! returns the text
    inline std::string getText() const {return text_.data();}
    //! clears the text
    inline void clear() {text_[0] = '\0';}
    //! shows the text as stars (for password fields)
    inline void setHidden(bool hide = true) {isHidden_ = hide;}
    virtual bool render() override;
  protected:
    //! the text, zero-terminated
    std::vector<char> text_;
    //! password field?
    bool isHidden_;
};

//! a button (the thing you can click on)
class Button : public InputField{
  public:
    Button();
    //! sets the callback function
    inline void setCbFunc(void (*click)(GuiElement*)) { handleClicks_ = click; }
    //! executes the callback function
    virtual void process() override { if (handleClicks_) (*handleClicks_)(this); }
    virtual bool render() override;
  protected:
    //! the callback function
    void (*handleClicks_)(GuiElement*);
};

//! a drop-down field: a button that opens a list of entries
/*! Choosing an entry sets its text and calls the callback. */
class DropDownButton : public Button {
  public:
    DropDownButton();
    //! adds an entry
    void addEntry(const std::string& entry);
    //! removes all entries
    void clear();
    //! sets the selected entry
    inline void setSelection(int idx) {selected_ = idx;}
    //! the selected entry
    inline int getSelection() const {return selected_;}
    virtual bool render() override;
  private:
    std::vector<std::string> entries_;
    int selected_;
};

//! a message box with an OK button, which closes it
class MessageBox : public Button {
  public:
    MessageBox();
    //! sets the position of the box's lower left corner
    virtual void setPosition(CGE::Vector2D pos) override;
    //! closes the box and calls the callback
    virtual void process() override;
    //! sets the message
    void setMessage(const std::string& text);
    virtual bool render() override;
    //! the dark box behind the message, below the engine's font texts
    virtual void renderBackground() override;
  private:
    //! the box
    CGE::Vector2D surSpan_;
    CGE::Vector2D surPos_;
    //! the message, in lines
    std::list<std::string> msg_;
};

//! the elements on the screen
class Manager{
public:
  Manager();
  ~Manager();
  //! adds an element, which the manager owns
  /*! Not immediately: all elements are replaced at the next render by the
      ones added like this (for the message thread and the callbacks). The
      texts of the engine's fonts are cleared right away when the first one is
      added, so that the texts printed for the new elements stay. */
  void add(GuiElement* element, bool immediate=true);
  //! deletes all elements and the texts of the engine's fonts
  /*! Not immediately: the elements at the next render, the texts right away. */
  void clear(bool immediate=true);
  //! deletes the element at position idx
  void remove(int idx, bool immediate=true);
  //! deletes the first element named name
  void remove(const std::string& name);
  //! returns the first element named name, or nullptr
  GuiElement* get(const std::string& name);
  inline std::list<GuiElement*>& getElements() {return elements_;}
  //! locks the elements (for the message thread)
  CGE::Mutex& getMutex() {return mutex_;}
  //! draws the elements and processes the one clicked
  /*! In the application's render, inside a pass with the draw context's 2D
      projection of the virtual screen (y up). */
  void render();
  //! the font the elements are drawn with
  ImFont* getFont();
private:
  //! applies the changes that weren't immediate
  void update();
  //! deletes the elements
  void deleteElements();
  //! clears the texts of the engine's fonts
  void clearTexts();
  std::list<GuiElement*> elements_;
  //! the elements that replace the current ones
  std::list<GuiElement*> newElements_;
  //! clear at the next render
  bool clear_;
  //! the indices to delete at the next render
  std::vector<int> removed_;
  CGE::Mutex mutex_;
  ImFont* font_;
};

}

//! the elements on the screen
extern Gui::Manager widgets;

#endif
