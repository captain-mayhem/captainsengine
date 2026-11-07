// A TIFF decoder for ImageLoader: baseline TIFF and the common extensions,
// without a third-party library (zlib, which the engine links anyway, does
// the Deflate compression).
//
// What it reads: the first image of a little- or big-endian TIFF (not
// BigTIFF), in strips or tiles, chunky or planar; compression none, PackBits,
// LZW or Deflate, with or without the horizontal predictor; bilevel, gray
// (white or black is zero), palette, RGB and CMYK at 1, 2, 4, 8 or 16 bits per
// sample, with an associated (premultiplied) or unassociated alpha channel.
// What it doesn't: JPEG and CCITT fax compression, YCbCr, floating point
// samples, and orientations other than top-left (the image is returned as it
// is stored). Those report an error and return null.
//
// The result has 1 (gray), 2 (gray, alpha), 3 (RGB) or 4 (RGBA) channels of
// 8 bits, top row first, with straight alpha - like the other loaders.

#include "loader.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

#include <zlib.h>

#include "../system/engine.h"
#include "../system/types.h"
#include "image.h"

TR_CHANNEL(CGE_Imageloader_TIFF);

using namespace CGE;

namespace{

enum Tag{
  TAG_IMAGE_WIDTH = 256,
  TAG_IMAGE_LENGTH = 257,
  TAG_BITS_PER_SAMPLE = 258,
  TAG_COMPRESSION = 259,
  TAG_PHOTOMETRIC = 262,
  TAG_FILL_ORDER = 266,
  TAG_STRIP_OFFSETS = 273,
  TAG_SAMPLES_PER_PIXEL = 277,
  TAG_ROWS_PER_STRIP = 278,
  TAG_STRIP_BYTE_COUNTS = 279,
  TAG_PLANAR_CONFIG = 284,
  TAG_PREDICTOR = 317,
  TAG_COLOR_MAP = 320,
  TAG_TILE_WIDTH = 322,
  TAG_TILE_LENGTH = 323,
  TAG_TILE_OFFSETS = 324,
  TAG_TILE_BYTE_COUNTS = 325,
  TAG_INK_SET = 332,
  TAG_EXTRA_SAMPLES = 338,
  TAG_SAMPLE_FORMAT = 339,
};

enum Compression{
  COMPRESSION_NONE = 1,
  COMPRESSION_LZW = 5,
  COMPRESSION_DEFLATE = 8,
  COMPRESSION_DEFLATE_OLD = 32946,
  COMPRESSION_PACKBITS = 32773,
};

enum Photometric{
  PHOTOMETRIC_WHITE_IS_ZERO = 0,
  PHOTOMETRIC_BLACK_IS_ZERO = 1,
  PHOTOMETRIC_RGB = 2,
  PHOTOMETRIC_PALETTE = 3,
  PHOTOMETRIC_SEPARATED = 5,
};

enum ExtraSample{
  EXTRA_UNSPECIFIED = 0,
  EXTRA_ASSOCIATED_ALPHA = 1,
  EXTRA_UNASSOCIATED_ALPHA = 2,
};

//! the TIFF file in memory, read in its byte order
class Reader{
public:
  //! the byte order comes from the header (readHeader()), or is given for data inside the file
  Reader(const uint8* data, size_t size, bool bigEndian = false) : mData(data), mSize(size), mBigEndian(bigEndian) {}

  bool readHeader(uint32& ifd){
    if (mSize < 8)
      return false;
    if (mData[0] == 'I' && mData[1] == 'I')
      mBigEndian = false;
    else if (mData[0] == 'M' && mData[1] == 'M')
      mBigEndian = true;
    else
      return false;
    if (u16(2) != 42)
      return false;
    ifd = u32(4);
    return true;
  }

  bool bigEndian() const {return mBigEndian;}
  size_t size() const {return mSize;}
  const uint8* data() const {return mData;}

