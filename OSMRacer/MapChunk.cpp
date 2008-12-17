
#include "MapChunk.h"

MapChunk::MapChunk(){

}

MapChunk::~MapChunk(){

}

void MapChunk::setDimensions(const Math::Vec3d& center, const Math::Vec3d& range){
  mTree.init(center, range);
}

void MapChunk::addStreetNode(double x, double y, double z, int id){
  mTree.at(x,y,z) = id;
}

void MapChunk::render(){
  mTree.renderDebug();
}