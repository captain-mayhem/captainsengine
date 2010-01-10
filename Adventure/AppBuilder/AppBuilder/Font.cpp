#include "Font.h"
#include "AdvDoc.h"
#include "Engine.h"


class FontBlitObject : public BlitObject{
public:
  FontBlitObject(GLuint masktexture, GLuint texture, const Vec2i& size, 
    const Vec2f& scale, int depth, const Vec2i& offset, const Vec2f& textrans, const Color& color) : 
BlitObject(texture, size, scale, depth, offset){
    mTexTrans = textrans;
    mMaskTexture = masktexture;
    mColor = color;
  }
  virtual ~FontBlitObject(){

  }
  virtual void blit(){
    glPushMatrix();
    glTranslatef(mPos.x,mPos.y,0.0f);
    glScalef(mSize.x,mSize.y,1.0f);
    //if (mMirrorX)
    //  glScalef(-1.,1.,1.);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(mScale.x, mScale.y, 1.0f);
    glTranslatef(mTexTrans.x, mTexTrans.y, 0);
    glMatrixMode(GL_MODELVIEW);
    glBindTexture(GL_TEXTURE_2D, mTex);
    glColor4ub(mColor.r,mColor.g,mColor.b,mColor.a);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glColor4ub(255,255,255,255);
    glPopMatrix();
  }
protected:
  Vec2f mTexTrans;
  GLuint mMaskTexture;
  Color mColor;
};

////////////////////////////////////////////////

FontRenderer::String::String(const Vec2i& pos, unsigned displayTime) : 
mPos(pos), mDisplayTime(displayTime), mSuspensionScript(NULL), mSpeaker(NULL), mCenterOffset(){
}

FontRenderer::String::~String(){
  if (mSuspensionScript){
    mSuspensionScript->resume();
    mSuspensionScript = NULL;
  }
  if (mSpeaker){
    mSpeaker->setState(CharacterObject::calculateState(mSpeaker->getState(), mSpeaker->isWalking(), false));
    mSpeaker = NULL;
  }
  clear();
}

void FontRenderer::String::clear(){
  for (unsigned i = 0; i < mString.size(); ++i){
    delete mString[i];
  }
  mString.clear();
}

void FontRenderer::String::render(unsigned interval){
  Vec2i pos = mPos;
  if (mSpeaker){
    pos = mSpeaker->getOverheadPos()-mCenterOffset;
  }
  for (unsigned i = 0; i < mString.size(); ++i){
    mString[i]->render(pos, false);
  }
  mDisplayTime -= interval;
}

void FontRenderer::String::setExtent(const Vec2i& extent){
  mCenterOffset = extent;
  mCenterOffset.x /= 2;
}

////////////////////////////////////////

FontRenderer::Font::Font(const FontData& data){
  mFontsize = data.fontsize;
  mNumChars = data.numChars;
  mCharwidths = data.charwidths;
  mImages.reserve(data.images.size()/2);
  for (unsigned i = 0; i < data.images.size()/2; ++i){
    mImages.push_back(Engine::instance()->genTexture(data.images[2*i+1], mTexSize, mScale, &data.images[2*i]));
  }
  mScale = Vec2f(mFontsize.x/(float)mTexSize.x, mFontsize.y/(float)mTexSize.y);
}

FontRenderer::Font::~Font(){
  for (std::list<String*>::iterator iter = mRenderQueue.begin(); iter != mRenderQueue.end(); ++iter){
    delete *iter;
  }
  mRenderQueue.clear();
}

FontRenderer::String& FontRenderer::Font::render(int x, int y, const std::string& text, int depth, const Color& color, unsigned displayTime){
  String* str = new String(Vec2i(x,y), displayTime);
  int xoffset = 0;
  for (unsigned i = 0; i < text.size(); ++i){
    unsigned char charnum = ((unsigned char)text[i])-0x20;
    int chardeviation = mCharwidths[charnum];
    int texnum = charnum/(mNumChars.x*mNumChars.y);
    charnum %= mNumChars.x*mNumChars.y;
    int rownum = charnum/mNumChars.x;
    charnum %= mNumChars.x;
    BlitObject* obj = new FontBlitObject(mImages[texnum], mImages[texnum], 
      mFontsize, mScale, depth, Vec2i(xoffset,0), Vec2f(charnum, rownum), color);
    str->append(obj);
    xoffset += chardeviation;
  }
  str->setExtent(Vec2i(xoffset,mFontsize.y));
  mRenderQueue.push_back(str);
  return *str;
}

Vec2i FontRenderer::Font::getTextExtent(const std::string& text){
  unsigned accu = 0;
  for (unsigned i = 0; i < text.size(); ++i){
    unsigned char charnum = ((unsigned char)text[i])-0x20;
    accu += mCharwidths[charnum];
  }
  return Vec2i(accu,mFontsize.y);
}

void FontRenderer::Font::blit(unsigned interval){
  for (std::list<String*>::iterator iter = mRenderQueue.begin(); iter != mRenderQueue.end(); ++iter){
    if ((*iter)->getTime() < 0){
      delete *iter;
      iter = mRenderQueue.erase(iter);
    }
    else{
      (*iter)->render(interval);
    }
    if (iter == mRenderQueue.end())
      break;
  }
}

void FontRenderer::Font::removeText(CharacterObject* chr){
  for (std::list<String*>::iterator iter = mRenderQueue.begin(); iter != mRenderQueue.end(); ++iter){
    if ((*iter)->getSpeaker() == chr){
      delete *iter;
      iter = mRenderQueue.erase(iter);
      break;
    }
  }
}

//////////////////////////////////////////////

FontRenderer::FontRenderer(AdvDocument* data):  mData(data){

}

FontRenderer::~FontRenderer(){
  for (unsigned i = 0; i < mFonts.size(); ++i){
    delete mFonts[i];
  }
}

void FontRenderer::loadFont(unsigned id){
  if (mFonts.size() <= id){
    mFonts.resize(id+1);
  }
  if (mFonts[id] == NULL){
    FontData fnt = mData->getFont(id);
    mFonts[id] = new Font(fnt);
  }
}

void FontRenderer::unloadFont(unsigned id){
  delete mFonts[id];
  mFonts[id] = NULL;
}

FontRenderer::String& FontRenderer::render(int x, int y, const std::string& text, int depth, int fontid, const Color& color, unsigned displayTime){
  return mFonts[fontid]->render(x, y, text, depth, color, displayTime);
}

Vec2i FontRenderer::getTextExtent(const std::string& text, int fontid){
  return mFonts[fontid]->getTextExtent(text);
}

void FontRenderer::prepareBlit(unsigned interval){
  for (unsigned i = 0; i < mFonts.size(); ++i){
    if (mFonts[i])
      mFonts[i]->blit(interval);
  }
}

void FontRenderer::removeText(CharacterObject* chr){
  for (unsigned i = 0; i < mFonts.size(); ++i){
    if (mFonts[i])
      mFonts[i]->removeText(chr);
  }
}
