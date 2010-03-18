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
    CharacterObject* getSpeaker() {return mSpeaker;}
    void setExtent(const Vec2i& extent);
  protected:
    Vec2i mPos;
    Vec2i mCenterOffset;
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
    String* render(int x, int y, const std::string& text, int depth, const Color& color, unsigned displayTime, const std::vector<Vec2i>& breakinfo);
    Vec2i getTextExtent(const std::string& text, std::vector<Vec2i>& breakinfo);
    void blit(unsigned interval);
    void removeText(CharacterObject* chr);
    void removeText(String* str);
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
  bool loadFont(unsigned id);
  void unloadFont(unsigned id);
  String* render(int x, int y, const std::string& text, int depth, int fontid, const std::vector<Vec2i>& breakinfo, const Color& col=Color(), unsigned displayTime=0);
  Vec2i getTextExtent(const std::string& text, int fontid, std::vector<Vec2i>& breakinfo);
  void prepareBlit(unsigned interval);
  void removeText(CharacterObject* chr);
  void removeText(String* str);
protected:
  AdvDocument* mData;
  std::vector<Font*> mFonts;
};

#endif