  //! 0 for reads past the end, which the callers check against size()
  uint16 u16(size_t pos) const{
    if (pos + 2 > mSize)
      return 0;
    const uint8* p = mData + pos;
    return mBigEndian ? (uint16)((p[0] << 8) | p[1]) : (uint16)((p[1] << 8) | p[0]);
  }
  uint32 u32(size_t pos) const{
    if (pos + 4 > mSize)
      return 0;
    const uint8* p = mData + pos;
    return mBigEndian ? ((uint32)p[0] << 24) | ((uint32)p[1] << 16) | ((uint32)p[2] << 8) | p[3]
      : ((uint32)p[3] << 24) | ((uint32)p[2] << 16) | ((uint32)p[1] << 8) | p[0];
  }

private:
  const uint8* mData;
  size_t mSize;
  bool mBigEndian;
};

//! the tags of the first image file directory
class Directory{
public:
  bool read(const Reader& file, uint32 ifd){
    uint16 count = file.u16(ifd);
    if (count == 0 || ifd + 2 + (size_t)count * 12 > file.size())
      return false;
    for (uint16 i = 0; i < count; ++i){
      size_t entry = ifd + 2 + (size_t)i * 12;
      Field field;
      field.tag = file.u16(entry);
      field.type = file.u16(entry + 2);
      uint32 n = file.u32(entry + 4);
      size_t width = typeSize(field.type);
      //types the tags here don't use (rationals, floats, ...) are kept as their count only
      if (width == 0 || n > (file.size() / width)){
        field.count = n;
        mFields.push_back(field);
        continue;
      }
      size_t bytes = width * n;
      size_t pos = bytes <= 4 ? entry + 8 : file.u32(entry + 8);
      if (pos + bytes > file.size())
        return false;
      field.values.reserve(n);
      for (uint32 v = 0; v < n; ++v){
        size_t p = pos + v * width;
        if (width == 1)
          field.values.push_back(file.data()[p]);
        else if (width == 2)
          field.values.push_back(file.u16(p));
        else
          field.values.push_back(file.u32(p));
      }
      field.count = n;
      mFields.push_back(field);
    }
    return true;
  }

  bool has(uint16 tag) const {return find(tag) != nullptr;}

  uint32 get(uint16 tag, uint32 def) const{
    const Field* field = find(tag);
    return field && !field->values.empty() ? field->values[0] : def;
  }

  const std::vector<uint32>* values(uint16 tag) const{
    const Field* field = find(tag);
    return field ? &field->values : nullptr;
  }

private:
  struct Field{
    uint16 tag = 0;
    uint16 type = 0;
    uint32 count = 0;
    std::vector<uint32> values;
  };

  //! the size of BYTE, SHORT and LONG values (and their signed and undefined kin); 0 for others
  static size_t typeSize(uint16 type){
    switch (type){
      case 1: case 2: case 6: case 7: return 1;
      case 3: case 8: return 2;
      case 4: case 9: case 13: return 4;
    }
    return 0;
  }

  const Field* find(uint16 tag) const{
    for (const Field& field : mFields){
      if (field.tag == tag)
        return &field;
    }
    return nullptr;
  }

