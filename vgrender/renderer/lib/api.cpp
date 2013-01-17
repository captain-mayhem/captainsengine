
#include "api.h"

VRSurface::VRSurface(int width, int height) : mBuffer(4, width, height){
  
}

void VRSurface::clear(unsigned char r, unsigned char g, unsigned char b, unsigned char a){
  unsigned char* data = mBuffer.getData();
  for (unsigned j = 0; j < mBuffer.getHeight(); ++j){
    for (unsigned i = 0; i < mBuffer.getWidth(); ++i){
      data[0] = r;
      data[1] = g;
      data[2] = b;
      data[3] = a;
      data += 4;
    }
  }
}

#include <cmath>

using namespace std;

void Triangle::raster(VRSurface* surface){
  float xMin = min(x0, x1); xMin = min(xMin, x2);
  float xMax = max(x0, x1); xMax = max(xMax, x2);
  float yMin = min(y0, y1); yMin = min(yMin, y2);
  float yMax = max(y0, y1); yMax = max(yMax, y2);

  float fAlpha = f12(x0, y0);
  float fBeta = f20(x1, y1);
  float fGamma = f01(x2, y2);
  bool boundAlpha = fAlpha*f12(-1, -1) > 0;
  bool boundBeta = fBeta*f20(-1, -1) > 0;
  bool boundGamma = fGamma*f01(-1, -1) > 0;
  float alpha = f12(xMin, yMin)/fAlpha;
  float beta = f20(xMin, yMin)/fBeta;
  float gamma = f01(xMin, yMin)/fGamma;
  float alphaLine = alpha;
  float betaLine = beta;
  float gammaLine = gamma;
  
  float alphaIncX = (y1 - y2)/fAlpha;
  float betaIncX = (y2 - y0)/fBeta;
  float gammaIncX = (y0 - y1)/fGamma;
  float alphaIncY = (x2 - x1)/fAlpha;
  float betaIncY = (x0 - x2)/fBeta;
  float gammaIncY = (x1 - x0)/fGamma;

  for (float y = yMin; y < yMax; ++y){
    alpha = alphaLine;
    beta = betaLine;
    gamma = gammaLine;
    for (float x = xMin; x < xMax; ++x){
      if (alpha >= 0 && beta >= 0 && gamma >= 0){
        if ((alpha > 0 || boundAlpha) && (beta > 0 || boundBeta) && (gamma > 0 || boundGamma)){
          unsigned char red = alpha*255;
          unsigned char green = beta*255;
          unsigned char blue = gamma*255;
          surface->setPixel(int(x+0.5f), int(y+0.5f), red, green, blue, 255);
        }
      }
      alpha += alphaIncX;
      beta += betaIncX;
      gamma += gammaIncX;
    }
    alphaLine += alphaIncY;
    betaLine += betaIncY;
    gammaLine += gammaIncY;
  }
}
