#include "ItemObject.h"

using namespace adv;

ItemObject::ItemObject(int state, const Vec2i& pos, const Vec2i& size, const std::string& name) :
Object2D(state, pos, size, name), mCount(1){

}
