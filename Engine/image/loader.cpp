#include "loader.h"

#include <cstdlib>
#include <cstdio>

#include "../system/engine.h"
#include "../io/BinFileReader.h"
#include "../io/MemReader.h"

#ifdef WIN32
#undef FAR
#define INT32 this_is_really
#define boolean an_ugly_hack
#endif
extern "C"{
#include <jpeglib.h>
#include <extern/gif/gif_lib.h>
#include <extern/png/png.h>
}

#include "image.h"

TR_CHANNEL(CGE_Imageloader);

using namespace CGE;

Image* ImageLoader::load(const char* filename, Type t){
  if (t == UNKNOWN)
    t = determineType(filename);
  switch(t){
    case JPG:
      return loadJPG(filename);
    case GIF:
      return loadGIF(filename);
    case BMP:
      return loadBMP(filename);
    case PNG:
      return loadPNG(filename);
    case UNKNOWN:
      return NULL;
  }
  return NULL;
}

Image* ImageLoader::load(void* memory, unsigned size, Type t){
  switch(t){
    case JPG:
      return loadJPG(memory, size);
    case GIF:
      return loadGIF(memory, size);
    case BMP:
      return loadBMP(memory, size);
    case PNG:
      return loadPNG(memory, size);
    case UNKNOWN:
      return NULL;
  }
  return NULL;
}

ImageLoader::Type ImageLoader::determineType(const std::string& filename){
  TR_USE(CGE_Imageloader);
  size_t pos = filename.find_last_of('.');
  std::string ext = filename.substr(pos+1);
  ext = toLower(ext);
  if (ext == "jpg" || ext == "jpeg")
    return JPG;
  else if (ext == "gif")
    return GIF;
  else if (ext == "bmp")
    return BMP;
  else if (ext == "png")
    return PNG;
  TR_WARN("Unknown image type %s", filename.c_str());
  return UNKNOWN;
}

static void decodeJPG(jpeg_decompress_struct* cinfo, Image *pImageData);

//loads the JPG file and returns it's data in a Image struct
Image* ImageLoader::loadJPG(const char *strFileName){
  TR_USE(CGE_Imageloader);
  struct jpeg_decompress_struct cinfo;
  Image *pImageData = NULL;
  FILE *pFile;
  if((pFile = fopen(strFileName, "rb")) == NULL){
    TR_ERROR("Unable to load JPG File %s!", strFileName);
    return NULL;
  }
  // Create an error handler
  jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);

  // Initialize the decompression object
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, pFile);

  pImageData = new Image();
  // Decode the jpeg file and fill in the image data structure to pass back
  decodeJPG(&cinfo, pImageData);

  jpeg_destroy_decompress(&cinfo);
  fclose(pFile);

  return pImageData;
}

METHODDEF(void) jpg_init_source(j_decompress_ptr cinfo)
{
}

METHODDEF(boolean) jpg_fill_input_buffer(j_decompress_ptr cinfo)
{
  struct jpeg_source_mgr* src = cinfo->src;
  static JOCTET FakeEOI[] = {0xff, JPEG_EOI};
  //WARNMS(cinfo, JWRN_JPEG_EOF);
  src->next_input_byte = FakeEOI;
  src->bytes_in_buffer = 2;
  return TRUE;
}

METHODDEF(void) jpg_skip_input_data(j_decompress_ptr cinfo, long num_bytes){
  struct jpeg_source_mgr* src = cinfo->src;
  if (num_bytes >= (long)src->bytes_in_buffer){
    jpg_fill_input_buffer(cinfo);
    return;
  }
  src->bytes_in_buffer -= num_bytes;
  src->next_input_byte += num_bytes;
}

METHODDEF(void) jpg_term_source(j_decompress_ptr cinfo){
}

