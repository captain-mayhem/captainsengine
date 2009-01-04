#ifndef OSMREADER_H
#define OSMREADER_H

#include <string>
#include <utility>

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
    NODE,
    WAY,
  };
  void readNode(TiXmlNode* node);
  bool processElement(TiXmlNode* node);
  bool processNode(TiXmlNode* node);
  bool processWay(TiXmlNode* node);
  template <typename T> void readAttribute(TiXmlElement* element, const char* name, T& value);
  std::pair<std::string,std::string> readTag(TiXmlElement* element);
  
  TiXmlDocument mDoc;
  State mState;
  MapChunk* mMap;
  int mCurrStreet;
  int mLastNode;
  //std::list<int> mWay;
};

#endif
