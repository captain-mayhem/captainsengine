#include "Engine.h"

#include <wx/image.h>

#include "AdvDoc.h"

Engine* Engine::mInstance = NULL;

Engine::Engine() : mData(NULL){
  mVerts[0] = 0; mVerts[1] = 1;
  mVerts[2] = 0; mVerts[3] = 0;
  mVerts[4] = 1; mVerts[5] = 1;
  mVerts[6] = 1; mVerts[7] = 0;
}

Engine::~Engine(){
}

void Engine::initGame(){
  //load cursor
  if (!mData)
    return;
  Cursor* cursor = mData->getCursor();
  mCursor = new Object2D();
  mCursor->state = 1;
  mCursor->blits.resize(cursor->size());
  for (unsigned j = 0; j < cursor->size(); ++j){
    mCursor->blits[j].resize((*cursor)[j].frames.size());
    for (unsigned k = 0; k < (*cursor)[j].frames.size(); ++k){
        BlitObject* obj = new BlitObject();
        obj->pos = (*cursor)[j].highlight;
        obj->depth = 20000;
        obj->tex = genTexture((*cursor)[j].frames[k], obj->size, obj->scale);
        mCursor->blits[j][k].push_back(obj);
    }
  }
  insertToBlit(mCursor->blits[mCursor->state-1][0]);
}

void Engine::exitGame(){
  //for (std::list<BlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){
  //  delete *iter;
  //}
  for (unsigned i = 0; i < mObjects.size(); ++i){
    delete mObjects[i];
  }
  mObjects.clear();
  mBlitQueue.clear();
  delete mCursor;
}

void Engine::render(){
  glVertexPointer(2, GL_SHORT, 0, mVerts);
  glTexCoordPointer(2, GL_SHORT, 0, mVerts);
  for (std::list<BlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){ 
    blit((*iter)->pos.x, (*iter)->pos.y, (*iter)->size.x, (*iter)->size.y, (*iter)->scale.x, (*iter)->scale.y, (*iter)->tex);
  }
}

bool Engine::loadRoom(std::string name){
  if (!mData)
    return false;
  Room* room = mData->getRoom(name);
  if (!room)
    return false;
  Object2D* roomobj = new Object2D();
  BlitObject* obj = new BlitObject();
  obj->depth = -1;
  obj->pos = Vec2i(0,0);
  obj->tex = genTexture(room->background, obj->size, obj->scale);
  roomobj->blits.resize(1);
  roomobj->blits[0].resize(1);
  roomobj->blits[0][0].push_back(obj);
  insertToBlit(obj);
  mObjects.push_back(roomobj);
  for (unsigned i = 0; i < room->objects.size(); ++i){
    Object2D* object = new Object2D();
    object->state = room->objects[i].state;
    Object* o = mData->getObject(room->objects[i].object);
    //calculate render depth
    int depth;
    if (room->objects[i].layer == 1)
      depth = 0;
    else if (room->objects[i].layer == 2)
      depth = room->objects[i].wm_depth;
    else
      depth = 10000;
    object->blits.resize(o->states.size());
    for (unsigned j = 0; j < object->blits.size(); ++j){
      object->blits[j].resize(o->states[j].frames.size());
      for (unsigned k = 0; k < o->states[j].frames.size(); ++k){
        for (unsigned l = (unsigned)o->states[j].frames[k].names.size(); l > 0; --l){
          obj = new BlitObject();
          obj->pos = room->objects[i].position+o->states[j].frames[k].offsets[l-1];
          obj->depth = depth;
          obj->tex = genTexture(o->states[j].frames[k].names[l-1], obj->size, obj->scale);
          object->blits[j][k].push_back(obj);
        }
      }
    }
    if (object->state > 0 && !object->blits[object->state-1].empty())
      insertToBlit(object->blits[object->state-1][0]);
    mObjects.push_back(object);
  }
  return true;
}

//TODO move into utility
unsigned roundToPowerOf2(unsigned x){
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

GLuint Engine::genTexture(std::string name, Vec2i& size, Vec2f& texscale){
  wxImage image(mData->getFilename(IMAGE,name).GetFullPath());
  size.x = image.GetWidth();
  size.y = image.GetHeight();
  Vec2i pow2(roundToPowerOf2(size.x), roundToPowerOf2(size.y));
  texscale.x = ((float)size.x)/pow2.x;
  texscale.y = ((float)size.y)/pow2.y;
  unsigned totalsize = size.x*size.y;
  unsigned char* rgb = image.GetData();
  unsigned char* alpha = image.GetAlpha();
  GLubyte* buffer = new GLubyte[totalsize*4];
  for (unsigned i = 0; i < totalsize; ++i){
    buffer[4*i] = rgb[3*i];
    buffer[4*i+1] = rgb[3*i+1];
    buffer[4*i+2] = rgb[3*i+2];
    if (alpha)
      buffer[4*i+3] = alpha[i];
    else{
      if (rgb[3*i] == 0xFF && rgb[3*i+1] == 0x0 && rgb[3*i+2] == 0xFF){
        buffer[4*i+3] = 0;
      }
      else{
        buffer[4*i+3] = 255;
      }
    }
  }
  GLuint tex;
  glGenTextures(1,&tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2.x, pow2.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  delete buffer;
  return tex;
}

void Engine::blit(int x, int y, int width, int height, float xscale, float yscale, GLuint tex){
  glPushMatrix();
  glTranslatef(x,y,0.0f);
  glScalef(width,height,1.0f);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glScalef(xscale, yscale, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glBindTexture(GL_TEXTURE_2D, tex);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glPopMatrix();
}

void Engine::insertToBlit(BlitObject* obj){
  mBlitQueue.push_front(obj);
  resortBlit(obj);
}

void Engine::insertToBlit(BlitGroup& obj){
  for (unsigned i = 0; i < obj.size(); ++i){
    insertToBlit(obj[i]);
  }
}

void Engine::resortBlit(BlitObject* obj){
  //search obj
  std::list<BlitObject*>::iterator objectiter;
  for (std::list<BlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){
    if (*iter == obj){
      objectiter = iter;
      break;
    }
  }
  //move object
  for (std::list<BlitObject*>::iterator iter = objectiter++; objectiter != mBlitQueue.end(); ++objectiter){
    if ((*objectiter)->depth < (*iter)->depth){
      std::swap(*objectiter, *iter);
    }
    else
      break;
  }
}

void Engine::setCursorPos(Vec2i pos){
  mCursorPos = pos;
  mCursor->blits[mCursor->state-1][0][0]->pos = pos;
}
