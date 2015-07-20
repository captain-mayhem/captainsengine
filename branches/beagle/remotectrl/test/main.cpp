
#include <iostream>

#include "datapool.hxx"

DataPool thePool;

class MyDataPoolListener : public DataPoolListener{
  virtual void eventOccurred(DataPoolEntry entry, DataPoolItem item){
    int command;
    switch(entry){
      case DP_COMMAND:
        item.read(command);
        std::cout << command << std::endl;
    }
  }
};

int main(int argc, char** argv){
  MyDataPoolListener listy;
  thePool.registerOnUpdate(DP_COMMAND, &listy);
  thePool.put(DP_COMMAND, 5);
  thePool.put(DP_COMMAND, 7);
  thePool.put(DP_DEBUG, "the test");
  std::vector<std::string> playlist;
  playlist.push_back("Song A");
  playlist.push_back("I don't know");
  thePool.put(DP_PLAYLIST, playlist);
  
  listy.processEvents();

  int command;
  thePool.get(DP_COMMAND).read(command);
  std::vector<std::string> plist;
  thePool.get(DP_PLAYLIST).read(plist);
  return 0;
}