Image* ImageLoader::loadJPG(void* memory, unsigned size){
  struct jpeg_decompress_struct cinfo;
  Image *pImageData = NULL;
  
  // Create an error handler
  jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);

  // Initialize the decompression object
  jpeg_create_decompress(&cinfo);
  
  cinfo.src = (struct jpeg_source_mgr*)(*cinfo.mem->alloc_small)((j_common_ptr)&cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
  cinfo.src->init_source = jpg_init_source;
  cinfo.src->fill_input_buffer = jpg_fill_input_buffer;
  cinfo.src->skip_input_data = jpg_skip_input_data;
  cinfo.src->resync_to_restart = jpeg_resync_to_restart;
  cinfo.src->term_source = jpg_term_source;
  cinfo.src->bytes_in_buffer = size;
  cinfo.src->next_input_byte = (const JOCTET*)memory;

  pImageData = new Image();
  // Decode the jpeg file and fill in the image data structure to pass back
  decodeJPG(&cinfo, pImageData);

  jpeg_destroy_decompress(&cinfo);

  return pImageData;
}

void decodeJPG(jpeg_decompress_struct* cinfo, Image *pImageData){
  //read in the header of the jpeg file
  jpeg_read_header(cinfo, TRUE);
  //decompress	
  jpeg_start_decompress(cinfo);

  pImageData->setFormat(cinfo->num_components, cinfo->image_width, cinfo->image_height);

  pImageData->allocateData();

  unsigned char** rowPtr = new unsigned char*[pImageData->getHeight()];
  for (unsigned i = 0; i < pImageData->getHeight(); i++)
    rowPtr[i] = &(pImageData->getData()[i * pImageData->getRowSpan()]);

  //read the rows in the image
  int rowsRead = 0;
  while (cinfo->output_scanline < cinfo->output_height){
    rowsRead += jpeg_read_scanlines(cinfo, &rowPtr[rowsRead], cinfo->output_height - rowsRead);
  }
  delete [] rowPtr;

  // Finish decompressing the data
  jpeg_finish_decompress(cinfo);
}

static CGE::Image* decodeGIF(GifFileType* giffile);

Image* ImageLoader::loadGIF(const char *fileName){
  TR_USE(CGE_Imageloader);
  GifFileType* giffile = DGifOpenFileName(fileName);
  if (!giffile){
    TR_ERROR("Unable to load GIF File %s!", fileName);
    return NULL;
  }
  return decodeGIF(giffile);
}

static int gif_input_func(GifFileType* giffile, GifByteType* bytes, int length){
  memcpy(bytes, giffile->UserData, length);
  unsigned char* data = (unsigned char*)giffile->UserData;
  data += length;
  giffile->UserData = data;
  return length;
}

Image* ImageLoader::loadGIF(void* memory, unsigned size){
  TR_USE(CGE_Imageloader);
  GifFileType* giffile = DGifOpen(memory, gif_input_func);
  if (!giffile){
    TR_ERROR("Unable to load GIF File from memory!");
    return NULL;
  }
  return decodeGIF(giffile);
}

Image* decodeGIF(GifFileType* giffile){
  TR_USE(CGE_Imageloader);
  GifRecordType recordtype;
  int extcode;
  GifByteType* extension;
  unsigned char transparent;
  bool hasTransparency = false;
  unsigned char* indices = NULL;
  do{
    DGifGetRecordType(giffile, &recordtype);
    switch(recordtype){
      case EXTENSION_RECORD_TYPE:
        DGifGetExtension(giffile, &extcode, &extension);
        if (extcode == GRAPHICS_EXT_FUNC_CODE){
          hasTransparency = (extension[1] & 0x01) > 0;
          transparent = extension[4];
        }
        while (extension != NULL){
          DGifGetExtensionNext(giffile, &extension);
        }
        break;
      case IMAGE_DESC_RECORD_TYPE:
        DGifGetImageDesc(giffile);
        indices  = new unsigned char[giffile->Image.Width*giffile->Image.Height];
        if (giffile->Image.Interlace){
          static int interlacedOffset[] = { 0, 4, 2, 1 };
          static int interlacedJumps[] = { 8, 8, 4, 2 };
          for (int i = 0; i < 4; ++i){
            for (int j = interlacedOffset[i]; j < giffile->Image.Height; j += interlacedJumps[i]){
              DGifGetLine(giffile, indices+j*giffile->Image.Width, giffile->Image.Width);
            }
          }
        }
        else{
          for (int i = 0; i < giffile->Image.Height; ++i){
            DGifGetLine(giffile, indices+i*giffile->Image.Width, giffile->Image.Width);
          }
        }
        /*{
        CGE::Image test(1, giffile->Image.Width, giffile->Image.Height, indices);
        char tmpstr[32];
        static int imgcount = 1;
        sprintf(tmpstr, "tmp%i", ++imgcount);
        test.debugWrite(tmpstr);
        }*/
        {
        CGE::Image* img = new CGE::Image();
        int channels = hasTransparency ? 4 : 3;
        img->setFormat(channels, giffile->Image.Width, giffile->Image.Height);
        img->allocateData();
        for (int i = 0; i < giffile->Image.Width*giffile->Image.Height; ++i){
          unsigned char* color = &giffile->SColorMap->Colors[indices[i]].Red;
          memcpy(img->getData()+channels*i, color, 3);
          if (hasTransparency){
            if (indices[i] == transparent)
              img->getData()[channels*i+3] = 0x0;
            else
              img->getData()[channels*i+3] = 0xFF;
          }
        }
        delete [] indices;
        DGifCloseFile(giffile);
        return img;
        }
        break;
      case TERMINATE_RECORD_TYPE:
        break;
      default:
        TR_BREAK("Unhandled record type");
        break;
    }
  } while (recordtype != TERMINATE_RECORD_TYPE);
  return NULL;
}

