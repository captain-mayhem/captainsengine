#include "CmdReceiver.h"

#include <cstdio>

#include "Engine.h"

CommandReceiver::CommandReceiver() : mStopRequested(false){
}

CommandReceiver::~CommandReceiver(){
}

void CommandReceiver::start(){
  mThread.create(start_routine, this);
}

void CommandReceiver::stop(){
  requestStop();
  mThread.join();
}

void CommandReceiver::start_routine(void* data){
  CommandReceiver* that = (CommandReceiver*)data;
  that->threadLoop();
}

void CommandReceiver::threadLoop(){
  mSocket.create();
  mSocket.bind(28406);
  mSocket.listen();
  while(!mStopRequested){
    mSocket.accept(mConnSocket);
    //mSocket.set_non_blocking(false);
    std::string msg;
    std::string cmd;
    while(!mStopRequested){
      mConnSocket.recv(msg);
      if (msg.length() > 0){
        cmd += msg;
        if (cmd[cmd.length()-1] == '\n'){
          parseCommand(cmd);
          cmd.clear();
        }
      }
      CGE::Thread::sleep(10);
    }
    //mConnSocket.close();
  }
}

void CommandReceiver::parseCommand(const std::string& cmd){
  char cmdid[4];
  cmdid[2] = '\0';
  int x; int y;
  sscanf(cmd.c_str(), "%s %i %i", cmdid, &x, &y);
  if (strcmp(cmdid, "mp") == 0){
    Command c;
    c.type = MOUSE_MOVE;
    c.x = x;
    c.y = y;
    mQueue.push(c);
  }
  else if (strcmp(cmdid, "mc") == 0){
    Command c;
    c.type = MOUSE_CLICK;
    c.x = x;
    c.y = y;
    mQueue.push(c);
  }
  else if (strcmp(cmdid, "mr") == 0){
    Command c;
    c.type = MOUSE_RIGHTCLICK;
    c.x = x;
    c.y = y;
    mQueue.push(c);
  }
  return;
}

void CommandReceiver::processCommands(){
  while (!mQueue.empty()){
    Command c = mQueue.front();
    mQueue.pop();
    switch(c.type){
      case MOUSE_MOVE:
        Engine::instance()->setCursorPos(Vec2i(c.x, c.y));
        break;
      case MOUSE_CLICK:
        Engine::instance()->leftClick(Vec2i(c.x, c.y));
        break;
      case MOUSE_RIGHTCLICK:
        Engine::instance()->rightClick(Vec2i(c.x, c.y));
        break;
    }
  }
}
