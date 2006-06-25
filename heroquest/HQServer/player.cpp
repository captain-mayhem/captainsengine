/***************************************************************************
                          player.cpp  -  description
                             -------------------
    begin                : Di Feb 15 2005
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

#include <iostream>
#include <fstream>
#include "player.h"
#include "admin.h"
#include "../HQClient/common.h"

using std::ifstream;

Player::Player(ServerSocket* ss){
  sock = ss;
  name = "guest"+toStr((int)ss);
  lastCreat_ = NULL;
}

Player::Player(const Player& p){
  name = string(p.name);
  pwd = string(p.pwd);
  sock = p.sock;
  status = p.status;
  lastCreat_ = p.lastCreat_;
}

Player::~Player(){
}

const Player& Player::operator << ( const string& s){
  ServerSocket* ss = this->getSock();
  *ss << s;
  return *this;
}
  
const Player& Player::operator >> (string& s){
  ServerSocket* ss = this->getSock();
  *ss >> s;
  return *this;
}

void Player::set_identity(string& name, const string& pwd){
  string tmp = this->name;
  this->name = name;
  this->pwd = pwd;
  globl.alter_idx(tmp, name);
}

//bool Player::isZargon() const{
  //implement only if needed
//  return false;
//}
