#ifndef ADV_MENU_H
#define ADV_MENU_H

#include "EngineObjects.h"

namespace adv{

class Menu : public RoomObject{
public:
  Menu();
  static CGE::Image* getBackground();
};

}

#endif