static Image* decodeBMP(Reader* rdr);

Image* ImageLoader::loadBMP(const char *filename){
  BinFileReader rdr(filename);
  if (!rdr.isWorking())
    return NULL;
  return decodeBMP(&rdr);
}

Image* ImageLoader::loadBMP(void* memory, unsigned size){
  MemReader rdr(memory, size);
  return decodeBMP(&rdr);
}

static void determineShift(unsigned mask, unsigned& right, unsigned& left){
  unsigned zeros = 0;
  while((mask & 1) == 0){
    ++zeros;
    mask >>= 1;
  }
  unsigned ones = 0;
  while((mask & 1) == 1){
    ++ones;
    mask >>= 1;
  }
  right = zeros;
  left = 8-ones;
}

Image* decodeBMP(Reader* rdr){
  TR_USE(CGE_Imageloader);
  //FILE *pFile = NULL;
  Image *pImage = new Image();
  unsigned short int bfType;
  long int bfOffBits;
  short int biPlanes;
  short int biBitCount;
  long int biSizeImage;
  int i;
  unsigned char temp;

  bfType = rdr->readShort();

  //check if file is a bitmap
  if (bfType != 19778)
    EXIT2("Not a Bitmap-File!\n");

  //get the file size
  rdr->skip(8);

  // get the position of the actual bitmap data
  bfOffBits = rdr->readInt();

  //skip size of bitmap info header
  int bihsize = rdr->readInt();

  // get the width of the bitmap
  int width = rdr->readInt();

  // get the height of the bitmap
  int height = rdr->readInt();

  // get the number of planes
  biPlanes = rdr->readShort();

  if (biPlanes != 1)
    EXIT2("Number of Planes not 1!\n");

  // get the number of bits per pixel
  biBitCount = rdr->readShort();

  int biCompression = rdr->readInt();
  int rmask = 0x7c00;
  int gmask = 0x3e0;
  int bmask = 0x1f;

  pImage->setFormat(3, width, height);

  //calculate the size of the image
  biSizeImage = pImage->getImageSize();
  pImage->allocateData();

  //seek to the actual data
  if (biCompression == 0){
    rdr->skip(bihsize-20);
  }
  else if (biCompression == 3){
    rdr->skip(bihsize-20);
    rmask = rdr->readInt();
    gmask = rdr->readInt();
    bmask = rdr->readInt();
  }
  else{
    TR_WARN("bitmap comression %i unhandled", biCompression);
    rdr->skip(bfOffBits-34);
  }

  unsigned rrshift, rlshift;
  unsigned grshift, glshift;
  unsigned brshift, blshift;
  determineShift(rmask, rrshift, rlshift);
  determineShift(gmask, grshift, glshift);
  determineShift(bmask, brshift, blshift);
  if (biBitCount == 16){
    //555
    for (int i = 0; i < width*height; ++i){
      unsigned short tmp = rdr->readShort();
      pImage->getData()[3*i+0] = ((tmp & rmask) >> rrshift) << rlshift;//((tmp >> 10)& 0x1f)<<3;
      pImage->getData()[3*i+1] = ((tmp & gmask) >> grshift) << glshift;//((tmp >> 5)& 0x1f)<<3;
      pImage->getData()[3*i+2] = ((tmp & bmask) >> brshift) << blshift;//>> bshift;//((tmp >> 0)& 0x1f)<<3;
    }
  }
  else if (biBitCount == 24){

    //calculate the stride
    int stuffedsize = width*3;
    stuffedsize = ((stuffedsize+3)/4)*4;
    int toskip = stuffedsize-width*3;
    for (i = 0; i < height; ++i){
      rdr->readBytes(pImage->getData()+i*width*3, width*3);
      rdr->skip(toskip);
    }

    // swap red and blue (bgr -> rgb)
    for (i = 0; i < biSizeImage; i += 3)
    {
      temp = pImage->getData()[i];
      pImage->getData()[i] = pImage->getData()[i + 2];
      pImage->getData()[i + 2] = temp;
    }
  }
	else if (biBitCount == 32){
		for (i = 0; i < height; ++i){
			for (int j = 0; j < width; ++j){
				//xrgb
				rdr->skip(1);
				rdr->readBytes(pImage->getData()+i*width*3+j*3, 3);
			}
    }
	}
  else{
    TR_ERROR("Unhandled bit depth %i", biBitCount);
  }

  pImage->flipHorizontally();
  /*
  char tmpstr[32];
  static int imgcount = 1;
  sprintf(tmpstr, "tmp%i", ++imgcount);
  pImage->debugWrite(tmpstr);
  */
  return pImage;
}

