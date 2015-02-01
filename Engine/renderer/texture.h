#ifndef CGE_TEXTURE_H
#define CGE_TEXTURE_H

#include <string>

using std::string;

namespace CGE{

class Image;

class Texture{
public:
  enum Format{
    AUTO = 0,
    GRAY = 1,
    RGB = 3,
    RGBA = 4,
    DEPTH,
    ALPHA,
  };
  Texture();
  virtual ~Texture();
  virtual void activate() const =0;
  virtual void deactivate() const =0;
  virtual bool createFromImage(Image* img, Format fmt) = 0;
  virtual bool createEmpty(unsigned width, unsigned height, Format fmt) = 0;
  static Texture** loadFromDat(const string& path, const string& filename, int& number);
  //! load a texture
  static Texture* create(const std::string& filename);
  //! get a filename
  std::string getFilename() const {return filename_;}
  //! set the filename
  void setFilename(std::string const& file) { filename_ = file; }
  //! get the name
  std::string getName() const;
protected:
  //! the filename
  std::string filename_;
};

}

#endif

