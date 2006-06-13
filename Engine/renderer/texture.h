#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

using std::string;

namespace Graphics{

class Texture{
public:
  Texture(string filename){}
  virtual ~Texture(){}
  virtual void activate()=0;
  static Texture** loadFromDat(const string& path, const string& filename, int& number);
protected:
  virtual bool load(string filename)=0;
};

}

#endif

