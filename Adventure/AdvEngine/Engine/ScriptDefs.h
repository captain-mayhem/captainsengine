#ifndef SCRIPTDEFS_H
#define SCRIPTDEFS_H

#include <cstdlib>
#include "String.h"

extern "C"{
#include <lua.h>
}

namespace adv{

class ExecutionContext;

class StackData{
  friend std::ostream& operator<<(std::ostream& strm, const StackData& data);
  friend std::istream& operator>>(std::istream& strm, StackData& data);
public:
  StackData() : mType(I), mStr(""), mInt(0) {}
  StackData(String str) : mType(S), mInt(0) {mStr = str; if (mStr == "true"){mBool = true; mType = B;}}
  StackData(int value) : mType(I) {mInt = value;}
  StackData(bool value) : mType(B) {mBool = value;}
  StackData(float value) : mType(F) {mFloat = value;}
  StackData(ExecutionContext* ctx) : mType(EC) {mContext = ctx;}
  String getString() const {return mStr;}
  int getInt() const {if (mType == F) return (int)mFloat; else if (mType == S) return atoi(mStr.c_str()); return mInt;}
  bool getBool() const {return mBool;}
  float getFloat() const {if (mType == I)return (float)mInt; return mFloat;}
  ExecutionContext* getEC() const {return mContext;}
  bool isInt() const {return mType == I;}
  bool isFloat() const {return mType == F;}
  bool isString() const {return mType == S;}
  bool isNumber() const {return mType == I || mType == F;}
  bool isBool() const { return mType == B; }
  bool isEC() const { return mType == EC; }
  static StackData fromStack(lua_State* L, int idx);
  static void pushStack(lua_State* L, StackData const& sd);
protected:
  enum Type{
    S, I, B, F, EC
  };
  Type mType;
  String mStr;
  union{
    int mInt;
    bool mBool;
    float mFloat;
    ExecutionContext* mContext;
  };
};

std::ostream& operator<<(std::ostream& strm, const StackData& data);
std::istream& operator>>(std::istream& strm, StackData& data);

class Stack{
  friend class CCALL;
public:
  Stack() : mL(NULL) {}
  Stack(const Stack& st){ mL = NULL; }
  ~Stack(){}
  void push(const StackData& v) {StackData::pushStack(mL, v);}
  StackData pop() {
    StackData data = StackData::fromStack(mL, -1);
    lua_pop(mL, 1);
    return data;
  }
  void clear(){
    lua_settop(mL, 0);
  }
  StackData top() { return StackData::fromStack(mL, -1); }
  StackData get(int idx) { return StackData::fromStack(mL, idx); }
  void setState(lua_State* L) { mL = L; }
protected:
  lua_State* mL;
};

}

#endif
