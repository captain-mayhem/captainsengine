#include "../../system/engine.h"
#include "GLEStexture.h"

using namespace CGE;

typedef unsigned char byte;
typedef unsigned short WORD;

GLESTexture::GLESTexture(string filename) : Texture(filename){
  load(filename);
}

bool GLESTexture::load(string filename){
  Image *img = NULL;
  string end;
  end += filename[filename.length()-3];
  end += filename[filename.length()-2];
  end += filename[filename.length()-1];
  if (end == "jpg")
    img = loadJPG(filename.c_str());
  else if (end == "tga")
    img = loadTGA(filename.c_str());
  else if (end == "bmp")
    img = loadBMP(filename.c_str());
  else{
    EXIT2("Format ."+end+" not supported\n");
  }

  glGenTextures(1, &tex_);
  glBindTexture(GL_TEXTURE_2D, tex_);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->sizeX, img->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img->data);

  if (img){
    if (img->data){
      free(img->data);
    }
    free(img);
  }
  return true;
}

void GLESTexture::activate(){
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex_);
}


void GLESTexture::deactivate(){
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

//simple loader for 24bit bitmaps (data has to be in rgb-format)
Image* GLESTexture::loadBMP(const char *filename){
  FILE *pFile = NULL;
  Image *pImage = (Image*)malloc(sizeof(Image));
  unsigned short int bfType;
  long int bfOffBits;
  short int biPlanes;
  short int biBitCount;
  long int biSizeImage;
  int i;
  unsigned char temp;

  if ((pFile = fopen(filename, "rb")) == NULL)
    EXIT2("File not found.");
  if(!fread(&bfType, sizeof(short int), 1, pFile))
    EXIT2("Error reading file!\n");

  //check if file is a bitmap
  if (bfType != 19778)
    EXIT2("Not a Bitmap-File!\n");

  //get the file size
  fseek(pFile, 8, SEEK_CUR);

  // get the position of the actual bitmap data
  if (!fread(&bfOffBits, sizeof(long int), 1, pFile))
    EXIT2("Error reading file!\n");

  //skip size of bitmap info header
  fseek(pFile, 4, SEEK_CUR);

  // get the width of the bitmap
  fread(&pImage->sizeX, sizeof(int), 1, pFile);

  // get the height of the bitmap
  fread(&pImage->sizeY, sizeof(int), 1, pFile);

  // get the number of planes
  fread(&biPlanes, sizeof(short int), 1, pFile);
  if (biPlanes != 1)
    EXIT2("Number of Planes not 1!\n");

  // get the number of bits per pixel
  if (!fread(&biBitCount, sizeof(short int), 1, pFile))
    EXIT2("Error reading file!\n");

  if (biBitCount != 24)
    EXIT2("Bits per Pixel not 24\n");

  pImage->channels = 3;

  //calculate the size of the image
  biSizeImage = pImage->sizeX * pImage->sizeY * pImage->channels;
  pImage->data = (byte*) malloc(biSizeImage);

  //seek to the actual data
  fseek(pFile, bfOffBits, SEEK_SET);
  if (!fread(pImage->data, biSizeImage, 1, pFile))
    EXIT2("Error loading file!\n");

  // swap red and blue (bgr -> rgb)
  for (i = 0; i < biSizeImage; i += 3)
  {
    temp = pImage->data[i];
    pImage->data[i] = pImage->data[i + 2];
    pImage->data[i + 2] = temp;
  }
  fclose(pFile);
  return pImage;
}


//loads the TGA file and returns it's data in an Image struct
Image *GLESTexture::loadTGA(const char *fileName){
  WORD width = 0, height = 0;
  byte length = 0;
  byte imageType = 0;
  byte bits = 0;
  FILE *pFile = NULL;
  int channels = 0;
  int stride = 0;
  int i = 0;

  if((pFile = fopen(fileName, "rb")) == NULL){
    EXIT2("Unable to load TGA File!");
    return NULL;
  }
  Image* pImageData = (Image*)malloc(sizeof(Image));

  //length
  fread(&length, sizeof(byte), 1, pFile);
  fseek(pFile,1,SEEK_CUR); 

  // read in the imageType (RLE, RGB, etc...)
  fread(&imageType, sizeof(byte), 1, pFile);

  // skip general information
  fseek(pFile, 9, SEEK_CUR);

  // read the width, height and bits per pixel (16, 24 or 32)
  fread(&width,  sizeof(WORD), 1, pFile);
  fread(&height, sizeof(WORD), 1, pFile);
  fread(&bits,   sizeof(byte), 1, pFile);

  // Now we move the file pointer to the pixel data
  fseek(pFile, length + 1, SEEK_CUR); 

  // Check if the image is RLE compressed or not
  if(imageType != TGA_RLE){
    // Check if the image is a 24 or 32-bit image
    if(bits == 24 || bits == 32){
      channels = bits / 8;
      stride = channels * width;
      pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));

      // load in all the pixel data line by line
      for(int y = 0; y < height; y++){
        // Store a pointer to the current line of pixels
        unsigned char *pLine = &(pImageData->data[stride * y]);
        fread(pLine, stride, 1, pFile);

        //bgr -> rgb
        for(i = 0; i < stride; i += channels){
          int temp = pLine[i];
          pLine[i] = pLine[i+2];
          pLine[i + 2] = temp;
        }
      }
    }
    // Check if the image is a 16 bit image (RGB stored in 1 unsigned short)
    else if(bits==16){
      unsigned short pixels = 0;
      int r=0, g=0, b=0;

      //convert 16 to 24 bit
      channels = 3;
      stride = channels * width;
      pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));

      // Load in all the pixel data pixel by pixel
      for(int i = 0; i < width*height; i++){
        fread(&pixels, sizeof(unsigned short), 1, pFile);
        //Convert the 16-bit pixel into an RGB
        b = (pixels & 0x1f) << 3;
        g = ((pixels >> 5) & 0x1f) << 3;
        r = ((pixels >> 10) & 0x1f) << 3;

        pImageData->data[i*3+0] = r;
        pImageData->data[i*3+1] = g;
        pImageData->data[i*3+2] = b;
      }
    }
    else
      return NULL;
  }
  //Run-Length Encoded (RLE)
  else{
    // Create some variables to hold the rleID, current colors read, channels, & stride.
    byte rleID = 0;
    int colorsRead = 0;
    channels = bits / 8;
    stride = channels * width;

    pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));
    byte *pColors = ((byte*)malloc(sizeof(byte)*channels));

    // Load in all the pixel data
    while(i < width*height){
      // Read in the current color count
      fread(&rleID, sizeof(byte), 1, pFile);
      if(rleID < 128){
        rleID++;
        // Go through and read all the unique colors found
        while(rleID){
          // Read in the current color
          fread(pColors, sizeof(byte) * channels, 1, pFile);
          pImageData->data[colorsRead+0] = pColors[2];
          pImageData->data[colorsRead+1] = pColors[1];
          pImageData->data[colorsRead+2] = pColors[0];
          // If it is a 4 channel 32-bit image, assign one more for the alpha
          if(bits == 32)
            pImageData->data[colorsRead+3] = pColors[3];
          i++;
          rleID--;
          colorsRead += channels;
        }
      }
      else{
        rleID -= 127;
        // Read in the current color, which is the same for a while
        fread(pColors, sizeof(byte) * channels, 1, pFile);
        // Go and read as many pixels as are the same
        while(rleID){
          pImageData->data[colorsRead+0] = pColors[2];
          pImageData->data[colorsRead+1] = pColors[1];
          pImageData->data[colorsRead+2] = pColors[0];
          if(bits == 32)
            pImageData->data[colorsRead+3] = pColors[3];
          i++;
          rleID--;
          colorsRead += channels;
        }
      }
    }
  }
  fclose(pFile);

  // fill in the Image structure to pass back
  pImageData->channels = channels;
  pImageData->sizeX = width;
  pImageData->sizeY = height;

  return pImageData;
}

