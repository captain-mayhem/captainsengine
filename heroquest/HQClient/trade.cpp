#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "input/mouse.h"

#include "textureManager.h"
#include "trade.h"
#ifdef _CLIENT_
#include "player.h"
#include "renderer.h"
#endif

using std::cerr;
using Math::Vector2D;
using Input::Mouse;

Trade::Trade(){
  items_.push_back(Item());
#ifdef _CLIENT_
  fnt_ = System::Engine::instance()->getFont();
  //fnt_.setBase(text.getBase());
  //fnt_.setColor(0,1,1);
  invPage_ = 1;
  shopPage_ = 1;
  chosenItem_ = Item();
  visible_ = true;
#endif
}

Trade::Trade(const Trade& i){
  items_ = vector<Item>(i.items_);
  lookup_ = map<string,int>(i.lookup_);
#ifdef _CLIENT_
  fnt_ = i.fnt_;
  invPage_ = i.invPage_;
  shopPage_ = i.shopPage_;
  chosenItem_ = Item(i.chosenItem_);
  visible_ = i.visible_;
#endif
}

void Trade::addItem(Item it){
  int idx = lookup_[it.getName()];
  //no such item stored
  if(idx == 0){
    items_.push_back(it);
    lookup_[it.getName()] = items_.size()-1;
  }
  else{
    for (int i = 0; i < it.getNumber(); i++)
      items_[idx].increase();
  }
}

bool Trade::deleteItem(string name){
  int idx = lookup_[name];
  if (idx != 0){
    //last item deleted
    if (!items_[idx].decrease()){
      items_[idx].reset();
      return false;
    }
    return true;
  }
  return false;
}


// get item from inventory
Item Trade::getItem(const string name){
  int idx = lookup_[name];
  return items_[idx];
}
    
//! writes item data
void Trade::write(ofstream& out)const{
  short length = items_.size()-1;
  out.write((char*)&length, sizeof(length));
  for (int i = 1; i < length+1; i++){
    items_[i].write(out);
  }
}

//! reads item data
void Trade::read(ifstream& in){
  short length;
  in.read((char*)&length, sizeof(length));
  for (int i = 0; i < length; i++){
    Item it;
    it.read(in);
    items_.push_back(it);
    lookup_[it.getName()] = items_.size()-1;
  }
}

string Trade::toString(){
  string ret = "";
  for (unsigned i = 0; i < items_.size(); i++){
    ret += items_[i].toString()+"!";
  }
  return ret;
}

void Trade::fromString(string it){
  int pos = 0;
  unsigned found;
  items_.clear();
  while ((found = it.find('!',pos)) != string::npos){
    string tmp = it.substr(pos,found-pos);
    pos = found+1;
    Item ite;
    ite.fromString(tmp);
    items_.push_back(ite);
    lookup_[ite.getName()] = items_.size()-1;
  }
}

void Trade::render(){
#ifdef _CLIENT_
  if (!visible_)
    return;
  //background
  glDisable(GL_BLEND);
  glColor4f(1.0,1.0,1.0,1.0);
  TextureManager::instance()->otherTex[2]->activate();
  //glBindTexture(GL_TEXTURE_2D, tex.otherTex[1]);
  glBegin(GL_QUADS);
    glTexCoord2f(1, 0);
    glVertex2i(SCREENWIDTH, SCREENHEIGHT);
    glTexCoord2f(0, 0);
    glVertex2i(0, SCREENHEIGHT);
    glTexCoord2f(0, 1);
    glVertex2i(0, 0);
    glTexCoord2f(1, 1);
    glVertex2i(SCREENWIDTH, 0);
  glEnd();
  glEnable(GL_BLEND);

  //get current inventory
  Creature* c = plyr.getCreature();
  Hero* h = dynamic_cast<Hero*>(c);
  if (h){
    //only a hero can go to the shop
    vector<Item> items = c->getInventory()->getItems();
    
    //inventory
    Vector2D pos(50, 485);
    short count = 0;
    fnt_->setColor(0,1,1);
    for (unsigned i = 9*(invPage_-1)+1; i < items.size() && i < 9*invPage_+1; i++){
      string name = items[i].getName();
      if (name.size() > 11)
        name.erase(11);
      int fill = (11 - name.size())/2;
      name.insert(name.begin(), fill, ' ');
      fnt_->glPrint(pos.x, pos.y, name.c_str(), 1);
      fnt_->glPrint(pos.x+28, pos.y-18, toStr(items[i].getNumber()).c_str(), 1);
      pos.x += 148;
      count++;
      if (count == 3){
        pos.x = 50;
        count = 0;
        pos.y -= 193;
      }
    }
  }
  fnt_->glPrint(177, 50, ("<<  Page "+toStr(invPage_)+"  >>").c_str(), 0);
  
  //shop
  Vector2D pos(575, 485);
  short count = 0;
  fnt_->setColor(0,1,1);
  for (unsigned i = 9*(shopPage_-1)+1; i < items_.size() && i < 9*shopPage_+1; i++){
    string name = items_[i].getName();
    if (name.size() > 11)
      name.erase(11);
    int fill = (11 - name.size())/2;
    name.insert(name.begin(), fill, ' ');
    fnt_->glPrint(pos.x, pos.y, name.c_str(), 1);
    fnt_->glPrint(pos.x+28, pos.y-18, items_[i].getAdditional().c_str(), 1);
    pos.x += 148;
    count++;
    if (count == 3){
      pos.x = 575;
      count = 0;
      pos.y -= 193;
    }
  }
  fnt_->glPrint(702, 50, ("<<  Page "+toStr(shopPage_)+"  >>").c_str(), 0);

  //stats
  fnt_->setColor(0,1,1);
  if (h){
    fnt_->glPrint(350, 680, ("Money: "+toStr(h->getMoney())).c_str(), 0);
  }

  //Vector2D click = gl->getMousePos();
  //fnt_->glPrint(600, 460, (toStr(click.x)+"/"+toStr(click.y)).c_str(), 0);

  if (chosenItem_.isValid()){
    fnt_->glPrint(450, 50, chosenItem_.getName().c_str(), 0);
    int price = toInt(chosenItem_.getAdditional());
    fnt_->glPrint(560, 680, ("Price: "+toStr(price)).c_str(), 0);
  }

  //fnt_.render();
#endif
}

