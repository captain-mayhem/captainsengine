#include "Font.h"
#include "AdvDoc.h"
#include "Engine.h"
#include "Textout.h"
#include <system/allocation.h>

using namespace adv;

namespace adv{

class FontBlitObject : public BlitObject{
public:
  FontBlitObject(CGE::Texture* masktexture, CGE::Texture* texture, const Vec2i& size, 
    const Vec2f& scale, int depth, const Vec2i& offset, const Vec2f& textrans, const Color& color) : 
BlitObject(texture, size, scale, depth, offset){
    mTexTrans = textrans;
    mMaskTexture = masktexture;
    mColor = color;
  }
  virtual ~FontBlitObject(){

  }
  virtual void blit(){
    CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
    rend->pushMatrix();
    rend->translate((float)mPos.x,(float)mPos.y,0.0f);
    rend->scale((float)mSize.x,(float)mSize.y,1.0f);
    //if (mMirrorX)
    //  glScalef(-1.,1.,1.);
    rend->switchMatrixStack(CGE::MatTexture);
    rend->resetModelView();
    rend->scale(mScale.x, mScale.y, 1.0f);
    rend->translate(mTexTrans.x, mTexTrans.y, 0);
    rend->switchMatrixStack(CGE::Modelview);
    mTex->activate();
    rend->setColor(mColor.r / 255.0f, mColor.g / 255.0f, mColor.b / 255.0f, mColor.a / 255.0f);
    Engine::instance()->drawQuad();
    rend->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    rend->popMatrix();
  }
  void setOpacity(unsigned char opacity){
    mColor.a = opacity;
  }
protected:
  Vec2f mTexTrans;
  CGE::Texture* mMaskTexture;
  Color mColor;
};

}

////////////////////////////////////////////////

FontRenderer::String::String(const Vec2i& pos, unsigned displayTime, bool keepOnScreen, unsigned fading) : 
mPos(pos), mDisplayTime(displayTime), mSuspensionScript(NULL), mSpeaker(NULL), mUnregisterSpeaker(true), mCenterOffset(), mKeepOnScreen(keepOnScreen), mBoundRoom(NULL), mFadingTime(fading), mTimeShown(0){
  if (mDisplayTime > 0 && mDisplayTime < 2*mFadingTime)
    mDisplayTime = 2*mFadingTime;
}

FontRenderer::String::~String(){
  endDisplaying(true);
  clear();
}

void FontRenderer::String::endDisplaying(bool immediate){
  if (mSuspensionScript){
    mSuspensionScript->resume();
    mSuspensionScript->unref();
    mSuspensionScript = NULL;
  }
  if (mSpeaker){
    if (mUnregisterSpeaker){
      mSpeaker->setTalking(false);
      if (Engine::instance()->getBlockingSpeaker() == mSpeaker)
        Engine::instance()->setBlockingSpeaker(NULL);
    }
    if (immediate)
      mSpeaker = NULL;
    else
      mUnregisterSpeaker = false;
  }
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
  if (mKeepOnScreen){
    if (pos.x+mCenterOffset.x*2 > Engine::instance()->getResolution().x && pos.x+mCenterOffset.x*2 < Engine::instance()->getResolution().x+mCenterOffset.x*2)
      pos.x = Engine::instance()->getResolution().x-mCenterOffset.x*2;
    if (pos.x < 0 && pos.x > -mCenterOffset.x*2)
      pos.x = 0;
    if (pos.y+mCenterOffset.y > Engine::instance()->getResolution().y && pos.y+mCenterOffset.y < Engine::instance()->getResolution().y+mCenterOffset.y)
      pos.y = Engine::instance()->getResolution().y-mCenterOffset.y;
    if (pos.y < 0/* && pos.y > -mCenterOffset.y/3*2*/)
      pos.y = 0;
  }
  //opacity
  if (mDisplayTime > 0){
    unsigned char opacity = 255;
    if (mFadingTime > 0){
      float factor;
      if (mTimeShown < mFadingTime)
        factor = mTimeShown/(float)mFadingTime;
      else
        factor = mDisplayTime/(float)mFadingTime;
      if (factor > 1.0f)
        factor = 1.0f;
      opacity = (unsigned char)(factor*255);
    }
    for (unsigned i = 0; i < mString.size(); ++i){
      mString[i]->setOpacity(opacity);
      mString[i]->render(pos, Vec2f(1.0f,1.0f), Vec2i());
    }
  }
  else{
    for (unsigned i = 0; i < mString.size(); ++i){
      mString[i]->render(pos, Vec2f(1.0f,1.0f), Vec2i());
    }
  }
  mDisplayTime -= interval;
  mTimeShown += interval;
  if (interval == 0 && mDisplayTime == 0)
    mDisplayTime = -1;
}

void FontRenderer::String::setExtent(const Vec2i& extent){
  mCenterOffset = extent;
  mCenterOffset.x /= 2;
}

