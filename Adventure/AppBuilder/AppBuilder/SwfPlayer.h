#ifndef ADV_SWF_PLAYER_H
#define ADV_SWF_PLAYER_H

#include "Sound.h"

namespace swf{
  class SwfReader;
}

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
  DataBuffer mData;
  swf::SwfReader* mReader;

};

#endif
