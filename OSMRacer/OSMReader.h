#include <string>
//#include <list>

#include "tinyxml.h"

class MapChunk;

class OSMReader{
public:
  OSMReader(const std::string& filename);
  ~OSMReader();
  void read(MapChunk* map);
protected:
  enum State{
    START,
    OSM,
    WAY,
  };
  void readNode(TiXmlNode* node);
  bool processElement(TiXmlNode* node);
  bool processWay(TiXmlNode* node);
  template <typename T> void readAttribute(TiXmlElement* element, const char* name, T& value);
  
  TiXmlDocument mDoc;
  State mState;
  MapChunk* mMap;
  //std::list<int> mWay;
};