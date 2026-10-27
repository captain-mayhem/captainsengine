#ifndef ADV_STREAM_VIDEO_PLAYER_H
#define ADV_STREAM_VIDEO_PLAYER_H

#include <memory>

#include <system/types.h>

#include "SoundPlayer.h"

namespace CGE{
  class Video;
}

namespace adv{

class BlitObject;

//! A video of playavi/playvideo: MPEG-1 or Theora through the engine's CGE::Video
/*! The pictures go into a layer above the game (DEPTH_VIDEO_LAYER), at the
    position and size the script gives; the sound plays through the
    engine's audio. A suspended script goes on when the video ends, or when
    it is skipped (escape in a cutscene). */
class StreamVideoPlayer : public VideoPlayer{
public:
  StreamVideoPlayer(const std::string& name, std::unique_ptr<CGE::Video> video);
  virtual ~StreamVideoPlayer();
  void play(bool looping);
  void stop();
  virtual bool update(unsigned time);
  virtual void render(unsigned time);
  void initLayer(int x, int y, int width, int height);
  void setSuspensionScript(ExecutionContext* ctx);
  virtual void realize() {}
protected:
  std::string mName;
  std::unique_ptr<CGE::Video> mVideo;
  BlitObject* mLayer;
  //the picture in the layer
  unsigned mFrameShown;
  Vec2i mRenderPos;
  Vec2f mScale;
  ExecutionContext* mSuspensionScript;
  bool mLooping;
  bool mStop;
};

}

#endif
