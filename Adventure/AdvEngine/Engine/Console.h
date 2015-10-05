#ifndef ADV_CONSOLE_H
#define ADV_CONSOLE_H

#include <deque>
#include "EngineObjects.h"

TR_CHANNEL_EXT(ADV_Console);

namespace adv{

class Console : public RoomObject, CGE::TraceOutputter{
public:
  Console();
  ~Console();
  virtual bool init();
  virtual void trace(unsigned channel, int level, const char* function, const char* message);
  void show(bool doit);
  bool isShown() { return mActive; }
  void input(const char* str);
  void output(const char* str);
  void print(const char* fmt, ...);
  void historyUp();
  void historyDown();
  void scrollUp();
  void scrollDown();
  static CGE::Image* getBackground();
  void render();
  virtual int getDepth();
protected:
  bool mActive;
  std::deque<std::string> mOutput;
  std::deque<std::string> mInput;
  int mHistoryPos;
  int mScrollPos;
};

}

#endif