void FontRenderer::String::setSuspensionScript(ExecutionContext* ctx){
  if (mSuspensionScript != NULL){
    mSuspensionScript->reset(true,true);
    mSuspensionScript->unref();
  }
  ctx->ref();
  mSuspensionScript = ctx;
}

void FontRenderer::String::remove(){
  mDisplayTime = mFadingTime;
  endDisplaying(false);
}

void FontRenderer::String::setOpacity(unsigned char opacity){
  for (unsigned i = 0; i < mString.size(); ++i){
    mString[i]->setOpacity(opacity);
  }
}

////////////////////////////////////////

FontRenderer::Font::Font(const FontData& data, int fading, bool unified) : mFading(fading){
  mFontsize = data.fontsize;
  mNumChars = data.numChars;
  mGlyphMap = data.glyphmap;
  if (unified){
    mImages.reserve(data.images.size());
    for (unsigned i = 0; i < data.images.size(); ++i){
      mImages.push_back(Engine::instance()->genTexture(data.images[i], mTexSize, mScale));
    }
  }
  else{
    mImages.reserve(data.images.size() / 2);
    for (unsigned i = 0; i < data.images.size() / 2; ++i){
      mImages.push_back(Engine::instance()->genTexture(data.images[2 * i + 1], mTexSize, mScale, data.images[2 * i]));
    }
  }
  mScale = Vec2f(mFontsize.x/(float)mTexSize.x, mFontsize.y/(float)mTexSize.y);
}

FontRenderer::Font::~Font(){
  for (std::list<String*>::iterator iter = mRenderQueue.begin(); iter != mRenderQueue.end(); ++iter){
    delete *iter;
  }
  mRenderQueue.clear();
  for (size_t i = 0; i < mImages.size(); ++i){
    delete mImages[i];
  }
}

FontRenderer::String* FontRenderer::Font::render(int x, int y, const std::string& text, int depth, const Color& color, unsigned displayTime, const std::vector<Vec2i>& breakinfo, bool keepOnScreen){
  String* str = new String(Vec2i(x,y), displayTime, keepOnScreen, mFading);
  unsigned max_len = 0;
  for (unsigned i = 0; i < breakinfo.size(); ++i){
    max_len = breakinfo[i].y > (int)max_len ? breakinfo[i].y : max_len;
  }
  unsigned line = 0;
  int xoffset = (max_len-breakinfo[line].y)/2;
  int yoffset = 0;
  for (unsigned i = 0; i < text.size(); ++i){
    unsigned char charnum = ((unsigned char)text[i])-0x20;
    FontData::Glyph& gl = mGlyphMap[charnum];
    int chardeviation = gl.advwidth;
    int texnum = charnum/(mNumChars.x*mNumChars.y);
    FontBlitObject* obj = new FontBlitObject(mImages[texnum], mImages[texnum], 
      Vec2f((float)gl.w, (float)gl.h), Vec2f(gl.w/(float)mTexSize.x,gl.h/(float)mTexSize.y), depth, Vec2i(xoffset+gl.xoffset,yoffset+gl.yoffset), Vec2f(gl.x/(float)gl.w, gl.y/(float)gl.h), color);
    str->append(obj);
    xoffset += chardeviation;
    if (i >= (unsigned)breakinfo[line].x){
      //line break
      ++line;
      xoffset = (max_len-breakinfo[line].y)/2;
      yoffset += mFontsize.y;
    }
  }
  str->setExtent(Vec2i(max_len,mFontsize.y*(unsigned)breakinfo.size()));
  mRenderQueue.push_back(str);
  return str;
}

