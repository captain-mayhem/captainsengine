#include "LuaChunk.h"

#include <cstring>

extern "C"{
#include <lua.h>
#include <lopcodes.h>
}

using namespace adv;

String LuaChunk::serialize(){
  String ret;
  //write header
  ret.resize(12);
  ret[0] = 0x1b;
  ret[1] = 'L';
  ret[2] = 'u';
  ret[3] = 'a';
  ret[4] = 0x52; //version
  ret[5] = 0;
  ret[6] = 1; //little endian
  ret[7] = sizeof(int);
  ret[8] = sizeof(size_t);
  ret[9] = sizeof(Instruction);
  ret[10] = sizeof(lua_Number);
  ret[11] = 0; //integral flag
  ret += "\x19\x93\r\n\x1a\n";
  LuaFunctionBlock fb;
  ret += fb.serialize();
  return ret;
}

void LuaChunk::store(String& s, size_t size){
  int pos = (int)s.size();
  s.resize(pos + sizeof(size_t));
  memcpy(&s[pos], &size, sizeof(size_t));
}

void LuaChunk::store(String& s, int val){
  int pos = (int)s.size();
  s.resize(pos + sizeof(int));
  memcpy(&s[pos], &val, sizeof(int));
}

void LuaChunk::append(String& s, char val){
  s.append(1, val);
}

LuaFunctionBlock::LuaFunctionBlock() : mFirstLine(0), mLastLine(0) {

}

String LuaFunctionBlock::serialize(){
  String ret;
  //LuaChunk::store(ret, (size_t)0); //name
  LuaChunk::store(ret, mFirstLine);
  LuaChunk::store(ret, mLastLine);
  LuaChunk::append(ret, 0); //parameters
  //LuaChunk::append(ret, 1); //upvalues
  LuaChunk::append(ret, 2); //vararg
  LuaChunk::append(ret, 2); //stack size

  LuaChunk::store(ret, (int)0);//code
  LuaChunk::store(ret, (int)0);//constants
  LuaChunk::store(ret, (int)0);//functions
  LuaChunk::store(ret, (int)0);//upvalues
  //debug info
  LuaChunk::store(ret, (size_t)0);//name
  LuaChunk::store(ret, (int)0);//line info
  LuaChunk::store(ret, (int)0);//local variables
  LuaChunk::store(ret, (int)0);//upvalues
  return ret;
}
