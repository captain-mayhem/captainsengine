#ifndef ADV_STRING_H
#define ADV_STRING_H

#ifdef UNIX
#include <stdint.h>
#endif
#include <string>

namespace adv{

class String{
  friend std::ostream& operator<<(std::ostream& strm, const String& str);
  friend std::istream& operator>>(std::istream& strm, String& str);
  friend String operator+(const char* ptr, const String& str);
public:
  String() : mStr() {}
  String(const char* str) : mStr(str ? str : "") {}
  String(const std::string& str) : mStr(str) {}
  String& operator+=(const String& s) {mStr += s.mStr; return *this;}
  String& operator+=(const char& ch) {mStr += ch; return *this;}
  char& operator[](int i) {return mStr[i];}
  String& erase (unsigned pos = 0, size_t len = npos) {mStr.erase(pos, len); return *this;}
  void clear() {mStr.clear();}

  String operator+(const String& s);
  String operator+(const char* s);
  bool operator==(const String& other) const {return mStr == other.mStr;}
  bool operator!=(const String& other) const {return mStr != other.mStr;}
  bool operator<(const String& s) const { return mStr < s.mStr;}
  const char* c_str() const {return mStr.c_str();}
  bool empty() const {return mStr.empty();}
  unsigned size() const {return (unsigned)mStr.size();}
  void resize(size_t n) { mStr.resize(n); }
  String& append(size_t n, char c) { mStr.append(n, c); return *this; }
  const char& operator[](int i) const {return mStr[i];}

  size_t find(char c, size_t pos = 0) { return mStr.find(c, pos); }

  String substr(unsigned pos = 0, size_t len = npos) const {String ret(mStr.substr(pos, len)); return ret;}

  //operator std::string() {return mStr;}
  operator const std::string&() const {return mStr;}

  const char& back() { return mStr[mStr.size() - 1]; }

  String removeAll(char ch) const;
  String toLower() const;
  bool ieqauls(const String& other);
  void trim();
  void replaceWith(char oldCh, char newCh);
  bool endsWith(const String& str);

  static const size_t npos = SIZE_MAX;
protected:
  std::string mStr;
};

std::ostream& operator<<(std::ostream& strm, const String& str);
std::istream& operator>>(std::istream& strm, String& str);
String operator+(const char* ptr, const String& str);

}

#endif