Vec2i FontRenderer::Font::getTextExtent(const std::string& text, std::vector<Vec2i>& breakinfo, unsigned maxStringWidth){
  unsigned accu = 0;
  unsigned wordaccu = 0;
  unsigned max_line = 0;
  unsigned last_idx = 0;
  for (unsigned i = 0; i < text.size(); ++i){
    unsigned char charnum = ((unsigned char)text[i])-0x20;
    FontData::Glyph& gl = mGlyphMap[charnum];
    wordaccu += gl.advwidth;
    if (text[i] == ' '){
      if (accu + wordaccu > maxStringWidth){
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
  breakinfo.push_back(Vec2i((int)text.size(), accu));
  max_line = accu > max_line ? accu : max_line;
  return Vec2i(max_line, mFontsize.y*(unsigned)breakinfo.size());
}

void FontRenderer::Font::blit(unsigned interval, RoomObject* mainroom, bool renderBoundText){
  for (std::list<String*>::iterator iter = mRenderQueue.begin(); iter != mRenderQueue.end(); ){
    if (!renderBoundText && mainroom != NULL && (*iter)->getRoom() == mainroom){
      ++iter;
      continue;
    }
    if (renderBoundText && (mainroom == NULL || (*iter)->getRoom() != mainroom)){
      ++iter;
      continue;
    }
    if ((*iter)->getTime() < 0){
      delete *iter;
      iter = mRenderQueue.erase(iter);
    }
    else{
      (*iter)->render(interval);
      ++iter;
    }
    if (iter == mRenderQueue.end())
      break;
  }
}

void FontRenderer::Font::removeText(CharacterObject* chr, bool immediately){
  for (std::list<String*>::iterator iter = mRenderQueue.begin(); iter != mRenderQueue.end(); ++iter){
    if ((*iter)->getSpeaker() == chr){
      if (immediately){
        delete *iter;
        iter = mRenderQueue.erase(iter);
      }
      else
        (*iter)->remove();
    }
    if (iter == mRenderQueue.end())
      break;
  }
}

void FontRenderer::Font::removeText(String* str){
  for (std::list<String*>::iterator iter = mRenderQueue.begin(); iter != mRenderQueue.end(); ++iter){
    if ((*iter) == str){
      (*iter)->remove();
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
  clearTextouts();
}

bool FontRenderer::loadFont(unsigned id){
  if (mFonts.size() <= id){
    mFonts.resize(id+1);
  }
  if (mFonts[id] == NULL){
    FontData fnt = mData->getFont(id);
    if (fnt.images.empty())
      return false;
    int fading = id == 0 ? 0 : mData->getProjectSettings()->font_fading[id-1];
    mFonts[id] = new Font(fnt, fading, mData->hasUnifiedFonts());
    fnt.destroyImages();
  }
  return true;
}

void FontRenderer::unloadFont(unsigned id){
  delete mFonts[id];
  mFonts[id] = NULL;
}

FontRenderer::String* FontRenderer::render(int x, int y, const std::string& text, int depth, int fontid, const std::vector<Vec2i>& breakinfo, const Color& color, unsigned displayTime, bool keepOnScreen){
  if (mFonts[fontid] == NULL)
    return NULL;
  return mFonts[fontid]->render(x, y, text, depth, color, displayTime, breakinfo, keepOnScreen);
}

Vec2i FontRenderer::getTextExtent(const std::string& text, int fontid, std::vector<Vec2i>& breakinfo, int maxStringWidth){
  if (mFonts[fontid] == NULL)
    return Vec2i(0, 0);
  return mFonts[fontid]->getTextExtent(text, breakinfo, maxStringWidth);
}

void FontRenderer::prepareTextouts(unsigned time){
  for (std::map<int,Textout*>::iterator iter = mTextouts.begin(); iter != mTextouts.end(); ++iter){
    iter->second->render(time);
  }
}

void FontRenderer::prepareBlit(unsigned interval, RoomObject* mainroom, bool renderBoundText){
  for (unsigned i = 0; i < mFonts.size(); ++i){
    if (mFonts[i])
      mFonts[i]->blit(interval, mainroom, renderBoundText);
  }
}

void FontRenderer::removeText(CharacterObject* chr, bool immediately){
  for (unsigned i = 0; i < mFonts.size(); ++i){
    if (mFonts[i])
      mFonts[i]->removeText(chr, immediately);
  }
}

void FontRenderer::removeText(String* str){
  for (unsigned i = 0; i < mFonts.size(); ++i){
    if (mFonts[i])
      mFonts[i]->removeText(str);
  }
}

Textout* FontRenderer::getTextout(int id){
  std::map<int, Textout*>::iterator iter = mTextouts.find(id);
  if (iter == mTextouts.end()){
    Textout* text = new Textout();
    mTextouts[id] = text;
    return text;
  }
  return iter->second;
}

void FontRenderer::clearTextouts(){
  for (std::map<int, Textout*>::iterator iter = mTextouts.begin(); iter != mTextouts.end(); ++iter){
    delete iter->second;
  }
  mTextouts.clear();
}

void FontRenderer::enableTextouts(bool enable){
  for (std::map<int, Textout*>::iterator iter = mTextouts.begin(); iter != mTextouts.end(); ++iter){
    iter->second->setEnabled(enable);
  }
}

void FontRenderer::save(std::ostream& out){
  out << mTextouts.size() << "\n";
  for (std::map<int,Textout*>::iterator iter = mTextouts.begin(); iter != mTextouts.end(); ++iter){
    out << iter->first << " ";
    iter->second->save(out);
  }
}

void FontRenderer::load(std::istream& in){
  unsigned numTextouts;
  in >> numTextouts;
  for (std::map<int,Textout*>::iterator iter = mTextouts.begin(); iter != mTextouts.end(); ++iter){
    delete iter->second;
  }
  mTextouts.clear();
  for (unsigned i = 0; i < numTextouts; ++i){
    int id;
    in >> id;
    Textout* to = new Textout();
    to->load(in);
    mTextouts[id] = to;
  }
}

void FontRenderer::disableBoundTextouts(RoomObject* room){
  for (std::map<int, Textout*>::iterator iter = mTextouts.begin(); iter != mTextouts.end(); ++iter){
    Textout* txt = iter->second;
    if (txt->getBoundRoom() == room->getName()){
      txt->setEnabled(false);
    }
  }
}
