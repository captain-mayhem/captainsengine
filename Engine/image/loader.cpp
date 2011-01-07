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
#include <extern/gif/gif_lib.h>
}

#include "image.h"

using namespace CGE;

Image* ImageLoader::load(void* memory, unsigned size, Type t){
  switch(t){
    case JPG:
      return loadJPG(memory, size);
    case GIF:
      return loadGIF(memory, size);
  }
  return NULL;
}

ImageLoader::Type ImageLoader::determineType(std::string filename){
  int pos = filename.find_last_of('.');
  std::string ext = toLower(filename.substr(pos+1));
  if (ext == "jpg" || ext == "jpeg")
    return JPG;
  else if (ext == "gif")
    return GIF;
  return UNKNOWN;
}

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

static int gif_input_func(GifFileType* giffile, GifByteType* bytes, int length){
  memcpy(bytes, giffile->UserData, length);
  unsigned char* data = (unsigned char*)giffile->UserData;
  data += length;
  giffile->UserData = data;
  return length;
}

Image* ImageLoader::loadGIF(void* memory, unsigned size){
  GifFileType* giffile = DGifOpen(memory, gif_input_func);
  GifRecordType recordtype;
  int extcode;
  GifByteType* extension;
  unsigned char transparent;
  bool hasTransparency;
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
        sprintf(tmpstr, "tmp%i.ppm", ++imgcount);
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
    }
  } while (recordtype != TERMINATE_RECORD_TYPE);
  return NULL;
}