struct pngReadStruct{
  unsigned char* data;
};

static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length){
  pngReadStruct* source = (pngReadStruct*)png_get_io_ptr(png_ptr);
  memcpy(data, source->data, length);
  source->data += length;
}

static Image* loadPNGinternal(png_structp png_ptr, png_infop info_ptr){
  TR_USE(CGE_Imageloader);

  png_read_info(png_ptr, info_ptr);

  unsigned width = 0;
  unsigned height = 0;
  int bitdepth = 0;
  int colortype = 0;
  int dummy;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitdepth, &colortype, &dummy, NULL, NULL);

  int channels = 0;
  if (colortype == PNG_COLOR_TYPE_GRAY){
    if (bitdepth < 8)
      png_set_expand_gray_1_2_4_to_8(png_ptr);
    channels = 1;
  }
  else if (colortype == PNG_COLOR_TYPE_GA)
    channels = 2;
  else if (colortype == PNG_COLOR_TYPE_PALETTE){
    png_set_palette_to_rgb(png_ptr);
    channels = 3;
  }
  else if (colortype == PNG_COLOR_TYPE_RGB)
    channels = 3;
  else if (colortype == PNG_COLOR_TYPE_RGBA)
    channels = 4;

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)){
    png_set_tRNS_to_alpha(png_ptr);
    channels = 4;
  }

  png_set_strip_16(png_ptr);
  png_set_packing(png_ptr);

  Image* image = new Image();
  image->setFormat(channels, width, height);
  image->allocateData();

  png_bytep* row_pointers = new png_bytep[height];
  for (unsigned row = 0; row < height; ++row){
    row_pointers[row] = (png_bytep)image->getData()+row*width*channels;
  }

  png_read_image(png_ptr, row_pointers);

  delete [] row_pointers;

  return image;
}

Image* ImageLoader::loadPNG(const char *fileName){
  FILE* fp = fopen(fileName, "rb");
  if (fp == NULL)
    return NULL;

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  setjmp(png_jmpbuf(png_ptr));

  png_init_io(png_ptr, fp);

  Image* img = loadPNGinternal(png_ptr, info_ptr);

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  fclose(fp);

  return img;
}

Image* ImageLoader::loadPNG(void* memory, unsigned size){
  TR_USE(CGE_Imageloader);
  
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  setjmp(png_jmpbuf(png_ptr));

  pngReadStruct rs;
  rs.data = (unsigned char*)memory;
  png_set_read_fn(png_ptr, &rs, png_read_data);

  Image* img = loadPNGinternal(png_ptr, info_ptr);

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  return img;
}
