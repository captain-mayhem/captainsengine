#include <cstdio>
#include <time.h>
#include <emscripten.h>

using namespace std;

extern "C"{
  
void fsMounted(){
  printf("FS mounted\n");
  // open to append
  FILE* file = fopen("/IDBFS/test.txt", "a");

  if (file){
      printf("writing to file\n");

      time_t rawtime;
      struct tm * timeinfo;

      time ( &rawtime );
      timeinfo = localtime ( &rawtime );
      fprintf ( file, "time:%s", asctime (timeinfo) );

      fclose(file);

      EM_ASM(     
          FS.syncfs(false,function (err) { // sync TO backing store
              assert(!err);
              ccall('fileWritten', 'v', [], []);
          });
      );

  }
}

void fileWritten(){
  printf("file written\n");
    EM_ASM(     
        FS.syncfs(true,function (err) { // sync FROM backing store
            assert(!err);
            ccall('fileSynced', 'v', [], []);
        });
    );  
}

void fileSynced(){
  printf("reading file\n");
  char c;
  FILE* file = fopen("/IDBFS/test.txt", "r");
  while ((c = getc(file)) != EOF)
      printf("%c",c);
  fclose(file);
}

}

int main(int argc, char** argv){
  printf("start main\n");
  
  EM_ASM(
    FS.mkdir('/IDBFS');
    FS.mount(IDBFS, {}, '/IDBFS');
    FS.syncfs(true, function (err) {
            assert(!err);
            ccall('fsMounted', 'v', [], []);
        }); // sync FROM backing store
  );
  
  emscripten_exit_with_live_runtime();
  printf("something went wrong\n");
}
