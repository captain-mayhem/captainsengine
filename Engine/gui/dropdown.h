#ifndef DROPDOWN_H
#define DROPDOWN_H

#include <vector>

#include "gui.h"

using std::vector;

namespace Gui{

class DDEntryButton : public Button {
public:
  //! Constructor
  DDEntryButton(int number);
  //! Destructor
  virtual ~DDEntryButton();
  //! set the parent button
  inline void setParent(Button* b) {parent_ = b;}
  //! process the click
  virtual void process();
private:
  //! the parent
  Button* parent_;
  //! the entry number
  int entryNum_;
};
  
//! This class is a drop down field.
/*! only a drop down field
 */
class DropDownButton : public Button {
  public:
    //! Constructor
    DropDownButton();
    //! Copy Constructor
    DropDownButton(const DropDownButton& d);
    //! Destructor
    virtual ~DropDownButton();
    //! determine drop down position
    /*! the position of the DDButton has to be set before calling this function
     *\param direction 1 means right, 2 bottom and 3 left.
     */
    void calcDDPos(int direction);
    //! add a drop down entry
    void addEntry(const string& entry);
    //! clear all entries
    void clear();
    //! process the click
    virtual void process();
    //! if the drop down field is opened
    inline bool isOpen() const {return isOpen_;}
    //! set selection
    inline void setSelection(int idx) {selected_ = idx;}
    //! get selection
    inline int getSelection() const {return selected_;}
  private:
    //! the drop down entries
    vector<DDEntryButton*> entries_;
    //! the current drop down position
    Math::Vector2D ddPos_;
    //! the status of the field
    bool isOpen_;
    //! the popup direction
    int direction_;
    //! the number of the current entry
    int entryNum_;
    //! the selection index
    int selected_;
};

}

#endif
