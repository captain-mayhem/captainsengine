#ifndef ADV_MENU_H
#define ADV_MENU_H

#include "EngineObjects.h"

namespace adv{

class Menu : public RoomObject{
public:
  Menu(int font);
  static CGE::Image* getBackground();
protected:
  void addButton(const char* text, int id);
  static unsigned mLineHeight;
};

}

#endif