/*
#define MEMREAD(into, howmany) memcpy(into, current, howmany); current += howmany;
int LZWDecoder(unsigned char* in, unsigned char* out, unsigned char initCodeSize, int alignedWidth, int width, int height, bool interlace);

Image* ImageLoader::loadGIF(void* memory, unsigned size){
  unsigned char* current = (unsigned char*) memory;
  char signature[6];
  MEMREAD(signature, 6);
  if (memcmp(signature, "GIF", 3) != 0){
    CGE::Log << "Cannot load gif-file";
    return NULL;
  }
  struct GIFGCEtag{
    unsigned char blockSize;
    unsigned char packedFields; //    0: Transparent Color Flag
										//    1: User Input Flag
										//  2-4: Disposal Method
    unsigned short delay;
    unsigned char transparent;
  } gifgce;
  bool hasGCE = false;
  struct GIFLSDtag{
    unsigned short screenWidth;
    unsigned short screenHeight;
    unsigned char packedFields; //  0-2: Size of Global Color Table
										            //    3: Sort Flag
										            //  4-6: Color Resolution
										            //    7: Global Color Table Flag
    unsigned char background;
    unsigned char pixelAspectRatio;
  } giflsd;
  MEMREAD(&giflsd, 7);
  CGE::Image* img = new CGE::Image();
  unsigned ctsize = (giflsd.packedFields & 0x7)+1;
  //colormap
  unsigned char* colormap = new unsigned char[(1<<ctsize)*3];
  if (giflsd.packedFields & 0x80){
      MEMREAD(colormap, (1<<ctsize)*3);
  }
  else{
    for (int i = 0; i < 256; ++i){
      memset(colormap+3*i, i, 3);
    }
  }
  do{
    unsigned char flag;
    MEMREAD(&flag, 1);
    if (flag == 0x21){ //extension block
      MEMREAD(&flag, 1);
      if (flag == 0xf9){ //graphic control extension
        MEMREAD(&gifgce, 5);
        MEMREAD(&flag, 1);
        hasGCE = true;
      }
      else{
        unsigned char length;
        MEMREAD(&length, 1);
        current += length;
      }
    }
    else if (flag == 0x2c){
      struct GIFIFtag{
        unsigned short xPos;
        unsigned short yPos;
        unsigned short width;
        unsigned short height;
        unsigned char packedFields;//  0-2: Size of Local Color Table
											//  3-4: (Reserved)
											//    5: Sort Flag
											//    6: Interlace Flag
											//    7: Local Color Table Flag
      } gifid;
      MEMREAD(&gifid, 9);
      bool localColorMap = (gifid.packedFields & 0x80) != 0;
      bool transparency = false;
      if (hasGCE){
        transparency = (gifgce.packedFields & 0x1) > 0;
      }
      int channels = transparency ? 4 : 3;
      img->setFormat(channels, gifid.width, gifid.height);
      img->allocateData();
      if (localColorMap){
        unsigned lctsize = (gifid.packedFields & 0x7)+1;
        MEMREAD(colormap, (1<<lctsize)*3);
      }
      unsigned char initsize;
      MEMREAD(&initsize, 1);

      unsigned char* img_start, *img_end;
      img_start = img_end = current;
      unsigned char data;
      MEMREAD(&data, 1);
      while (data){
        current = img_end+data+1;
        img_end = current;
        MEMREAD(&data, 1);
      }
      current = img_start;
      unsigned char* in = new unsigned char[img_end-img_start+4];
      unsigned char* tmp = in;
      MEMREAD(&data, 1);
      while(data){
        MEMREAD(tmp, data);
        tmp += data;
        MEMREAD(&data, 1);
      }
      unsigned char* out = new unsigned char[img->getWidth()*img->getHeight()];
      LZWDecoder(in, out, initsize, img->getWidth(), img->getWidth(), img->getHeight(), (gifid.packedFields&0x40) > 0);
      
      CGE::Image test(1, img->getWidth(), img->getHeight(), out);
      char tmpstr[32];
      static int imgcount = 1;
      sprintf(tmpstr, "tmp%i.ppm", ++imgcount);
      test.debugWrite(tmpstr);

      for (unsigned i = 0; i < img->getWidth()*img->getHeight(); ++i){
        unsigned char* color = colormap+3*out[i];
        memcpy(img->getData()+channels*i, color, 3);
        if (transparency){
          if (out[i] == gifgce.transparent)
            img->getData()[channels*i+3] = 0x0;
          else
            img->getData()[channels*i+3] = 0xFF;
        }
      }
      delete [] colormap;
      delete [] in;
      delete [] out;
      break;
    }
  } while (current-(unsigned char*)memory < (int)size);
  //img->setFormat(channels, giflsd.screenWidth, giflsd.screenHeight);
  return img;
}

int LZWDecoder(unsigned char* in, unsigned char* out, unsigned char initCodeSize, int alignedWidth, int width, int height, bool interlace){
  	int n;
	int row=0,col=0;				// used to point output if Interlaced
	int nPixels, maxPixels;			// Output pixel counter

	short CodeSize;					// Current CodeSize (size in bits of codes)
	short ClearCode;				// Clear code : resets decompressor
	short EndCode;					// End code : marks end of information

	long whichBit;					// Index of next bit in bufIn
	long LongCode;					// Temp. var. from which Code is retrieved
	short Code;						// Code extracted
	short PrevCode;					// Previous Code
	short OutCode;					// Code to output

	// Translation Table:
	short Prefix[4096];				// Prefix: index of another Code
	unsigned char Suffix[4096];		// Suffix: terminating character
	short FirstEntry;				// Index of first free entry in table
	short NextEntry;				// Index of next free entry in table

	unsigned char OutStack[4097];	// Output buffer
	int OutIndex;					// Characters in OutStack

	int RowOffset;					// Offset in output buffer for current row

	// Set up values that depend on InitCodeSize Parameter.
	CodeSize = initCodeSize+1;
	ClearCode = (1 << initCodeSize);
	EndCode = ClearCode + 1;
	NextEntry = FirstEntry = ClearCode + 2;

	whichBit=0;
	nPixels = 0;
	maxPixels = width*height;
	RowOffset =0;

	while (nPixels<maxPixels) {
		OutIndex = 0;							// Reset Output Stack

		// GET NEXT CODE FROM bufIn:
		// LZW compression uses code items longer than a single byte.
		// For GIF Files, code sizes are variable between 9 and 12 bits 
		// That's why we must read data (Code) this way:
		LongCode=*((long*)(in+whichBit/8));	// Get some bytes from bufIn
		LongCode>>=(whichBit&7);				// Discard too low bits
		Code =(short)(LongCode & ((1<<CodeSize)-1) );	// Discard too high bits
		whichBit += CodeSize;					// Increase Bit Offset

		// SWITCH, DIFFERENT POSIBILITIES FOR CODE:
		if (Code == EndCode)					// END CODE
			break;								// Exit LZW Decompression loop

		if (Code == ClearCode) {				// CLEAR CODE:
			CodeSize = initCodeSize+1;			// Reset CodeSize
			NextEntry = FirstEntry;				// Reset Translation Table
			PrevCode=Code;				// Prevent next to be added to table.
			continue;							// restart, to get another code
		}
		if (Code < NextEntry)					// CODE IS IN TABLE
			OutCode = Code;						// Set code to output.

		else {									// CODE IS NOT IN TABLE:
			OutIndex++;			// Keep "first" character of previous output.
			OutCode = PrevCode;					// Set PrevCode to be output
		}

		// EXPAND OutCode IN OutStack
		// - Elements up to FirstEntry are Raw-Codes and are not expanded
		// - Table Prefices contain indexes to other codes
		// - Table Suffices contain the raw codes to be output
		while (OutCode >= FirstEntry) {
			if (OutIndex > 4096) return 0;
			OutStack[OutIndex++] = Suffix[OutCode];	// Add suffix to Output Stack
			OutCode = Prefix[OutCode];				// Loop with preffix
		}

		// NOW OutCode IS A RAW CODE, ADD IT TO OUTPUT STACK.
		if (OutIndex > 4096) return 0;
		OutStack[OutIndex++] = (unsigned char) OutCode;

		// ADD NEW ENTRY TO TABLE (PrevCode + OutCode)
		// (EXCEPT IF PREVIOUS CODE WAS A CLEARCODE)
		if (PrevCode!=ClearCode) {
			Prefix[NextEntry] = PrevCode;
			Suffix[NextEntry] = (unsigned char) OutCode;
			NextEntry++;

			// Prevent Translation table overflow:
			if (NextEntry>=4096) return 0;
      
			// INCREASE CodeSize IF NextEntry IS INVALID WITH CURRENT CodeSize
			if (NextEntry >= (1<<CodeSize)) {
				if (CodeSize < 12) CodeSize++;
				else {}				// Do nothing. Maybe next is Clear Code.
			}
		}

		PrevCode = Code;

		// Avoid the possibility of overflow on 'bufOut'.
		if (nPixels + OutIndex > maxPixels) OutIndex = maxPixels-nPixels;

		// OUTPUT OutStack (LAST-IN FIRST-OUT ORDER)
		for (n=OutIndex-1; n>=0; n--) {
			if (col==width)						// Check if new row.
			{
				if (interlace) {				// If interlaced::
					     if ((row&7)==0) {row+=8; if (row>=height) row=4;}
					else if ((row&3)==0) {row+=8; if (row>=height) row=2;}
					else if ((row&1)==0) {row+=4; if (row>=height) row=1;}
					else row+=2;
				}
				else							// If not interlaced:
					row++;

				RowOffset=row*alignedWidth;		// Set new row offset
				col=0;
			}
			out[RowOffset+col]=OutStack[n];	// Write output
			col++;	nPixels++;					// Increase counters.
		}

	}	// while (main decompressor loop)

	return whichBit;
}
*/
