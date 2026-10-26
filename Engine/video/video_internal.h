#ifndef CGE_VIDEO_INTERNAL_H
#define CGE_VIDEO_INTERNAL_H

// The video module's internals: the two decoders and the colour conversion.

#include "videodecoder.h"

namespace CGE{

namespace VideoInternal{

//! one plane of a Y'CbCr picture
struct Plane{
  const unsigned char* data;
  int stride;
};

//! converts a Y'CbCr picture (BT.601, video range) to RGBA8
/*! xShift and yShift: the chroma planes' subsampling (1 for half, as in
    4:2:0). x0 and y0: where the picture starts in the planes. */
void yuvToRgba(const Plane& y, const Plane& cb, const Plane& cr, int xShift, int yShift,
  int x0, int y0, int width, int height, unsigned char* rgba);

//! an MPEG-1 program stream, or null
std::unique_ptr<VideoDecoder> openMpeg(std::vector<unsigned char>& data);
//! Theora in Ogg, or null
std::unique_ptr<VideoDecoder> openTheora(std::vector<unsigned char>& data);

}

}

#endif
