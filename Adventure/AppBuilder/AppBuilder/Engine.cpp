#include "Engine.h"

#include <wx/image.h>

#include "AdvDoc.h"

Engine* Engine::mInstance = NULL;

Engine::Engine() : mData(NULL){

}

Engine::~Engine(){

}

bool Engine::loadRoom(std::string name){
  if (!mData)
    return false;
  Room* room = mData->getRoom(name);
  if (!room)
    return false;
  GLuint tex = genTexture(room->background);
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
    else
      buffer[4*i+3] = 255;
  }
  GLuint tex;
  glGenTextures(1,&tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.GetWidth(), image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  delete buffer;
  return tex;
}
