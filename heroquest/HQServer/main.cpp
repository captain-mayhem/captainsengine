/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Fr Feb 11 13:34:51 CET 2005
    copyright            : (C) 2005 by Marco Staginski
    email                : captain@captain-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <iostream>
#include <string>
#ifdef WIN32
#include <direct.h>
#ifdef _DEBUG
#include <windows.h>
#include <crtdbg.h>
#endif
#endif
//#include "SDL.h"
//#include "SDL_thread.h"
#include "system/engine.h"
#include "system/thread.h"
#include "serversocket.h"
#include "../HQClient/socketexception.h"
#include "../HQClient/opcodes.h"
#include "../HQClient/world.h"
#include "../HQClient/script.h"
#include "message.h"
#include "admin.h"
#include "gamestate.h"

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

const char SEPARATOR = '\004';

const string getCwd(){
  char cwd[2048];
#ifdef WIN32
  _getcwd(cwd, 2048);
#else
  getcwd(cwd, 2048);
#endif
  return string(cwd);
}

Admin globl;
Message msg;
World wrld;
GameState game;
CGE::Mutex mutex;
Script scr;
void running(void* s);
string path;
string home;

void engineMain(int argc, char** argv) {
#ifdef WIN32
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC 1
//int flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
//flag |= _CRTDBG_CHECK_ALWAYS_DF;
//_CrtSetDbgFlag(flag);
#endif
#endif
  short port;

  if (argc < 2){
    //cerr << "Usage: HQServer-Linux <port>" << endl;
    //exit(EXIT_FAILURE);
    port = 28405;
  }
  else
	  port = atoi(argv[1]);

  //calculate correct directory;
  path = string(argv[0]);
  int t = path.find_last_of('/');
  if (t != -1 && path[0] != '/'){
    path.erase(t);
    globl.setPath(getCwd()+"/"+path+"/");
  }
  else{
    globl.setPath(getCwd()+"/");
  }
  path = globl.getPath();
  home = path;

  //mutex = SDL_CreateMutex();

  //now init global data and settings
  game.init();
  globl.init();

  try{
    ServerSocket server(port);
    //take on connection
    while (true){
      ServerSocket* new_sock = new ServerSocket();
      server.accept(*new_sock);

      //CGE::Thread* th = SDL_CreateThread(running, (void*)new_sock);
      CGE::Thread* th = new CGE::Thread();
      th->create(running, (void*)new_sock);
      
      globl.add(new_sock,th);
    }
  }
  catch(SocketException& e){
	  cout << e.description() << endl;
  }
  //return 0;
}

void running(void* s){
  ServerSocket *ss = (ServerSocket*) s;

  // get time
  time_t times;
  if (time(&times) == (time_t) -1) {
    perror("Cannot get time:");
  }

  struct tm * broken_times;
  broken_times = localtime(&times);

  // write time formatted to string
  char output[18];
  if (strftime(output, 18, "%d.%m.%Y %H:%M", broken_times) == 0) {
        printf("Error formating time.");
  }
  output[16] = '\n';
  output[17] = '\0';
  string welcome("\nHero Quest Server v"+string(VERSTRING)+"\nServer Time: ");
  welcome += output;
  *ss << welcome;

  while(1){
    string receive;
    //int cmd;

    try{
      *ss >> receive;
    }
    catch(SocketException& e){
      cout << e.description() << endl;
      globl.end(ss);
    }
    int pos = 0;
    unsigned found;
    while ((found = receive.find(SEPARATOR,pos)) != string::npos){
      string tmp = receive.substr(pos,found-pos);
      pos = found+1;
      msg.process(ss, tmp);
    }
  }
}
