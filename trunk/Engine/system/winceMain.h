
#ifndef WINCE_MAIN_H
#define WINCE_MAIN_H

#ifdef UNDER_CE

#include <Windows.h>

int main(int argv, char* argc[]);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE oldinstance, LPTSTR cmdline, int cmdShow){
  char* argv[2];
  argv[0] = "Captains Game Engine";
  char tmp[1024];
  wcstombs(tmp, cmdline, 1024);
  argv[1] = tmp;
  main(2, argv);
}

#endif

#endif