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
  for (std::list<Object2D*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){
    delete *iter;
  }
}

void Engine::render(){
  glVertexPointer(2, GL_SHORT, 0, mVerts);
  glTexCoordPointer(2, GL_SHORT, 0, mVerts);
  for (std::list<Object2D*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){ 
    blit((*iter)->pos.x, (*iter)->pos.y, (*iter)->size.x, (*iter)->size.y, (*iter)->tex);
  }
}

bool Engine::loadRoom(std::string name){
  if (!mData)
    return false;
  Room* room = mData->getRoom(name);
  if (!room)
    return false;
  Object2D* obj = new Object2D();
  obj->depth = -1;
  obj->pos = Vec2i(0,0);
  obj->size = room->size;
  obj->tex = genTexture(room->background);
  insertToBlit(obj);
  for (unsigned i = 0; i < room->objects.size(); ++i){
    obj = new Object2D();
    obj->pos = room->objects[i].position;
    if (room->objects[i].layer == 1)
      obj->depth = 0;
    else if (room->objects[i].layer == 2)
      obj->depth = room->objects[i].wm_depth;
    else
      obj->depth = 10000;
    Object* o = mData->getObject(room->objects[i].object);
    obj->size = o->size;
    obj->tex = genTexture(o->states[0].frames[0].names[0]);
    insertToBlit(obj);
  }
  return true;
}

GLuint Engine::genTexture(std::string name){
  wxImage image(mData->getFilename(IMAGE,name).GetFullPath());
  unsigned size = image.GetWidth()*image.GetHeight();
  unsigned char* rgb = image.GetData();
  unsigned char* alpha = image.GetAlpha();
  GLubyte* buffer = new GLubyte[size*4];
  for (unsigned i = 0; i < size; ++i){
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.GetWidth(), image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  delete buffer;
  return tex;
}

void Engine::blit(int x, int y, int width, int height, GLuint tex){
  glPushMatrix();
  glTranslatef(x,y,0.0f);
  glScalef(width,height,1.0f);
  glBindTexture(GL_TEXTURE_2D, tex);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glPopMatrix();
}

void Engine::insertToBlit(Object2D* obj){
  mBlitQueue.push_front(obj);
  resortBlit(obj);
}

void Engine::resortBlit(Object2D* obj){
  //search obj
  std::list<Object2D*>::iterator objectiter;
  for (std::list<Object2D*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){
    if (*iter == obj){
      objectiter = iter;
      break;
    }
  }
  //move object
  for (std::list<Object2D*>::iterator iter = objectiter++; objectiter != mBlitQueue.end(); ++objectiter){
    if ((*objectiter)->depth < (*iter)->depth){
      std::swap(*objectiter, *iter);
    }
    else
      break;
  }
}
