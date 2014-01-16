#include "Loader.h"

using namespace adv;

bool ResLoader::run(){
  mMutex.lock();
  mCond.wait(mMutex);
  mMutex.unlock();
  return true;
}
