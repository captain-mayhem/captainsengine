#include "SwfPlayer.h"

#include <io/MemReader.h>
#include <io/Tracing.h>

#include "BlitObjects.h"

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

  enum Opcode{
    End=0,
    ShowFrame=1,
    SetBackgroundColor=9,
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
        uint8 bitmask = (1 << mBitsLeft)-1;
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
      return (int32)readUBits(howmany);
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
    float readFixed(){
      uint16 val = readUShort();
      float ret = (float)(val >> 8);
      ret += (val - ((val >> 8) << 8))/(float(1<<8));
      return ret;
    }
  protected:
    uint8 mCurrByte;
    uint8 mBitsLeft;
  };
}

SwfPlayer::SwfPlayer(const std::string& name, const DataBuffer& db) : mData(db), mReader(NULL), mStop(true), mClock(0){
  mReader = new swf::SwfReader(mData.data, mData.length);
  parseFile();
}

SwfPlayer::~SwfPlayer(){
  delete mReader;
  delete mLayer;
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
  if (mLayer && !mStop)
    mLayer->render(mRenderPos, mScale, Vec2i());
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
      default:
        TR_WARN("unhandled opcode %i data length %i", tag, length);
        mReader->skip(length);
        break;
    }
  }
  return !mStop;
}

void SwfPlayer::render(){
  mLayer->bind();
  glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mLayer->unbind();
  ++mFrameNum;
}

