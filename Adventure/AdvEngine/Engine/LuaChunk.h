#ifndef ADV_LUA_CHUNK_H
#define ADV_LUA_CHUNK_H

#include "String.h"

namespace adv{

class LuaFunctionBlock{
public:
  LuaFunctionBlock();
  String serialize();
private:
  String mName;
  int mFirstLine;
  int mLastLine;
};

class LuaChunk{
public:
  String serialize();
  static void store(String& s, size_t size);
  static void store(String& s, int val);
  static void append(String& s, char val);
private:
};

}

#endif