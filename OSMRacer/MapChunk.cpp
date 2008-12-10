
#include "MapChunk.h"

MapChunk::MapChunk(){

}

MapChunk::~MapChunk(){

}

void MapChunk::addStreetNode(double x, double y, double z, int id){
  mTree.at(x,y,z) = id;
}