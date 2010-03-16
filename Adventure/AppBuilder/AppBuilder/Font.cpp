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
  if (pos.x+mCenterOffset.x*2 > Engine::instance()->getResolution().x && pos.x+mCenterOffset.x*2 < Engine::instance()->getResolution().x+mCenterOffset.x)
    pos.x = Engine::instance()->getResolution().x-mCenterOffset.x*2;
  if (pos.x < 0 && pos.x > -mCenterOffset.x)
    pos.x = 0;
  if (pos.y+mCenterOffset.y > Engine::instance()->getResolution().y && pos.y+mCenterOffset.y < Engine::instance()->getResolution().y+mCenterOffset.y/2)
    pos.y = Engine::instance()->getResolution().y-mCenterOffset.y;
  if (pos.y < 0 && pos.y > -mCenterOffset.y/3*2)
    pos.y = 0;
  for (unsigned i = 0; i < mString.size(); ++i){
    mString[i]->render(pos, false, Vec2i());
  }
  mDisplayTime -= interval;
}

void FontRenderer::String::setExtent(const Vec2i& extent){
  mCenterOffset = extent;
  mCenterOffset.x /= 2;
}

////////////////////////////////////////

static const int max_string_width = 300;

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

FontRenderer::String& FontRenderer::Font::render(int x, int y, const std::string& text, int depth, const Color& color, unsigned displayTime, const std::vector<Vec2i>& breakinfo){
  String* str = new String(Vec2i(x,y), displayTime);
  unsigned max_len = 0;
  for (unsigned i = 0; i < breakinfo.size(); ++i){
    max_len = breakinfo[i].y > (int)max_len ? breakinfo[i].y : max_len;
  }
  unsigned line = 0;
  int xoffset = (max_len-breakinfo[line].y)/2;
  int yoffset = 0;
  for (unsigned i = 0; i < text.size(); ++i){
    unsigned char charnum = ((unsigned char)text[i])-0x20;
    int chardeviation = mCharwidths[charnum];
    int texnum = charnum/(mNumChars.x*mNumChars.y);
    charnum %= mNumChars.x*mNumChars.y;
    int rownum = charnum/mNumChars.x;
    charnum %= mNumChars.x;
    BlitObject* obj = new FontBlitObject(mImages[texnum], mImages[texnum], 
      mFontsize, mScale, depth, Vec2i(xoffset,yoffset), Vec2f(charnum, rownum), color);
    str->append(obj);
    xoffset += chardeviation;
    if (i >= (unsigned)breakinfo[line].x){
      //line break
      ++line;
      xoffset = (max_len-breakinfo[line].y)/2;
      yoffset += mFontsize.y;
    }
  }
  str->setExtent(Vec2i(max_len,mFontsize.y*breakinfo.size()));
  mRenderQueue.push_back(str);
  return *str;
}

Vec2i FontRenderer::Font::getTextExtent(const std::string& text, std::vector<Vec2i>& breakinfo){
  unsigned accu = 0;
  unsigned wordaccu = 0;
  unsigned max_line = 0;
  unsigned last_idx = 0;
  for (unsigned i = 0; i < text.size(); ++i){
    unsigned char charnum = ((unsigned char)text[i])-0x20;
    wordaccu += mCharwidths[charnum];
    if (text[i] == ' '){
      if (accu + wordaccu > max_string_width){
        breakinfo.push_back(Vec2i(last_idx, accu));
        max_line = accu > max_line ? accu : max_line;
        accu = 0;
      }
      accu += wordaccu;
      last_idx = i;
      wordaccu = 0;
    }
  }
  accu += wordaccu;
  breakinfo.push_back(Vec2i(text.size(), accu));
  max_line = accu > max_line ? accu : max_line;
  return Vec2i(max_line, mFontsize.y*breakinfo.size());
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

bool FontRenderer::loadFont(unsigned id){
  if (mFonts.size() <= id){
    mFonts.resize(id+1);
  }
  if (mFonts[id] == NULL){
    FontData fnt = mData->getFont(id);
    if (fnt.images.empty())
      return false;
    mFonts[id] = new Font(fnt);
  }
  return true;
}

void FontRenderer::unloadFont(unsigned id){
  delete mFonts[id];
  mFonts[id] = NULL;
}

FontRenderer::String& FontRenderer::render(int x, int y, const std::string& text, int depth, int fontid, const std::vector<Vec2i>& breakinfo, const Color& color, unsigned displayTime){
  return mFonts[fontid]->render(x, y, text, depth, color, displayTime, breakinfo);
}

Vec2i FontRenderer::getTextExtent(const std::string& text, int fontid, std::vector<Vec2i>& breakinfo){
  return mFonts[fontid]->getTextExtent(text, breakinfo);
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
