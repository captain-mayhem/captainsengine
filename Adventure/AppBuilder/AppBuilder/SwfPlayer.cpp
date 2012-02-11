#include "SwfPlayer.h"

#include <io/MemReader.h>
#include <io/GZipReader.h>
#include <io/Tracing.h>

#include "BlitObjects.h"
#include "Engine.h"

using namespace std;

TR_CHANNEL(ADV_SWF);

namespace swf{
  struct Rect{
    int32 xMin;
    int32 xMax;
    int32 yMin;
    int32 yMax;
  };

  struct RGB{
    uint8 red;
    uint8 green;
    uint8 blue;
  };

  struct RGBA{
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 alpha;
  };

  struct Matrix{
    Matrix() : scaleX(1.0f), scaleY(1.0f), rotateSkew0(0), rotateSkew1(0), translateX(0), translateY(0) {}
    float scaleX;
    float scaleY;
    float rotateSkew0;
    float rotateSkew1;
    float translateX;
    float translateY;
  };

  struct ColorTransformAlpha{
    RGBA mult;
    RGBA add;
  };

  enum Opcode{
    End=0,
    ShowFrame=1,
    SetBackgroundColor=9,
    PlaceObject2=26,
    DefineBitsLossless2=36,
    SoundStreamHead2=45,
  };

  class SwfReader : public CGE::MemReader {
  public:
    SwfReader(void* address, unsigned int length) : CGE::MemReader(address, length), mBitsLeft(0){
    }
    void resetBits(){
      mBitsLeft = 0;
    }
    uint32 readUBits(uint8 howmany){
      uint32 ret = 0;
      while (howmany > 0){
        if (mBitsLeft == 0){
          mCurrByte = readUChar();
          mBitsLeft = 8;
        }
        uint8 bitmask = (1 << min(mBitsLeft, howmany))-1;
        int rightshift = ((int)mBitsLeft-howmany);
        if (rightshift < 0){
          howmany -= mBitsLeft;
          mBitsLeft = 0;
          rightshift = 0;
        }
        else{
          mBitsLeft -= howmany;
          howmany = 0;
        }
        ret |= ((mCurrByte >> rightshift) & bitmask) << howmany;
      }
      return ret;
    }
    int32 readBits(uint8 howmany){
      int32 ret = (int32)readUBits(howmany);
      if (ret & (1 << howmany)){
        //correct the sign
        DebugBreak();
      }
      return ret;
    }
    float readFixedBits(uint8 howmany){
      int32 val = readBits(howmany);
      float ret = (float)(val >> 8);
      ret += (val - ((val >> 8) << 8))/(float(1<<8));
      return ret;
    }
    Rect readRect(){
      Rect ret;
      uint8 numBits = readUBits(5);
      ret.xMin = readBits(numBits)/20;
      ret.xMax = readBits(numBits)/20;
      ret.yMin = readBits(numBits)/20;
      ret.yMax = readBits(numBits)/20;
      resetBits();
      return ret;
    }
    RGB readRGB(){
      RGB ret;
      ret.red = readUChar();
      ret.green = readUChar();
      ret.blue = readUChar();
      return ret;
    }
    void readMatrix(Matrix& mat){
      uint8 hasScale = readUBits(1);
      if (hasScale == 1){
        uint8 numBits = readUBits(5);
        mat.scaleX = readFixedBits(numBits);
        mat.scaleY = readFixedBits(numBits);
      }
      uint8 hasRotate = readUBits(1);
      if (hasRotate == 1){
        uint8 numBits = readUBits(5);
        mat.rotateSkew0 = readFixedBits(numBits);
        mat.rotateSkew1 = readFixedBits(numBits);
      }
      uint8 numBits = readUBits(5);
      mat.translateX = readBits(numBits)/20.0f;
      mat.translateY = readBits(numBits)/20.0f;
      resetBits();
    }
    void read(ColorTransformAlpha& trans){
      uint8 hasAdd = readUBits(1);
      uint8 hasMult = readUBits(1);
      uint8 numBits = readUBits(4);
      if (hasMult == 1){
        trans.mult.red = readBits(numBits);
        trans.mult.green = readBits(numBits);
        trans.mult.blue = readBits(numBits);
        trans.mult.alpha = readBits(numBits);
      }
      if (hasAdd == 1){
        trans.add.red = readBits(numBits);
        trans.add.green = readBits(numBits);
        trans.add.blue = readBits(numBits);
        trans.add.alpha = readBits(numBits);
      }
      resetBits();
    }
    float readFixed(){
      int16 val = readShort();
      float ret = (float)(val >> 8);
      ret += (val - ((val >> 8) << 8))/(float(1<<8));
      return ret;
    }
  protected:
    uint8 mCurrByte;
    uint8 mBitsLeft;
  };

