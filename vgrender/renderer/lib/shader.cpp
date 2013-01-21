#include "shader.h"
#include "framebuffer.h"

void VRShader::shade(VRSurface* surface, int x, int y){
  execute();
  surface->setPixel(x, y, (unsigned char)(255*mColor[0]), 
    (unsigned char)(255*mColor[1]), 
    (unsigned char)(255*mColor[2]), 
    (unsigned char)(255*mColor[3]));
}

void FlatShader::execute(){
  mColor[0] = 1.0;
  mColor[1] = 0.0;
  mColor[2] = 0.0;
  mColor[3] = 1.0;
}

void GouraudShader::execute(){
  mColor[0] = mVarying[0][0];
  mColor[1] = mVarying[0][1];
  mColor[2] = mVarying[0][2];
  mColor[3] = mVarying[0][3];
}