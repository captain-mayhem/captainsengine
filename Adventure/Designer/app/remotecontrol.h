#ifndef SD_REMOTECONTROL_H
#define SD_REMOTECONTROL_H

#include <memory>
#include <string>

#include <model/project.h>

namespace CGE{
class Socket;
}

namespace sd{

//! the connection to a running game's command receiver (the Adventure's
//! CmdReceiver, port 28406): it says "cge <width> <height>" and takes lines,
//! as from the C# RemoteControl plugin
class RemoteControl{
public:
  RemoteControl();
  ~RemoteControl();
  //! false and why if there is no game
  bool connect(const std::string& host, int port, std::string& error);
  void disconnect();
  bool connected() const {return socket_ != nullptr;}
  //! the game's size in its pixels
  Vec2i size() const {return size_;}
  //! sends a line; the game cuts its last character, the C# client's carriage return
  void send(const std::string& line);
private:
  std::unique_ptr<CGE::Socket> socket_;
  Vec2i size_;
};

}

#endif