void Trade::update(){
#ifdef _CLIENT_
  if (!visible_ || Input::Mouse::instance()->isGuiClick())
    return;
  Vector2D click = Input::Mouse::instance()->getClickPos();
  //forward button inventory
  if (click.x >= 300 && click.x <= 325){
    if (click.y >= 700 && click.y <= 720){
      Mouse::instance()->resetMousePos();
      invPage_++;
    }
  }
  //backward button inventory
  if (click.x >= 180 && click.x <= 200){
    if (click.y >= 700 && click.y <= 720){
      if (invPage_ > 1){
        Mouse::instance()->resetMousePos();
        invPage_--;
      }
    }
  }
  //forward button shop
  if (click.x >= 825 && click.x <= 850){
    if (click.y >= 700 && click.y <= 720){
      Mouse::instance()->resetMousePos();
      shopPage_++;
    }
  }
  //backward button shop
  if (click.x >= 705 && click.x <= 725){
    if (click.y >= 700 && click.y <= 720){
      if (shopPage_ > 1){
        Mouse::instance()->resetMousePos();
        shopPage_--;
      }
    }
  }

  //get current inventory
  Creature* c = plyr.getCreature();
  Hero* h = dynamic_cast<Hero*>(c);
  if (h){
    //only a hero can go to the shop
    vector<Item> items = c->getInventory()->getItems();
    //first item column
    if (click.x >= 65 && click.x <= 155){
      if (click.y >= 125 && click.y <= 280){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+1;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
      else if (click.y >= 315 && click.y <= 475){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+4;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
      else if (click.y >= 510 && click.y <= 670){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+7;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
    }
    //second item column
    if (click.x >= 210 && click.x <= 300){
      if (click.y >= 125 && click.y <= 280){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+2;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
      else if (click.y >= 315 && click.y <= 475){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+5;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
      else if (click.y >= 510 && click.y <= 670){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+8;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
    }
    //third item column
    if (click.x >= 355 && click.x <= 450){
      if (click.y >= 125 && click.y <= 280){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+3;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
      else if (click.y >= 315 && click.y <= 475){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+6;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
      else if (click.y >= 510 && click.y <= 670){
        Mouse::instance()->resetMousePos();
        unsigned idx = 9*(invPage_-1)+9;
        if (idx < items.size())
          chosenItem_ = items[idx];
      }
    }
  }

  //shop
  //first item column
  if (click.x >= 590 && click.x <= 680){
    if (click.y >= 125 && click.y <= 280){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+1;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+4;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+7;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
  }
  //second item column
  if (click.x >= 735 && click.x <= 825){
    if (click.y >= 125 && click.y <= 280){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+2;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+5;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+8;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
  }
  //third item column
  if (click.x >= 880 && click.x <= 975){
    if (click.y >= 125 && click.y <= 280){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+3;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+6;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      Mouse::instance()->resetMousePos();
      unsigned idx = 9*(shopPage_-1)+9;
      if (idx < items_.size())
        chosenItem_ = items_[idx];
    }
  }

#endif
}
