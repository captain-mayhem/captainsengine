#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

using std::string;

namespace Graphics{

class Texture{
public:
  Texture(string filename);
  virtual ~Texture();
  virtual void activate()=0;
  virtual void deactivate()=0;
  static Texture** loadFromDat(const string& path, const string& filename, int& number);
  //! load a texture
  static Texture* create(const std::string& filename);
  //! get a filename
  std::string getFilename() const {return filename_;}
  //! get the name
  std::string getName() const;
protected:
  virtual bool load(string filename)=0;
  //! the filename
  std::string filename_;
};

}

#endif

