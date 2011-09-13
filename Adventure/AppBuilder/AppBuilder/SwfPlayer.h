#ifndef ADV_SWF_PLAYER_H
#define ADV_SWF_PLAYER_H

#include "Sound.h"

namespace swf{
  class SwfReader;
}

class RenderableBlitObject;

class SwfPlayer : public VideoPlayer{
public:
  SwfPlayer(const std::string& name, const DataBuffer& db);
  ~SwfPlayer();
  void play(bool looping);
  void stop();
  bool update(unsigned time);
  void initLayer(int x, int y, int width, int height);
  void setSuspensionScript(ExecutionContext* ctx);
protected:
  bool parseFile();
  bool processTags();
  void render();
  DataBuffer mData;
  swf::SwfReader* mReader;
  bool mStop;
  unsigned mClock;
  Vec2i mRenderPos;
  Vec2f mScale;
  RenderableBlitObject* mLayer;

  unsigned mFrameTime;
  unsigned mFrameNum;
  Vec2i mSize;
  float mClearColor[3];
};

#endif
