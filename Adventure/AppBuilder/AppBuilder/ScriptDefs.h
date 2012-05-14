#ifndef SCRIPTDEFS_H
#define SCRIPTDEFS_H

namespace adv{

class ExecutionContext;

class StackData{
  friend std::ostream& operator<<(std::ostream& strm, const StackData& data);
  friend std::istream& operator>>(std::istream& strm, StackData& data);
public:
  StackData() : mType(I), mStr(""), mInt(0) {}
  StackData(std::string str) : mType(S), mInt(0) {mStr = str; if (mStr == "true"){mBool = true; mType = B;}}
  StackData(int value) : mType(I) {mInt = value;}
  StackData(bool value) : mType(B) {mBool = value;}
  StackData(float value) : mType(F) {mFloat = value;}
  StackData(ExecutionContext* ctx) : mType(EC) {mContext = ctx;}
  std::string getString() const {return mStr;}
  int getInt() const {return mInt;}
  bool getBool() const {return mBool;}
  float getFloat() const {if (mType == I)return (float)mInt; return mFloat;}
  ExecutionContext* getEC() const {return mContext;}
  bool isInt() const {return mType == I;}
  bool isString() const {return mType == S;}
protected:
  enum Type{
    S, I, B, F, EC
  };
  Type mType;
  std::string mStr;
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
public:
  Stack(){}
  Stack(const Stack& st){mVariables = st.mVariables;}
  ~Stack(){}
  void push(const StackData& v) {mVariables.push_back(v);}
  StackData pop() {
    StackData data = mVariables.back();
    mVariables.pop_back();
    return data;
  }
  void clear(){
    mVariables.clear();
  }
  StackData& top() {return mVariables.back();}
protected:
  std::vector<StackData> mVariables;
};

}

#endif