//decodes the jpeg and fills in the Image structure
void GLESTexture::decodeJPG(jpeg_decompress_struct* cinfo, Image *pImageData){
  //read in the header of the jpeg file
  jpeg_read_header(cinfo, TRUE);
  //decompress	
  jpeg_start_decompress(cinfo);

  pImageData->channels = cinfo->num_components;
  pImageData->sizeX    = cinfo->image_width;
  pImageData->sizeY    = cinfo->image_height;

  int rowSpan = cinfo->image_width * cinfo->num_components;
  pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*rowSpan*pImageData->sizeY));

  unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];
  for (int i = 0; i < pImageData->sizeY; i++)
    rowPtr[i] = &(pImageData->data[i * rowSpan]);

  //read the rows in the image
  int rowsRead = 0;
  while (cinfo->output_scanline < cinfo->output_height){
    rowsRead += jpeg_read_scanlines(cinfo, &rowPtr[rowsRead], cinfo->output_height - rowsRead);
  }
  delete [] rowPtr;

  // Finish decompressing the data
  jpeg_finish_decompress(cinfo);
}

//loads the JPG file and returns it's data in a Image struct
Image* GLESTexture::loadJPG(const char *strFileName){
  struct jpeg_decompress_struct cinfo;
  Image *pImageData = NULL;
  FILE *pFile;
  if((pFile = fopen(strFileName, "rb")) == NULL){
    EXIT2("Unable to load JPG File!");
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
