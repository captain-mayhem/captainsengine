#ifndef OSMREADER_H
#define OSMREADER_H

#include <list>
#include <string>
#include <utility>

#include <system/types.h>

#include "tinyxml.h"

#include "MapInfo.h"

class OSMMapAdapter;

class OSMReader{
public:
  struct WayInfo{
    enum WayType{
      TYPE_UNKNOWN,
      STREET,
    };
    WayInfo() : waytype(TYPE_UNKNOWN), wayid(0), waynodes(), street() {}
    WayType waytype;
    uint64 wayid;
    std::list<uint64> waynodes;
    StreetInfo street;
  };

  OSMReader(const std::string& filename);
  ~OSMReader();
  void read(OSMMapAdapter* map);
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
  uint64 readULongAttribute(TiXmlElement* element, const char* name);
  double readDoubleAttribute(TiXmlElement* element, const char* name);
  std::pair<std::string,std::string> readTag(TiXmlElement* element);
  
  TiXmlDocument mDoc;
  State mState;
  OSMMapAdapter* mMap;
  WayInfo mCurrWay;
};

#endif
