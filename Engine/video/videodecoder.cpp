// Picks the decoder for a video, and converts the decoders' Y'CbCr pictures
// to RGBA.

#include "video_internal.h"

#include <cstring>

using namespace CGE;

namespace{

//! an MPEG-1 pack header: its start code and the '0010' marker (MPEG-2's is '01')
bool isMpeg1(const unsigned char* data, size_t size){
  return size >= 5 && data[0] == 0 && data[1] == 0 && data[2] == 1 && data[3] == 0xba && (data[4] & 0xf0) == 0x20;
}

bool isOgg(const unsigned char* data, size_t size){
  return size >= 4 && memcmp(data, "OggS", 4) == 0;
}

unsigned char clampByte(int value){
  return (unsigned char)(value < 0 ? 0 : value > 255 ? 255 : value);
}

}

bool VideoDecoder::isVideo(const unsigned char* data, size_t size){
  return isMpeg1(data, size) || isOgg(data, size);
}

std::unique_ptr<VideoDecoder> VideoDecoder::open(std::vector<unsigned char> data){
  if (isMpeg1(data.data(), data.size()))
    return VideoInternal::openMpeg(data);
  if (isOgg(data.data(), data.size()))
    return VideoInternal::openTheora(data);
  return nullptr;
}

void VideoInternal::yuvToRgba(const Plane& y, const Plane& cb, const Plane& cr, int xShift, int yShift,
  int x0, int y0, int width, int height, unsigned char* rgba){
  //BT.601 in 8.8 fixed point; luma from 16 to 235, chroma from 16 to 240
  for (int row = 0; row < height; ++row){
    const unsigned char* yRow = y.data + (y0+row)*y.stride + x0;
    const unsigned char* cbRow = cb.data + ((y0+row) >> yShift)*cb.stride;
    const unsigned char* crRow = cr.data + ((y0+row) >> yShift)*cr.stride;
    unsigned char* out = rgba + (size_t)row*width*4;
    for (int col = 0; col < width; ++col){
      int c = 298*(yRow[col]-16) + 128;
      int d = cbRow[(x0+col) >> xShift] - 128;
      int e = crRow[(x0+col) >> xShift] - 128;
      out[0] = clampByte((c + 409*e) >> 8);
      out[1] = clampByte((c - 100*d - 208*e) >> 8);
      out[2] = clampByte((c + 516*d) >> 8);
      out[3] = 255;
      out += 4;
    }
  }
}
