#ifndef ADV_STRING_H
#define ADV_STRING_H

#include <string>

namespace adv{

class String{
  friend std::ostream& operator<<(std::ostream& strm, const String& str);
  friend std::istream& operator>>(std::istream& strm, String& str);
public:
  String() : mStr() {}
  String(const char* str) : mStr(str) {}
  String(const std::string& str) : mStr(str) {}
  String& operator+=(const String& s) {mStr += s.mStr; return *this;}
  String& operator+=(const char& ch) {mStr += ch; return *this;}
  char& operator[](int i) {return mStr[i];}
  String& erase (unsigned pos = 0, unsigned len = npos) {mStr.erase(pos, len); return *this;}
  void clear() {mStr.clear();}

  String operator+(const String& s);
  String operator+(const char* s);
  bool operator==(const String& other) const {return mStr == other.mStr;}
  bool operator!=(const String& other) const {return mStr != other.mStr;}
  bool operator<(const String& s) const { return mStr < s.mStr;}
  const char* c_str() const {return mStr.c_str();}
  bool empty() const {return mStr.empty();}
  unsigned size() const {return mStr.size();}
  const char& operator[](int i) const {return mStr[i];}

  String substr(unsigned pos = 0, unsigned len = npos) const {String ret(mStr.substr(pos, len)); return ret;}

  operator std::string() {return mStr;}
  operator const std::string() const {return mStr;}

  String removeAll(char ch) const;
  String toLower() const;

  static const unsigned npos = 4294967295;
protected:
  std::string mStr;
};

std::ostream& operator<<(std::ostream& strm, const String& str);
std::istream& operator>>(std::istream& strm, String& str);

}

#endif
