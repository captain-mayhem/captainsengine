//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: common.hh                                                  |
//  ==================================================================

#ifndef COMMON_HH
#define COMMON_HH

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>


//maximum path length
#ifndef WIN32
#define MAX_PATH 1024
#endif

//progam version
#define VERSION 0.5
#define VERSTRING "0.5"

//! width of the virtual screen
const int SCREENWIDTH = 1024;
//! height of the virtual screen
const int SCREENHEIGHT = 768;

//! waits a short period to avoid buffer overflow
inline void wait(){
  //SDL_Delay(3);
}

//! provides simple geometric forms
/*! Currently only cube needed
 */
class Forms{
  public:
    //! draws a textured cube using vertex arrays
    /*! if no texture coordinates are given, the texture
     *  is mapped one each side once
     */
    static void drawCube(float* texCoords=NULL);

    //! draws a unit textured cube using vertex arrays
    /*! if no texture coordinates are given, the texture
     *  is mapped one each side once
     */
    static void drawCube2(float* texCoords=NULL);

};

#endif
