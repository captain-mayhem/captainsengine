
#include "SimpleMapAdapter.h"

#include <fstream>
#include <sstream>

#include <math/vector.h>

//#include "MapChunk.h"

using namespace CGE;

void SimpleMapAdapter::load(std::string filename, MapChunk* map){
  mMap = map;
  std::ifstream in(filename.c_str());
  std::string line;
  int state = NODES;
  while (in){
    getline(in, line);
    std::istringstream strm(line);
    switch(state){
      case NODES:
        state = readNode(strm);
        break;
      case STREETINFOS:
        state = readStreetinfo(strm);
        break;
      case STREETS:
        state = readStreets(strm);
        break;
      case FINISHED:
        map->setExtent(mLimits);
        break;
    }
  }
  in.close();
}

SimpleMapAdapter::ReadState SimpleMapAdapter::readNode(std::istream& strm){
  int id;
  strm >> id;
  if (id == -1)
    return STREETINFOS;
  Vec3f point;
  strm >> point.x; strm >> point.z; 
  if (!(strm >> point.y))
    point.y = 0;
  if ((unsigned)id >= mNodes.size())
    mNodes.resize(id+1);
  mLimits.addPoint(point);
  mNodes[id] = mMap->addNode(point);
  return NODES;
}

SimpleMapAdapter::ReadState SimpleMapAdapter::readStreetinfo(std::istream& strm){
  int id;
  strm >> id;
  if (id == -1)
    return STREETS;
  if ((unsigned)id >= mStreets.size())
    mStreets.resize(id+1);
  StreetInfo info;
  unsigned tmp;
  strm >> tmp;
  info.streettype = (StreetInfo::Type)tmp;
  strm >> tmp;
  info.lanes = static_cast<uint8>(tmp);
  strm >> info.oneway;
  strm >> info.name;
  mStreets[id] = mMap->addStreet(info);
  return STREETINFOS;
}

SimpleMapAdapter::ReadState SimpleMapAdapter::readStreets(std::istream& strm){
  int id;
  strm >> id;
  if (id == -1)
    return FINISHED;
  int from, to;
  strm >> from;
  while (strm >> to){
    mMap->addStreetSegment(mNodes[from], mNodes[to], mStreets[id]);
    from = to;
  }
  return STREETS;
}
