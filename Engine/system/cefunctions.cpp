#include <cstdlib>

namespace std{
bool uncaught_exception(){
    return true;
}
}

extern "C"{
char* setlocale(int _Category, const char* _Locale){
  return NULL;
}

time_t mktime(struct tm * _Tm){
  return 0;
}

time_t time(time_t * _Time){
  return 0;
}

struct tm* gmtime(const time_t * _Time){
  return NULL;
}

struct tm* localtime(const time_t * _Time){
  return NULL;
}

size_t strftime(char* _Buf, size_t _SizeInBytes, const char* _Format, const struct tm * _Tm){
  return 0;
}

char* getenv(const char* _VarName){
  return NULL;
}

clock_t clock(void){
  return 0;
}

int rename(const char* _OldFilename, const char* _NewFilename){
  return 0;
}

int remove(const char* _Filename){
  return 0;
}

int system(const char* _Command){
  return 0;
}

int strcoll(const char* _Str1, const char* _Str2){
  return 0;
}

};