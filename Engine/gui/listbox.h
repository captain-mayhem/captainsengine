//  ==================================================================
// |                         Hero Engine                              |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: listbox.h                                                  |
//  ==================================================================

#ifndef LISTBOX_H
#define LISTBOX_H

#include "gui.h"
#include "../math/vector.h"

namespace Gui{

//! a dialog field
class ListBox : public GuiElement {
public:
  //! Constructor
  ListBox();
  //! Destructor
  virtual ~ListBox();
  //! The render function
  virtual void render();
  //! Is the dialog clicked
  virtual bool isClicked(const ::Math::Vector2D& pos);
  //! Process the click
  virtual void process();
  //! Add an entry
  virtual void addEntry(std::string entry);
  //! Add multiple entries
  void addEntries(const std::vector<std::string>& entries);
  //! Clear the entries
  void clear() {selected_ = -1; entries_.clear();}
  //! Set the highlight color
  inline void setHighlightColor(const Math::Vector3D& color) {selColor_ = color;}
  //! sets the callback function
  inline void setCbFunc(void (*click)(GuiElement*)) { handleClicks_ = click; }
  //! get selected item
  inline std::string selectedItem() {if (selected_ == -1) return "none"; return entries_[selected_];}
protected:
  //! The list box entries
  std::vector<std::string> entries_;
  //! The selected item
  int selected_;
  //! Highlight color
  Math::Vector3D selColor_;
  //! The callback function
  void (*handleClicks_)(GuiElement*);
};

}

#endif

