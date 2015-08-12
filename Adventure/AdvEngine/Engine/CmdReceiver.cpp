#include "CmdReceiver.h"

#include <cstdio>

#include "Engine.h"

using namespace adv;

CommandReceiver::CommandReceiver() : mStopRequested(false), mMultiline(false){
}

CommandReceiver::~CommandReceiver(){
  while (!mQueue.empty()){
    Command c = mQueue.front();
    if (c.type == SCRIPT)
      free(c.str);
    mQueue.pop();
  }
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
  mSocket.set_non_blocking(true);
  while(!mStopRequested){
    if (!mSocket.accept(mConnSocket)){
      CGE::Thread::sleep(100);
      continue;
    }
    mConnSocket.set_non_blocking(true);
    std::string msg;
    msg = "cge "+toStr(Engine::instance()->getResolution().x+32)+" "+toStr(Engine::instance()->getResolution().y+32)+"\n";
    mConnSocket.send(msg);
    msg.clear();
    std::string cmd;
    while(!mStopRequested){
      if (mConnSocket.recv(msg) < 0)
        break;
      if (msg.length() > 0){
        cmd += msg;
        size_t pos = cmd.find('\n');
        while (pos != cmd.npos){
          std::string begin = cmd.substr(0, pos-1);
          cmd = cmd.substr(pos+1);
          pos = cmd.find('\n');
          parseCommand(begin);
        }
        cmd.clear();
      }
      CGE::Thread::sleep(20);
    }
    //mConnSocket.close();
  }
}

void CommandReceiver::parseCommand(const std::string& cmd){
  char cmdid[4];
  cmdid[3] = '\0';
  int x; int y;
  if (mMultiline){
    if (cmd == "***"){
      mMultiline = false;
      Command c;
      c.type = SCRIPT;
      c.str = strdup(mMsg.c_str());
      mQueue.push(c);
      mMsg.clear();
    }
    else{
      mMsg += cmd+"\n";
    }
  }
  else if (cmd[0] == 'm'){
    sscanf(cmd.c_str(), "%s %i %i", cmdid, &x, &y);
    if (strcmp(cmdid, "mps") == 0){
      Command c;
      c.type = MOUSE_MOVE;
      c.x = x;
      c.y = y;
      mQueue.push(c);
    }
    else if (strcmp(cmdid, "mcl") == 0){
      Command c;
      c.type = MOUSE_CLICK;
      c.x = x;
      c.y = y;
      mQueue.push(c);
    }
    else if (strcmp(cmdid, "mcr") == 0){
      Command c;
      c.type = MOUSE_RIGHTCLICK;
      c.x = x;
      c.y = y;
      mQueue.push(c);
    }
  }
  else if (cmd[0] == 's'){
    sscanf(cmd.c_str(), "%s\n", cmdid);
    if (strcmp(cmdid, "scr") == 0){
      mMultiline = true;
    }
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
      case SCRIPT:{
        ExecutionContext* ctx = Engine::instance()->getInterpreter()->parseProgram(c.str);
        if (ctx){
          Engine::instance()->getInterpreter()->execute(ctx, true);
        }
        free(c.str);
        }
    }
  }
}
