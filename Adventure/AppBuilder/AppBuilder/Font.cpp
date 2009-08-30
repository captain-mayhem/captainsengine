#include "Font.h"
#include "AdvDoc.h"
#include "Engine.h"


class FontBlitObject : public BlitObject{
public:
  FontBlitObject(GLuint masktexture, GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset, const Vec2f& textrans) : 
BlitObject(texture, size, scale, depth, offset){
    mTexTrans = textrans;
    mMaskTexture = masktexture;
  }
  virtual ~FontBlitObject(){

  }
  virtual void FontBlitObject::blit(){
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
    //glColor3f(1.0,0.0,0.0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //glColor3f(1.0,1.0,1.0);
    glPopMatrix();
  }
protected:
  Vec2f mTexTrans;
  GLuint mMaskTexture;
};

////////////////////////////////////////////////

FontRenderer::String::String(){
}

FontRenderer::String::~String(){
  clear();
}

void FontRenderer::String::clear(){
  for (unsigned i = 0; i < mString.size(); ++i){
    delete mString[i];
  }
  mString.clear();
}

void FontRenderer::String::render(const Vec2i& pos){
  for (unsigned i = 0; i < mString.size(); ++i){
    mString[i]->render(pos, false);
  }
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
}

void FontRenderer::Font::render(int x, int y, const std::string& text){
  static String str;
  str.clear();
  int xoffset = 0;
  for (unsigned i = 0; i < text.size(); ++i){
    char charnum = text[i]-0x20;
    int chardeviation = mCharwidths[charnum];
    int texnum = charnum/(mNumChars.x*mNumChars.y);
    charnum %= mNumChars.x*mNumChars.y;
    int rownum = charnum/mNumChars.x;
    charnum %= mNumChars.x;
    BlitObject* obj = new FontBlitObject(mImages[texnum], mImages[texnum], 
      mFontsize, mScale, 15000, Vec2i(xoffset,0), Vec2f(charnum, rownum));
    str.append(obj);
    xoffset += chardeviation;
  }
  str.render(Vec2i(x,y));
}

unsigned FontRenderer::Font::getTextExtent(const std::string& text){
  unsigned accu = 0;
  for (unsigned i = 0; i < text.size(); ++i){
    char charnum = text[i]-0x20;
    accu += mCharwidths[charnum];
  }
  return accu;
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

void FontRenderer::render(int x, int y, const std::string& text, int fontid){
  mFonts[fontid]->render(x, y, text);
}

unsigned FontRenderer::getTextExtent(const std::string& text, int fontid){
  return mFonts[fontid]->getTextExtent(text);
}
