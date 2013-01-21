#include "triangle.h"

#include <cmath>

#include "state.h"
#include "framebuffer.h"
#include "shader.h"

using namespace std;

#if 0

#define f01(x, y) ((y0 - y1)*x + (x1 - x0)*y + x0*y1 - x1*y0)
#define f12(x, y) ((y1 - y2)*x + (x2 - x1)*y + x1*y2 - x2*y1)
#define f20(x, y) ((y2 - y0)*x + (x0 - x2)*y + x2*y0 - x0*y2)

void Triangle::raster(VRState* state, int numInterpolations){
  float x0 = mCoords[mIdx0*2];
  float x1 = mCoords[mIdx1*2];
  float x2 = mCoords[mIdx2*2];
  float y0 = mCoords[mIdx0*2+1];
  float y1 = mCoords[mIdx1*2+1];
  float y2 = mCoords[mIdx2*2+1];
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
          state->getCurrentSurface()->setPixel(int(x+0.5f), int(y+0.5f), red, green, blue, 255);
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
#else

#define fl2fp(val, shift) ((int)((float)(1 << shift) * val + 0.5f))
#define fmul(a, b) ((a*b) >> 4)

void Triangle::raster(VRState* state, int numInterpolations){
  //28.4 fixed-point
  const int Y1 = (int)(16.0f * mCoords[mIdx0*2+1] +0.5);
  const int Y2 = (int)(16.0f * mCoords[mIdx1*2+1] +0.5);
  const int Y3 = (int)(16.0f * mCoords[mIdx2*2+1] +0.5);
  const int X1 = (int)(16.0f * mCoords[mIdx0*2] +0.5);
  const int X2 = (int)(16.0f * mCoords[mIdx1*2] +0.5);
  const int X3 = (int)(16.0f * mCoords[mIdx2*2] +0.5);

  //deltas 28.4
  const int DX12 = X1 - X2;
  const int DX23 = X2 - X3;
  const int DX31 = X3 - X1;
  const int DY12 = Y1 - Y2;
  const int DY23 = Y2 - Y3;
  const int DY31 = Y3 - Y1;

  //fixed-point deltas 24.8
  const int FDX12 = DX12 << 4;
  const int FDX23 = DX23 << 4;
  const int FDX31 = DX31 << 4;
  const int FDY12 = DY12 << 4;
  const int FDY23 = DY23 << 4;
  const int FDY31 = DY31 << 4;

  //bounding rectangle 32.0
  int minx = min(X1, X2); minx = min(minx, X3); minx = (minx + 0xF) >> 4;
  int maxx = max(X1, X2); maxx = max(maxx, X3); maxx = (maxx + 0xF) >> 4;
  int miny = min(Y1, Y2); miny = min(miny, Y3); miny = (miny + 0xF) >> 4;
  int maxy = max(Y1, Y2); maxy = max(maxy, Y3); maxy = (maxy + 0xF) >> 4;

  //block size, must be power of two
  const int q = 8;

  // Start in corner of block
  minx &= ~(q - 1);
  miny &= ~(q - 1);

  int cbypos = miny;

  //half-edge constants 24.8
  int C1 = DY12 * X1 - DX12 * Y1;
  int C2 = DY23 * X2 - DX23 * Y2;
  int C3 = DY31 * X3 - DX31 * Y3;

  //top-left fill convention correction
  if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
  if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
  if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

  //interpolation setup
  //24.8
  int fAlpha = C2 + DX23 * Y1 - DY23 * X1;

  int CY1 = C1 + DX12 * (miny << 4) - DY12 * (minx << 4);
  int CY2 = C2 + DX23 * (miny << 4) - DY23 * (minx << 4);
  int CY3 = C3 + DX31 * (miny << 4) - DY31 * (minx << 4);

  int interp_dx[NUM_VARYING*4];
  int interp_dy[NUM_VARYING*4];
  int interp_starts[NUM_VARYING*4];

  //for (int i = 0; i < NUM_VARYING; ++i){
  for (int i = 0; i < 4; ++i){
    interp_dx[i] = (int)(FDY23*mVarying[0][mIdx0*4+i])+(int)(FDY31*mVarying[0][mIdx1*4+i])+(int)(FDY12*mVarying[0][mIdx2*4+i]);
    interp_dy[i] = (int)(FDX23*mVarying[0][mIdx0*4+i])+(int)(FDX31*mVarying[0][mIdx1*4+i])+(int)(FDX12*mVarying[0][mIdx2*4+i]);
    interp_starts[i] = (int)(CY2*mVarying[0][mIdx0*4+i])+(int)(CY3*mVarying[0][mIdx1*4+i])+(int)(CY1*mVarying[0][mIdx2*4+i]);
  }
  //}

  //interpolation setup end

  //loop through blocks
  for(int y = miny; y < maxy; y += q){
    for(int x = minx; x < maxx; x += q){
      // corners of block
      int x0 = x << 4;
      int x1 = (x + q - 1) << 4;
      int y0 = y << 4;
      int y1 = (y + q - 1) << 4;

      //evaluate half-space functions
      bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
      bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
      bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
      bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
      int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

      bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
      bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
      bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
      bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
      int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

      bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
      bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
      bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
      bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
      int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

      //skip block when outside an edge
      if(a == 0x0 || b == 0x0 || c == 0x0)
        continue;

      int ypos = cbypos;

      //accept whole block when totally covered
      if(a == 0xF && b == 0xF && c == 0xF){
        int interp_values[NUM_VARYING*4];
        for (int i = 0; i < 4; ++i){
          interp_values[i] = interp_starts[i]+(y-miny)*interp_dy[i]-(x-minx)*interp_dx[i];
        }
        for(int iy = 0; iy < q; iy++){
          
          int interp_res[NUM_VARYING*4];
          for (int i = 0; i < 4; ++i){
            interp_res[i] = interp_values[i];
          }

          for(int ix = x; ix < x + q; ix++){
            float res[4];
            for (int i = 0; i < 4; ++i){
              res[i] = interp_res[i]/(float)fAlpha;
            }
            state->getActiveShader()->setVarying(0, res);
            state->getActiveShader()->shade(state->getCurrentSurface(), ix, ypos);
            //state->getCurrentSurface()->setPixel(ix, ypos, (interp_res[0]/(float)fAlpha)*255, (interp_res[1]/(float)fAlpha)*255, (interp_res[2]/(float)fAlpha)*255, (interp_res[3]/(float)fAlpha)*255);
            
            for (int i = 0; i < 4; ++i){
              interp_res[i] -= interp_dx[i];
            }
          }
          for (int i = 0; i < 4; ++i){
            interp_values[i] += interp_dy[i];
          }
          ypos++;
        }
      }
      else{ //partially covered block
        int CY1 = C1 + DX12 * y0 - DY12 * x0;
        int CY2 = C2 + DX23 * y0 - DY23 * x0;
        int CY3 = C3 + DX31 * y0 - DY31 * x0;

        int interp_values[NUM_VARYING*4];
        
        for (int i = 0; i < 4; ++i){
          //interp_values[i] = (int)(CY2*mVarying[0][mIdx0*4+i])+(int)(CY3*mVarying[0][mIdx1*4+i])+(int)(CY1*mVarying[0][mIdx2*4+i]);
          interp_values[i] = interp_starts[i]+(y-miny)*interp_dy[i]-(x-minx)*interp_dx[i];
        }

        for(int iy = y; iy < y + q; iy++){
          int CX1 = CY1;
          int CX2 = CY2;
          int CX3 = CY3;
          int interp_res[NUM_VARYING*4];

          for (int i = 0; i < 4; ++i){
            interp_res[i] = interp_values[i];
          }

          for(int ix = x; ix < x + q; ix++){
            if(CX1 > 0 && CX2 > 0 && CX3 > 0){
              float res[4];
              for (int i = 0; i < 4; ++i){
                res[i] = interp_res[i]/(float)fAlpha;
              }
              state->getActiveShader()->setVarying(0, res);
              state->getActiveShader()->shade(state->getCurrentSurface(), ix, ypos);
              //state->getCurrentSurface()->setPixel(ix, ypos, (interp_res[0]/(float)fAlpha)*255, (interp_res[1]/(float)fAlpha)*255, (interp_res[2]/(float)fAlpha)*255, (interp_res[3]/(float)fAlpha)*255);
            }
            CX1 -= FDY12;
            CX2 -= FDY23;
            CX3 -= FDY31;
            for (int i = 0; i < 4; ++i){
              interp_res[i] -= interp_dx[i];
            }
          }
          CY1 += FDX12;
          CY2 += FDX23;
          CY3 += FDX31;
          for (int i = 0; i < 4; ++i){
            interp_values[i] += interp_dy[i];
          }
          ++ypos;
        }
      }
    }
    cbypos += q;
  }
}

#endif