  class Character{
  public:
    Character(const CGE::Image& img){
      glGenTextures(1, &mTexture);
      glBindTexture(GL_TEXTURE_2D, mTexture);
      if (img.getNumChannels() != 4)
        DebugBreak();
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getWidth(), img.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
    }
    ~Character(){
      glDeleteTextures(1, &mTexture);
    }
  protected:
    GLuint mTexture;
  };

  class Displayable{
  public:
    Displayable(Character* chr) : mChar(chr){
    }
    Matrix& getMatrix() {return mMatrix;}
  protected:
    Character* mChar;
    Matrix mMatrix;
  };
}

SwfPlayer::SwfPlayer(const std::string& name, const DataBuffer& db) : mData(db), mReader(NULL), mStop(true), mClock(0){
  mReader = new swf::SwfReader(mData.data, mData.length);
  parseFile();
}

SwfPlayer::~SwfPlayer(){
  delete mReader;
  delete mLayer;
  for (unsigned i = 0; i < mDisplayList.size(); ++i){
    delete mDisplayList[i];
  }
  for (std::map<uint16,swf::Character*>::iterator iter = mDictionary.begin(); iter != mDictionary.end(); ++iter){
    delete iter->second;
  }
}

void SwfPlayer::play(bool looping){
  mStop = false;
}

void SwfPlayer::stop(){
  mStop = true;
}

bool SwfPlayer::update(unsigned time){
  mClock += time;
  if (mClock >= mFrameTime*mFrameNum){
    processTags();
  }
  if (mLayer && !mStop){
    Engine::instance()->beginRendering();
    mLayer->render(mRenderPos, mScale, Vec2i());
    Engine::instance()->endRendering();
  }
  return !mStop;
}

void SwfPlayer::initLayer(int x, int y, int width, int height){
  mRenderPos.x = x;
  mRenderPos.y = y;
  mScale.x = width/(float)mSize.x;
  mScale.y = height/(float)mSize.y;
  mLayer = new RenderableBlitObject(mSize.x, mSize.y, DEPTH_VIDEO_LAYER);
}

void SwfPlayer::setSuspensionScript(ExecutionContext* ctx){
}

bool SwfPlayer::parseFile(){
  unsigned char signature[3];
  mReader->readBytes(signature, 3);
  if (signature[0] != 'F'){
    if (signature[0] == 'C')
      DebugBreak(); //compressed stream
    return false;
  }
  if (signature[1] != 'W' || signature[2] != 'S')
    return false;
  uint8 version = mReader->readUChar();
  uint32 length = mReader->readUInt();
  swf::Rect rect = mReader->readRect();
  mSize.x = rect.xMax-rect.xMin;
  mSize.y = rect.yMax-rect.yMin;
  float framerate = mReader->readFixed();
  mFrameTime = (unsigned)(1000/framerate);
  uint16 numframes = mReader->readUShort();
  mFrameNum = 0;
  return true;
}

