#ifndef FONT_H
#define FONT_H

#include "EngineObjects.h"

class FontRenderer{
public:
  class String{
  public:
    String();
    ~String();
    void append(BlitObject* obj) {mString.push_back(obj);}
    void clear();
    void render(const Vec2i& pos);
  protected:
    std::vector<BlitObject*> mString;
  };
protected:
  class Font{
  public:
    Font(const FontData& data);
    ~Font();
    void render(int x, int y, const std::string& text);
    unsigned getTextExtent(const std::string& text);
  protected:
    void buildBlitObject();
    Vec2i mFontsize;
    Vec2i mNumChars;
    Vec2i mTexSize;
    Vec2f mScale;
    std::vector<GLuint> mImages;
    std::vector<unsigned> mCharwidths;
  };
public:
  FontRenderer(AdvDocument* data);
  ~FontRenderer();
  void loadFont(unsigned id);
  void unloadFont(unsigned id);
  void render(int x, int y, const std::string& text, int fontid);
  unsigned getTextExtent(const std::string& text, int fontid);
protected:
  AdvDocument* mData;
  std::vector<Font*> mFonts;
};

#endif
