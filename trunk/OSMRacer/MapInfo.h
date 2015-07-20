#ifndef MAP_INFO_H
#define MAP_INFO_H

#include <common/RefCountedObject.h>

struct StreetInfo : public CGE::RefCountedObject{
  enum Type{
    MOTORWAY,
    FEDERAL_STREET,
    COUNTY_STREET,
    BIG_LOCAL_STREET,
    MEDIUM_LOCAL_STREET,
    SMALL_LOCAL_STREET
  };
  StreetInfo() : streettype(MEDIUM_LOCAL_STREET), lanes(1), oneway(false) {}
  StreetInfo(const StreetInfo& s) : RefCountedObject(s), streettype(s.streettype), lanes(s.lanes), oneway(s.oneway), name(std::string(name)){
  }
  Type streettype;
  uint8 lanes;
  bool oneway;
  std::string name;
};

#endif