  std::vector<Field> mFields;
};

//! PackBits: a count byte n, then n+1 literal bytes (n >= 0) or one byte repeated 1-n times (n < 0)
bool unpackBits(const uint8* src, size_t len, std::vector<uint8>& out, size_t expected){
  size_t pos = 0;
  while (pos < len && out.size() < expected){
    int n = (int8_t)src[pos++];
    if (n >= 0){
      size_t run = (size_t)n + 1;
      if (pos + run > len)
        return false;
      out.insert(out.end(), src + pos, src + pos + run);
      pos += run;
    }
    else if (n != -128){
      if (pos >= len)
        return false;
      out.insert(out.end(), (size_t)(1 - n), src[pos++]);
    }
  }
  return true;
}

//! TIFF's LZW: codes of 9 to 12 bits, most significant bit first, with the "early change" of width
bool unpackLZW(const uint8* src, size_t len, std::vector<uint8>& out, size_t expected){
  const int CLEAR = 256;
  const int END = 257;
  //the old, least-significant-bit-first variant (before TIFF 6) starts with a clear code read the other way
  if (len >= 2 && src[0] == 0 && (src[1] & 1))
    return false;
  std::vector<int> prefix(4096);
  std::vector<uint8> suffix(4096);
  std::vector<uint8> first(4096);
  std::vector<int> length(4096);
  for (int i = 0; i < 256; ++i){
    prefix[i] = -1;
    suffix[i] = (uint8)i;
    first[i] = (uint8)i;
    length[i] = 1;
  }
  int width = 9;
  int next = 258;
  int prev = -1;
  size_t bitPos = 0;
  auto emit = [&](int code){
    size_t start = out.size();
    out.resize(start + length[code]);
    for (int c = code, i = length[code] - 1; c >= 0 && i >= 0; c = prefix[c], --i)
      out[start + i] = suffix[c];
  };
  while (out.size() < expected){
    if (bitPos + width > len * 8)
      break;
    int code = 0;
    for (int b = 0; b < width; ++b, ++bitPos)
      code = (code << 1) | ((src[bitPos >> 3] >> (7 - (bitPos & 7))) & 1);
    if (code == END)
      break;
    if (code == CLEAR){
      width = 9;
      next = 258;
      prev = -1;
      continue;
    }
    if (prev < 0){
      if (code >= 256)
        return false;
      emit(code);
      prev = code;
      continue;
    }
    //a known code adds the previous string plus its first byte; the code
    //being defined right now (the KwKwK case) is the previous string plus
    //the previous string's first byte, emitted once it exists
    if (code > next || (code == next && next >= 4096))
      return false;
    uint8 added = code < next ? first[code] : first[prev];
    bool known = code < next;
    if (known)
      emit(code);
    if (next < 4096){
      prefix[next] = prev;
      suffix[next] = added;
      first[next] = first[prev];
      length[next] = length[prev] + 1;
      ++next;
    }
    if (!known)
      emit(code);
    prev = code;
    if (next >= (1 << width) - 1 && width < 12)
      ++width;
  }
  return true;
}

//! Deflate (zlib format), up to the expected size
bool inflate(const uint8* src, size_t len, std::vector<uint8>& out, size_t expected){
  out.resize(expected);
  z_stream stream = {};
  stream.next_in = (Bytef*)src;
  stream.avail_in = (uInt)len;
  stream.next_out = out.data();
  stream.avail_out = (uInt)expected;
  if (inflateInit(&stream) != Z_OK)
    return false;
  int result = ::inflate(&stream, Z_FINISH);
  size_t produced = expected - stream.avail_out;
  inflateEnd(&stream);
  out.resize(produced);
  //a truncated stream still gives the rows it holds
  return result == Z_STREAM_END || result == Z_OK || result == Z_BUF_ERROR;
}

uint8 reverseBits(uint8 b){
  b = (uint8)((b & 0xf0) >> 4 | (b & 0x0f) << 4);
  b = (uint8)((b & 0xcc) >> 2 | (b & 0x33) << 2);
  return (uint8)((b & 0xaa) >> 1 | (b & 0x55) << 1);
}

}

