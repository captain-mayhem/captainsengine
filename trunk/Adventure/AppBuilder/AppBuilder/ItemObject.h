#ifndef ADV_ITEM_OBJECT_H
#define ADV_ITEM_OBJECT_H

#include "EngineObjects.h"

namespace adv{

class ItemObject : public Object2D{
public:
  ItemObject(int state, const Vec2i& pos, const Vec2i& size, const std::string& name);
  virtual Type getType() {return ITEM;}
  void setCount(int count) {mCount = count;}
  int getCount() {return mCount;}
protected:
  int mCount;
};

}

#endif
