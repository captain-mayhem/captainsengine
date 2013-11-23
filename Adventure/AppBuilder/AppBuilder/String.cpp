#include "String.h"

using namespace adv;

String String::operator+(const String& s){
  std::string ret = (std::string)(*this)+(std::string)s;
  return ret.c_str();
}

namespace adv{

std::ostream& operator<<(std::ostream& strm, const String& str){
  strm << str.size() << " " << str.data();
  return strm;
}

std::istream& operator>>(std::istream& strm, String& str){
  unsigned size;
  strm >> size;
  char tmp;
  strm.get(tmp);
  str.resize(size);
  strm.read((char*)str.data(), size);
  return strm;
}

}
