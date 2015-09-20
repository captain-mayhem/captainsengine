#include "String.h"

#include <iostream>
#include "Engine.h"

using namespace adv;

String String::operator+(const String& s){
  String ret;
  ret.mStr = mStr + s.mStr;
  return ret;
}

String String::operator+(const char* s){
  String ret;
  ret.mStr = mStr+s;
  return ret;
}

String String::removeAll(char ch) const{
  String ret;
  for (unsigned i = 0; i < mStr.size(); ++i){
    if (mStr.at(i) != ch)
      ret.mStr.append(1, mStr.at(i));
  }
  return ret;
}

String String::toLower() const {
  String ret;
  ret.mStr.resize(mStr.size());
  for (unsigned i = 0; i < mStr.length(); i++){
    ret.mStr.at(i) = tolower(mStr.at(i));
  }
  return ret;
}

bool String::ieqauls(const String& other){
  return _stricmp(c_str(), other.c_str()) == 0;
}

void String::trim(){
  if (mStr.empty())
    return;
  size_t end;
  for (end = mStr.size() - 1; end > 0; --end){
    if (mStr[end] < 0 || !isspace(mStr[end]))
      break;
  }
  size_t begin;
  for (begin = 0; begin <= end; ++begin){
    if (mStr[begin] < 0 || !isspace(mStr[begin]))
      break;
  }
  mStr = mStr.substr(begin, end - begin + 1);
}

namespace adv{

std::ostream& operator<<(std::ostream& strm, const String& str){
  strm << str.mStr.size() << " " << str.mStr.data();
  return strm;
}

std::istream& operator>>(std::istream& strm, String& str){
  str.mStr = std::string();
  unsigned size;
  strm >> size;
  char tmp;
  strm.get(tmp);
  str.mStr.resize(size);
  strm.read((char*)str.mStr.data(), size);
  return strm;
}

String operator+(const char* ptr, const String& str){
  return String(ptr) + str;
}

}

void String::replaceWith(char oldCh, char newCh){
  for (size_t i = 0; i < mStr.size(); i++){
    if (mStr[i] == oldCh)
      mStr[i] = newCh;
  }
}

bool String::endsWith(const String& str){
  size_t pos = mStr.rfind(str.mStr);
  return pos == mStr.size() - str.size();
}
