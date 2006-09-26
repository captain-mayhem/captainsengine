#ifndef DROPDOWN_H
#define DROPDOWN_H

#include <vector>

#include "gui.h"

using std::vector;

namespace Gui{

class DDEntryButton : public Button {
public:
  //! Constructor
  DDEntryButton();
  //! Destructor
  virtual ~DDEntryButton();
  //! set the parent button
  inline void setParent(Button* b) {parent_ = b;}
  //! process the click
  virtual void process();
private:
  Button* parent_;
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
    inline bool isOpen() {return isOpen_;}
  private:
    //! the drop down entries
    vector<DDEntryButton*> entries_;
    //! the current drop down position
    Math::Vector2D ddPos_;
    //! the status of the field
    bool isOpen_;
    //! the popup direction
    int direction_;
};

}

#endif
