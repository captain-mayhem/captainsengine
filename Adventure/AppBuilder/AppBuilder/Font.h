#ifndef FONT_H
#define FONT_H

#include <queue>
#include <list>
#include "EngineObjects.h"

class FontRenderer{
public:
  class String{
  public:
    String(const Vec2i& pos, unsigned displayTime);
    //String(const String& str);
    ~String();
    void append(BlitObject* obj) {mString.push_back(obj);}
    void clear();
    void render(unsigned interval);
    int getTime() {return mDisplayTime;}
    void setSuspensionScript(ExecutionContext* ctx) {mSuspensionScript = ctx;}
    void setSpeaker(CharacterObject* chr) {mSpeaker = chr;}
  protected:
    Vec2i mPos;
    int mDisplayTime;
    std::vector<BlitObject*> mString;
    ExecutionContext* mSuspensionScript;
    CharacterObject* mSpeaker;
  };
protected:
  class Font{
  public:
    Font(const FontData& data);
    ~Font();
    String& render(int x, int y, const std::string& text, const Color& color, unsigned displayTime);
    Vec2i getTextExtent(const std::string& text);
    void blit(unsigned interval);
  protected:
    void buildBlitObject();
    Vec2i mFontsize;
    Vec2i mNumChars;
    Vec2i mTexSize;
    Vec2f mScale;
    std::vector<GLuint> mImages;
    std::vector<unsigned> mCharwidths;
    std::list<String*> mRenderQueue;
  };
public:
  FontRenderer(AdvDocument* data);
  ~FontRenderer();
  void loadFont(unsigned id);
  void unloadFont(unsigned id);
  String& render(int x, int y, const std::string& text, int fontid, const Color& col=Color(), unsigned displayTime=0);
  Vec2i getTextExtent(const std::string& text, int fontid);
  void prepareBlit(unsigned interval);
protected:
  AdvDocument* mData;
  std::vector<Font*> mFonts;
};

#endif
