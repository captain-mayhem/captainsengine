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

#include "SDL.h"
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

using std::cerr;
using std::endl;
using std::string;
using std::ofstream;

//maximum path length
#ifndef WIN32
#define MAX_PATH 1024
#endif

//error
#define Error(A) {cerr << A << endl;}
//fatal error
#define Fat_Error(A) {cerr << A << endl; exit(-1);}
//log
extern ofstream Log;

//progam version
#define VERSION 0.5
#define VERSTRING "0.5"

//! width of the virtual screen
const int SCREENWIDTH = 1024;
//! height of the virtual screen
const int SCREENHEIGHT = 768;

//! changes an integer to a string
inline string toStr(const int i){
  char tmp[16];
  memset(tmp, 0, 16);
  sprintf(tmp, "%d", i);
  return string(tmp);
}

//! changes a string to an integer
inline const int toInt(const string& s){
  return atoi(s.c_str());
}

//! changes a string to lowercase
inline const string toLower(string& s){
  for (unsigned i = 0; i < s.length(); i++){
    s.at(i) = tolower(s.at(i));
  }
  return s;
}

//! waits a short period to avoid buffer overflow
inline void wait(){
  SDL_Delay(3);
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
