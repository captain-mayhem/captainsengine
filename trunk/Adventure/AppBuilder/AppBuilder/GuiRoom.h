#ifndef GUI_ROOM_H
#define GUI_ROOM_H

namespace adv{

class GuiRoom : public RoomObject{
public:
  GuiRoom() : RoomObject(1, Vec2i(), Engine::instance()->getResolution(), "#gui", Vec2i(), false), mGUIState(INVISIBLE), mAllowAdd(true) {}
  virtual ~GuiRoom(){
    for (std::vector<Object2D*>::iterator iter = mOldObjects.begin(); iter != mOldObjects.end(); ++iter){
      delete (*iter);
    }
    mOldObjects.clear();
  }

  void scriptUpdate(){
    for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
      while(!(*iter)->getScript()->getEvents().empty())
        Engine::instance()->getInterpreter()->executeImmediately((*iter)->getScript());
    }
  }

  void addUIElement(Object2D* elem, int offset){
    if (!mAllowAdd){
      delete elem;
      return;
    }
    if (mGUIState == CLEARED){ //normal operation, allow the clear
      mGUIState = INVISIBLE;
      clear();
    }
    mGUIState = ADDED;

    if (offset != 0){
      for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
        Vec2i pos = (*iter)->getPosition();
        pos.y += offset;
        (*iter)->setPosition(pos);
      }
    }
    mObjects.push_back(elem);
  }

  bool clear(){
    if (mGUIState == FADING_OUT) //no clear during fade out
      return false;

    if (mGUIState == ADDED){ //normal operation, delayed clear
      mGUIState = CLEARED;
      return false;
    }
    else if (mGUIState == CLEARED){
      if (getFadeout() > 0)
        mGUIState = FADING_OUT;
      else
        mGUIState = INVISIBLE;
    }

    if (mGUIState == FADING_OUT){ //signal clear animatin
      return true;
    }

    //do the real clearing
    for (std::vector<Object2D*>::iterator iter = mOldObjects.begin(); iter != mOldObjects.end(); ++iter){
      delete (*iter);
    }
    mOldObjects.clear();

    mOldObjects = mObjects;

    mObjects.clear();
    return false;
  }

  virtual void setFadeout(int time){
    //fade out finished
    if (mGUIState == FADING_OUT){
      mGUIState = INVISIBLE;
      clear();
    }
    RoomObject::setFadeout(time);
  }

  virtual void render(){
    //render the old objects to avoid flickering when the current textscene is built up
    for (std::vector<Object2D*>::iterator iter = mOldObjects.begin(); iter != mOldObjects.end(); ++iter){
      (*iter)->render();
    }
  }

  virtual void setOpacity(unsigned char opacity){
    //apply opacity to current and old objects
    RoomObject::setOpacity(opacity);
    for (std::vector<Object2D*>::iterator iter = mOldObjects.begin(); iter != mOldObjects.end(); ++iter){
      Color c = (*iter)->getLightingColor();
      c.a = opacity;
      (*iter)->setLightingColor(c);
    }
  }

  void setAllowAdd(bool doit) {
    mAllowAdd = doit;
    if (mAllowAdd)
      return;
    for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
      delete (*iter);
    }
    mObjects.clear();
  }

  virtual Object2D* getObjectAt(const Vec2i& pos){
    if (mGUIState == FADING_OUT || mGUIState == INVISIBLE)
      return NULL;
    return RoomObject::getObjectAt(pos);
  }
protected:
  enum State{
    ADDED,
    CLEARED,
    FADING_OUT,
    INVISIBLE,
  };

  State mGUIState;
  std::vector<Object2D*> mOldObjects;
  bool mAllowAdd;
};

}

#endif
