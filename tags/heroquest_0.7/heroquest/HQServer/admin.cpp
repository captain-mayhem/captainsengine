/***************************************************************************
                          admin.cpp  -  description
                             -------------------
    begin                : Sa Feb 12 2005
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
 
#include "time.h"
#ifndef WIN32
  #include "unistd.h"
#endif
#include <string>
#include <iostream>
#include <fstream>
//#include "SDL.h"
//#include "SDL_thread.h"
#include "system/thread.h"

#include "player.h"
#include "../HQClient/common.h"
#include "../HQClient/opcodes.h"
#include "../HQClient/socketexception.h"
#include "gamestate.h"
#include "admin.h"

using std::string;
using std::endl;
using std::cout;
using std::cerr;
using std::ifstream;

using System::Thread;
using System::Mutex;

// a mutex to lock data for save multithread access
extern System::Mutex mutex;

//Construcor
Admin::Admin() : zargon(NULL) {
  //dummy entry to distinguish "not found" ( = 0 ) in hash and "found first entry";
  ServerSocket* ss = NULL;
  Player dummy(ss);
  players.push_back(dummy);
  Thread* th = NULL;
  threads.push_back(th);
}

//Destructor
Admin::~Admin(){

}

//add a socket
void Admin::add(ServerSocket* ss, Thread* th){
  mutex.lock();
  Player pl(ss);
  //first player that connects gets special status
  if (players.size() == 1){
    pl.setStatus(2);
  }
  else{
    pl.setStatus(1);
  }
  players.push_back(pl);
  threads.push_back(th);
  int idx = players.size()-1;
  plr_idx1[ss] = idx;
  plr_idx2[pl.getName()] = idx;
  mutex.unlock();
}

//end socket connection
void Admin::end(ServerSocket* ss){
  string name;
  ServerSocket* newSs = NULL;
  mutex.lock();
  vector<Player>::iterator it;
  it = players.begin();
  vector<Thread*>::iterator it2;
  it2 = threads.begin();
  Thread* thread = NULL;
  for (++it; it != players.end(); it++){
    it2++;
    //this player has left
    if(it->getSock() == ss){
      unsigned count = 0;
      while(game.isActivePlayer(ss) && players.size() > 2 && count < players.size()){
        newSs = game.getNextPlayer();
	      count++;
      }
      if (globl.getZargon() == ss){
        zargon = NULL;
      }
      name = it->getName();
      plr_idx1[ss] = 0;
      plr_idx2[name] = 0;
      players.erase(it);
      thread = *it2;
      threads.erase(it2);
      break;
    }
  }
  mutex.unlock();
  chat(ss, toStr(CHAT)+ name +" left the server");
  delete ss;
  //quit server if last player left
  if (players.size() == 1){
    //SDL_DestroyMutex(mutex);
    exit(EXIT_SUCCESS);
  }
  if (newSs != NULL){
    *newSs << toStr(CHAT)+ " It's your turn";
    globl.chat(newSs, toStr(CHAT)+" It's "+ globl.getPlayer(newSs)->getName()+ "'s turn.");
  }
  //SDL_KillThread(thread);
  thread->destroy();
}

//chat to someone
void Admin::chat(const ServerSocket* ss, const string message){
  //SDL_mutexP(mutex);
  mutex.lock();
  for (unsigned i = 1; i < players.size(); i++){
    ServerSocket *sock = players[i].getSock();
    if (sock != ss){
      try{
        *sock << message;
      }
      catch(SocketException& e){
        cout << e.description() << endl;
      }
    }
  }
  //SDL_mutexV(mutex);
  mutex.unlock();
}

// broadcast message
void Admin::broadcast(const string message){
  messages.push_back(message);
  //SDL_mutexP(mutex);
  mutex.lock();
  for (unsigned i = 1; i < players.size(); i++){
    ServerSocket *sock = players[i].getSock();
    try{
      *sock << message;
    }
    catch(SocketException& e){
      cout << e.description() << endl;
    }
  }
  //SDL_mutexV(mutex);
  mutex.unlock();
}

// has access?
bool Admin::accessGranted(const string& name, const string& pwd){
  string savedPwd = pwd_lookup[name];
  if (savedPwd == pwd && pwd != "0")
    return true;
  return false;
}

//get the player
Player* Admin::getPlayer(ServerSocket* ss){
  int idx = plr_idx1[ss];
  if (idx == 0)
    return NULL;
  return &players[idx];
}

//get the player
Player* Admin::getPlayer(const string& name){
  int idx = plr_idx2[name];
  if (idx == 0)
    return NULL;
  return &players[idx];
}

//list players
const string Admin::listPlayers(){
  string ret;
  for (unsigned i = 0; i < players.size(); i++){
    string temp = players[i].getName();
    if (temp == "guest0")
      continue;
    ret += temp + " ";
  }
  return ret;
}

// change index when a player_name changed
void Admin::alter_idx(string& old_name, string& new_name){
  int idx = plr_idx2[old_name];
  plr_idx2[old_name] = 0;
  plr_idx2[new_name] = idx;
}

//init
void Admin::init(){
  //readin players.dat
  string name;
  name = path+"players.dat";
  ifstream in(name.c_str());
  if (!(in >> name)){
    cerr << "players.dat not found" << endl;
    exit(EXIT_FAILURE);
  }
  string pwd;
  while(in >> name){
    in >> pwd;
    pwd_lookup[name] = pwd;
  }
  in.close();
}

//update client state
void Admin::update(const ServerSocket* ss){
  if (messages.empty())
    return;
  list<string>::iterator it;
  //SDL_mutexP(mutex);
  mutex.lock();
  for (it = messages.begin(); it != messages.end(); it++){
    wait();
    *ss << *it;
  }
  //SDL_mutexV(mutex);
  mutex.unlock();
}

//set zargon
bool Admin::setZargon(ServerSocket* ss){
  //SDL_mutexP(mutex);
  mutex.lock();
  bool ret = false;
  if (zargon == NULL){
    zargon = ss;
    ret = true;
  }
  //SDL_mutexV(mutex);
  mutex.unlock();
  return ret;
}

//get zargon
ServerSocket* Admin::getZargon() const{
  return zargon;
}

