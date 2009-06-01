#ifndef SIMPLEMAPADAPTER_H
#define SIMPLEMAPADAPTER_H

#include <string>
#include <vector>

#include <math/bbox.h>

#include "MapChunk.h"

class SimpleMapAdapter{
  enum ReadState{
    NODES,
    STREETINFOS,
    STREETS,
    FINISHED,
  };
public:
  void load(std::string filename, MapChunk* map);
protected:
  ReadState readNode(std::istream& strm);
  ReadState readStreetinfo(std::istream& strm);
  ReadState readStreets(std::istream& strm);

  MapChunk* mMap;
  CGE::BBox mLimits;
  std::vector<MapChunk::NodeRef> mNodes;
  std::vector<MapChunk::StreetRef> mStreets;
};

#endif
