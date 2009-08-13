#include "EngineObjects.h"
#include <wx/image.h>
#include "Engine.h"

BlitObject::BlitObject(std::string texture, int depth, Vec2i offset) : 
mDepth(depth), mOffset(offset){
  genTexture(texture);
}

BlitObject::~BlitObject(){
  glDeleteTextures(1, &mTex);
}

//TODO move into utility
unsigned roundToPowerOf2(unsigned x){
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

void BlitObject::genTexture(const std::string& name){
  std::string filename = Engine::instance()->resolveFilename(IMAGE, name);
  wxImage image(filename);
  mSize.x = image.GetWidth();
  mSize.y = image.GetHeight();
  Vec2i pow2(roundToPowerOf2(mSize.x), roundToPowerOf2(mSize.y));
  mScale.x = ((float)mSize.x)/pow2.x;
  mScale.y = ((float)mSize.y)/pow2.y;
  unsigned totalsize = mSize.x*mSize.y;
  unsigned char* rgb = image.GetData();
  unsigned char* alpha = image.GetAlpha();
  GLubyte* buffer = new GLubyte[totalsize*4];
  for (unsigned i = 0; i < totalsize; ++i){
    buffer[4*i] = rgb[3*i];
    buffer[4*i+1] = rgb[3*i+1];
    buffer[4*i+2] = rgb[3*i+2];
    if (alpha)
      buffer[4*i+3] = alpha[i];
    else{
      if (rgb[3*i] == 0xFF && rgb[3*i+1] == 0x0 && rgb[3*i+2] == 0xFF){
        buffer[4*i+3] = 0;
      }
      else{
        buffer[4*i+3] = 255;
      }
    }
  }
  glGenTextures(1,&mTex);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2.x, pow2.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSize.x, mSize.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  delete buffer;
}

bool BlitObject::operator<(const BlitObject& obj){
  return mDepth < obj.mDepth;
}

void BlitObject::render(Vec2i pos){
  mPos.x = mOffset.x+pos.x;
  mPos.y = mOffset.y+pos.y;
  Engine::instance()->insertToBlit(this);
}

void BlitObject::blit(){
  glPushMatrix();
  glTranslatef(mPos.x,mPos.y,0.0f);
  glScalef(mSize.x,mSize.y,1.0f);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glScalef(mScale.x, mScale.y, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glPopMatrix();
}

///

BlitGroup::BlitGroup(std::vector<std::string> textures, std::vector<Vec2i> offsets, int depth){
  for (unsigned l = (unsigned)textures.size(); l > 0; --l){
    BlitObject* obj = new BlitObject(textures[l-1], depth, offsets[l-1]);
    mBlits.push_back(obj);
  }
}

BlitGroup::BlitGroup(const std::string& texture, const Vec2i& offset, int depth){
  BlitObject* obj = new BlitObject(texture, depth, offset);
  mBlits.push_back(obj);
}

BlitGroup::~BlitGroup(){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    delete mBlits[i];
  }
}

void BlitGroup::render(Vec2i pos){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    mBlits[i]->render(pos);
  }
}

Object2D::Object2D(int state, Vec2i pos) : mState(state), mPos(pos){

}

Object2D::~Object2D(){
  for (unsigned i = 0; i < blits.size(); ++i){
    for (unsigned j = 0; j < blits[i].size(); ++j){
      delete blits[i][j];
    }
  }
}

void Object2D::render(){
  if (mState <= 0 || blits[mState-1].empty())
    return;
  blits[mState-1][0]->render(mPos);
}
