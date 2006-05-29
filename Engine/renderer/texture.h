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
protected:
  virtual bool load(string filename)=0;
};

}

#endif

