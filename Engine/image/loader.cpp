#include "loader.h"

#include <cstdlib>
#include <cstdio>

#include "../system/engine.h"

#ifdef WIN32
#undef FAR
#define INT32 this_is_really
#define boolean an_ugly_hack
#endif
extern "C"{
#include <jpeglib.h>
}

#include "image.h"

using namespace CGE;

static void decodeJPG(jpeg_decompress_struct* cinfo, Image *pImageData);

//loads the JPG file and returns it's data in a Image struct
Image* ImageLoader::loadJPG(const char *strFileName){
  struct jpeg_decompress_struct cinfo;
  Image *pImageData = NULL;
  FILE *pFile;
  if((pFile = fopen(strFileName, "rb")) == NULL){
    CGE::Log << "Unable to load JPG File!";
    return NULL;
  }
  // Create an error handler
  jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);

  // Initialize the decompression object
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, pFile);

  pImageData = (Image*)malloc(sizeof(Image));
  // Decode the jpeg file and fill in the image data structure to pass back
  decodeJPG(&cinfo, pImageData);

  jpeg_destroy_decompress(&cinfo);
  fclose(pFile);

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