Image* ImageLoader::loadTIFF(void* memory, unsigned size){
  TR_USE(CGE_Imageloader_TIFF);
  Reader file((const uint8*)memory, size);
  uint32 ifd = 0;
  Directory dir;
  if (!file.readHeader(ifd) || !dir.read(file, ifd)){
    TR_ERROR("Not a TIFF file or a broken one");
    return nullptr;
  }

  const uint32 width = dir.get(TAG_IMAGE_WIDTH, 0);
  const uint32 height = dir.get(TAG_IMAGE_LENGTH, 0);
  const uint32 spp = dir.get(TAG_SAMPLES_PER_PIXEL, 1);
  const uint32 compression = dir.get(TAG_COMPRESSION, COMPRESSION_NONE);
  const uint32 planar = dir.get(TAG_PLANAR_CONFIG, 1);
  const uint32 predictor = dir.get(TAG_PREDICTOR, 1);
  const bool reversedBits = dir.get(TAG_FILL_ORDER, 1) == 2;
  uint32 bps = dir.get(TAG_BITS_PER_SAMPLE, 1);
  if (const std::vector<uint32>* all = dir.values(TAG_BITS_PER_SAMPLE)){
    for (uint32 b : *all){
      if (b != bps){
        TR_ERROR("TIFF with different sizes of samples");
        return nullptr;
      }
    }
  }
  //bilevel images have no photometric tag at times; then 0 is white
  const uint32 photometric = dir.get(TAG_PHOTOMETRIC, PHOTOMETRIC_WHITE_IS_ZERO);

  if (width == 0 || height == 0 || spp == 0 || spp > 16 || (width > (1u << 24)) || (height > (1u << 24))
    || (uint64_t)width * height * 4 > 0x7fffffff){
    TR_ERROR("TIFF of an unsupported size %ux%u", width, height);
    return nullptr;
  }
  if (bps != 1 && bps != 2 && bps != 4 && bps != 8 && bps != 16){
    TR_ERROR("TIFF with %u bits per sample is not supported", bps);
    return nullptr;
  }
  if (dir.get(TAG_SAMPLE_FORMAT, 1) != 1 && dir.get(TAG_SAMPLE_FORMAT, 1) != 2){
    TR_ERROR("TIFF with floating point samples is not supported");
    return nullptr;
  }
  if (compression != COMPRESSION_NONE && compression != COMPRESSION_LZW && compression != COMPRESSION_DEFLATE
    && compression != COMPRESSION_DEFLATE_OLD && compression != COMPRESSION_PACKBITS){
    TR_ERROR("TIFF compression %u is not supported", compression);
    return nullptr;
  }
  if (predictor != 1 && !(predictor == 2 && (bps == 8 || bps == 16))){
    TR_ERROR("TIFF predictor %u is not supported for %u bits", predictor, bps);
    return nullptr;
  }

  //the color samples, then the extra ones
  uint32 colorSamples;
  switch (photometric){
    case PHOTOMETRIC_WHITE_IS_ZERO:
    case PHOTOMETRIC_BLACK_IS_ZERO:
    case PHOTOMETRIC_PALETTE:
      colorSamples = 1;
      break;
    case PHOTOMETRIC_RGB:
      colorSamples = 3;
      break;
    case PHOTOMETRIC_SEPARATED:
      //only CMYK, the usual ink set
      colorSamples = 4;
      if (dir.get(TAG_INK_SET, 1) != 1){
        TR_ERROR("TIFF with an ink set other than CMYK is not supported");
        return nullptr;
      }
      break;
    default:
      TR_ERROR("TIFF photometric interpretation %u is not supported", photometric);
      return nullptr;
  }
  if (spp < colorSamples){
    TR_ERROR("TIFF has %u samples for %u colors", spp, colorSamples);
    return nullptr;
  }
  if (photometric == PHOTOMETRIC_PALETTE && (bps > 8 || !dir.values(TAG_COLOR_MAP)
    || dir.values(TAG_COLOR_MAP)->size() < 3u * (1u << bps))){
    TR_ERROR("TIFF palette image without a usable color map");
    return nullptr;
  }
  //the first extra sample is alpha if it says so
  uint32 alpha = EXTRA_UNSPECIFIED;
  if (spp > colorSamples){
    const std::vector<uint32>* extra = dir.values(TAG_EXTRA_SAMPLES);
    if (extra && !extra->empty())
      alpha = (*extra)[0];
  }
  const bool hasAlpha = alpha == EXTRA_ASSOCIATED_ALPHA || alpha == EXTRA_UNASSOCIATED_ALPHA;

  //the chunks: strips or tiles, for each plane if planar
  const bool tiled = dir.has(TAG_TILE_WIDTH);
  const uint32 chunkWidth = tiled ? dir.get(TAG_TILE_WIDTH, 0) : width;
  const uint32 chunkHeight = tiled ? dir.get(TAG_TILE_LENGTH, 0) : (std::min)(dir.get(TAG_ROWS_PER_STRIP, height), height);
  const std::vector<uint32>* offsets = dir.values(tiled ? TAG_TILE_OFFSETS : TAG_STRIP_OFFSETS);
  const std::vector<uint32>* counts = dir.values(tiled ? TAG_TILE_BYTE_COUNTS : TAG_STRIP_BYTE_COUNTS);
  if (chunkWidth == 0 || chunkHeight == 0 || !offsets){
    TR_ERROR("TIFF without image data");
    return nullptr;
  }
  const uint32 across = (width + chunkWidth - 1) / chunkWidth;
  const uint32 down = (height + chunkHeight - 1) / chunkHeight;
  const uint32 planes = planar == 2 ? spp : 1;
  const uint32 samplesInChunk = planar == 2 ? 1 : spp;
  if ((uint64_t)across * down * planes > offsets->size()){
    TR_ERROR("TIFF has too few strips or tiles");
    return nullptr;
  }
  const size_t rowBytes = ((size_t)chunkWidth * samplesInChunk * bps + 7) / 8;
  const uint32 maxValue = (1u << bps) - 1;

  //the samples of all pixels at 8 bits; palette indices stay unscaled
  std::vector<uint8> samples((size_t)width * height * spp);
  std::vector<uint8> chunk;
  std::vector<uint8> source;
  for (uint32 plane = 0; plane < planes; ++plane){
    for (uint32 cy = 0; cy < down; ++cy){
      for (uint32 cx = 0; cx < across; ++cx){
        uint32 index = (plane * down + cy) * across + cx;
        uint32 offset = (*offsets)[index];
        //uncompressed images may leave the byte counts out
        uint32 count = counts && index < counts->size() ? (*counts)[index] : (uint32)(file.size() - (std::min<size_t>)(offset, file.size()));
        if (offset >= file.size())
          continue;
        count = (uint32)(std::min<size_t>)(count, file.size() - offset);
        //strips at the bottom are shorter, tiles are padded
        uint32 rows = tiled ? chunkHeight : (std::min)(chunkHeight, height - cy * chunkHeight);
        size_t expected = rowBytes * rows;
        const uint8* src = file.data() + offset;
        if (reversedBits){
          source.assign(src, src + count);
          for (uint8& b : source)
            b = reverseBits(b);
          src = source.data();
        }
        chunk.clear();
        bool ok = true;
        switch (compression){
          case COMPRESSION_NONE:
            chunk.assign(src, src + (std::min<size_t>)(count, expected));
            break;
          case COMPRESSION_PACKBITS:
            ok = unpackBits(src, count, chunk, expected);
            break;
          case COMPRESSION_LZW:
            ok = unpackLZW(src, count, chunk, expected);
            break;
          default:
            ok = inflate(src, count, chunk, expected);
            break;
        }
        if (!ok){
          TR_ERROR("Broken TIFF image data");
          return nullptr;
        }
        //a short chunk leaves the rest of it black (and transparent)
        chunk.resize(expected, 0);

        //undo the horizontal differencing, row by row
        if (predictor == 2){
          for (uint32 r = 0; r < rows; ++r){
            uint8* row = chunk.data() + r * rowBytes;
            if (bps == 8){
              for (size_t i = samplesInChunk; i < (size_t)chunkWidth * samplesInChunk; ++i)
                row[i] = (uint8)(row[i] + row[i - samplesInChunk]);
            }
            else{
              Reader words(row, rowBytes, file.bigEndian());
              for (size_t i = samplesInChunk; i < (size_t)chunkWidth * samplesInChunk; ++i){
                uint16 value = (uint16)(words.u16(i * 2) + words.u16((i - samplesInChunk) * 2));
                if (file.bigEndian()){
                  row[i*2] = (uint8)(value >> 8);
                  row[i*2+1] = (uint8)value;
                }
                else{
                  row[i*2] = (uint8)value;
                  row[i*2+1] = (uint8)(value >> 8);
                }
              }
            }
          }
        }

        //into the image's samples, scaled to 8 bits
        for (uint32 r = 0; r < rows; ++r){
          uint32 y = cy * chunkHeight + r;
          if (y >= height)
            break;
          const uint8* row = chunk.data() + r * rowBytes;
          Reader words(row, rowBytes, file.bigEndian());
          for (uint32 c = 0; c < chunkWidth; ++c){
            uint32 x = cx * chunkWidth + c;
            if (x >= width)
              break;
            for (uint32 s = 0; s < samplesInChunk; ++s){
              size_t i = (size_t)c * samplesInChunk + s;
              uint32 value;
              if (bps == 8)
                value = row[i];
              else if (bps == 16)
                value = words.u16(i * 2) >> 8;
              else{
                size_t bit = i * bps;
                value = (row[bit >> 3] >> (8 - bps - (bit & 7))) & maxValue;
                if (photometric != PHOTOMETRIC_PALETTE)
                  value = value * 255 / maxValue;
              }
              samples[((size_t)y * width + x) * spp + plane + s] = (uint8)value;
            }
          }
        }
      }
    }
  }

  //the samples as gray or RGB, and alpha
  const int channels = (colorSamples == 1 && photometric != PHOTOMETRIC_PALETTE ? 1 : 3) + (hasAlpha ? 1 : 0);
  const size_t pixels = (size_t)width * height;
  unsigned char* pixelData;
  //the samples are the pixels already: hand them over
  if (spp == (uint32)channels && (photometric == PHOTOMETRIC_BLACK_IS_ZERO || photometric == PHOTOMETRIC_RGB)
    && alpha != EXTRA_ASSOCIATED_ALPHA){
    pixelData = new unsigned char[samples.size()];
    memcpy(pixelData, samples.data(), samples.size());
  }
  else{
    pixelData = new unsigned char[pixels * channels];
    const std::vector<uint32>* map = dir.values(TAG_COLOR_MAP);
    const size_t mapSize = photometric == PHOTOMETRIC_PALETTE ? (size_t)1 << bps : 0;
    for (size_t p = 0; p < pixels; ++p){
      const uint8* in = samples.data() + p * spp;
      unsigned char* out = pixelData + p * channels;
      switch (photometric){
        case PHOTOMETRIC_WHITE_IS_ZERO:
          out[0] = (uint8)(255 - in[0]);
          break;
        case PHOTOMETRIC_BLACK_IS_ZERO:
          out[0] = in[0];
          break;
        case PHOTOMETRIC_PALETTE:
          //16-bit color map entries: all reds, then all greens, then all blues
          out[0] = (uint8)((*map)[in[0]] >> 8);
          out[1] = (uint8)((*map)[mapSize + in[0]] >> 8);
          out[2] = (uint8)((*map)[2 * mapSize + in[0]] >> 8);
          break;
        case PHOTOMETRIC_RGB:
          out[0] = in[0];
          out[1] = in[1];
          out[2] = in[2];
          break;
        case PHOTOMETRIC_SEPARATED:
          out[0] = (uint8)((255 - in[0]) * (255 - in[3]) / 255);
          out[1] = (uint8)((255 - in[1]) * (255 - in[3]) / 255);
          out[2] = (uint8)((255 - in[2]) * (255 - in[3]) / 255);
          break;
      }
      if (hasAlpha){
        uint8 a = in[colorSamples];
        int colors = channels - 1;
        out[colors] = a;
        //premultiplied colors back to straight ones
        if (alpha == EXTRA_ASSOCIATED_ALPHA){
          for (int c = 0; c < colors; ++c)
            out[c] = a == 0 ? 0 : (uint8)(std::min)(255, out[c] * 255 / a);
        }
      }
    }
  }

  Image* image = new Image();
  image->setFormat(channels, width, height);
  image->setData(pixelData);
  return image;
}
