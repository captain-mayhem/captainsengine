#ifndef CMD_RECEIVER
#define CMD_RECEIVER

#include <system/thread.h>
#include <system/socket.h>

#include <queue>

class CommandReceiver{
public:
  CommandReceiver();
  ~CommandReceiver();
  void start();
  void requestStop() {mStopRequested = true;}
  void stop();
  void processCommands();
protected:
  enum CommandType{
    MOUSE_MOVE,
  };
  struct Command{
    CommandType type;
    int x;
    int y;
  };
  static void start_routine(void* data);
  void threadLoop();
  void parseCommand(const std::string& cmd);
  CGE::Thread mThread;
  bool mStopRequested;
  CGE::Socket mSocket;
  CGE::Socket mConnSocket;
  std::queue<Command> mQueue;
};

#endif
