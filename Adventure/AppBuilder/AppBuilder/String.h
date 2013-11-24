#ifndef ADV_STRING_H
#define ADV_STRING_H

#include <string>

namespace adv{

class String : public std::string{
  friend std::ostream& operator<<(std::ostream& strm, const String& str);
  friend std::istream& operator>>(std::istream& strm, String& str);
public:
  String() : std::string() {}
  String(const char* str) : std::string(str) {}
  String operator+(const String& s);
  String operator+(const char* s);

  String removeAll(char ch) const;
};

std::ostream& operator<<(std::ostream& strm, const String& str);
std::istream& operator>>(std::istream& strm, String& str);

}

#endif