bool SwfPlayer::processTags(){
  TR_USE(ADV_SWF);
  bool stop = false;
  while(!stop){
    uint16 tagandlen = mReader->readUShort();
    swf::Opcode tag = (swf::Opcode)((tagandlen >> 6) & ((1 << 10)-1));
    int32 length = tagandlen & ((1 << 6)-1);
    if (length == 0x3f)
      length = mReader->readInt();
    switch(tag){
      case swf::End:
        mStop = true;
        stop = true;
        break;
      case swf::ShowFrame:
        render();
        stop = true;
        break;
      case swf::SetBackgroundColor:{
        swf::RGB color = mReader->readRGB();
        mClearColor[0] = color.red/255.0f;
        mClearColor[1] = color.green/255.0f;
        mClearColor[2] = color.blue/255.0f;
        break;
      }
      case swf::PlaceObject2:
      {
        uint8 flags = mReader->readUChar();
        uint16 depth = mReader->readUShort();
        if (flags & 0x2){ //has character
          uint16 character = mReader->readUShort();
          swf::Character* chr = getCharacter(character);
          if (chr == NULL){
            TR_WARN("trying to place unknown character %i", character);
          }
          else{
            swf::Displayable* disp = new swf::Displayable(chr);
            setDisplayable(disp, depth);
          }
        }
        swf::Displayable* disp = getDisplayable(depth);
        if (disp == NULL){
          TR_WARN("trying to modify unknown displayable %i", depth);
        }
        if (flags & 0x4){ //has matrix
          if (disp){
            mReader->readMatrix(disp->getMatrix());
          }
          else{
            swf::Matrix mat;
            mReader->readMatrix(mat);
          }
        }
        if (flags & 0x8){ //has color transform
          swf::ColorTransformAlpha cta;
          mReader->read(cta);
        }
        if (flags & 0x10){ //has ratio
          DebugBreak();
        }
        if (flags & 0x20){ //has name
          DebugBreak();
        }
        if (flags & 0x40){ //has clip depth
        }
        if (flags & 0x80){ //has clip actions
        }
        break;
      }
      case swf::DefineBitsLossless2:{
        uint16 charid = mReader->readUShort();
        uint8 format = mReader->readUChar();
        uint16 width = mReader->readUShort();
        uint16 height = mReader->readUShort();
        if (format == 3){
          uint16 colortablesize = ((uint16)mReader->readUChar())+1;
          uint16 rowsize = ((width+3) >> 2) << 2;
          uint32 datasize = colortablesize*4+rowsize*height;
          CGE::GZipReader bmprdr(mReader->getCurrentData(), length-8, datasize);
          mReader->skip(length-8);
          uint8* colortable = new uint8[colortablesize*4];
          bmprdr.readBytes(colortable, colortablesize*4);
          uint8* pixeldata = new uint8[width*height];
          for (unsigned i = 0; i < height; ++i){
            bmprdr.readBytes(pixeldata+i*width, width);
            bmprdr.skip(rowsize-width);
          }
          CGE::Image img(4, width, height, colortable, pixeldata);
          delete [] pixeldata;
          delete [] colortable;
          swf::Character* chr = new swf::Character(img);
          insertCharacter(charid, chr);
        }
        else
          DebugBreak();
        break;                              
      }
      default:
        TR_WARN("unhandled opcode %i data length %i", tag, length);
        mReader->skip(length);
        break;
    }
  }
  return !mStop;
}

void SwfPlayer::insertCharacter(uint16 id, swf::Character* chr){
  mDictionary[id] = chr;
}

swf::Character* SwfPlayer::getCharacter(uint16 id){
  std::map<uint16,swf::Character*>::iterator iter = mDictionary.find(id);
  if (iter == mDictionary.end())
    return NULL;
  return iter->second;
}

void SwfPlayer::setDisplayable(swf::Displayable* disp, uint16 depth){
  while(depth >= mDisplayList.size()){
    mDisplayList.push_back(NULL);
  }
  if (mDisplayList[depth] != NULL)
    delete mDisplayList[depth];
  mDisplayList[depth] = disp;
}

swf::Displayable* SwfPlayer::getDisplayable(uint16 depth){
  if (depth >= mDisplayList.size())
    return NULL;
  return mDisplayList[depth];
}

void SwfPlayer::render(){
  mLayer->bind();
  glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mLayer->unbind();
  ++mFrameNum;
}

