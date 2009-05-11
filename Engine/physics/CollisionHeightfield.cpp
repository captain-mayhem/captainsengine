
#include "CollisionHeightfield.h"

using namespace CGE;

CollisionHeightfield::CollisionHeightfield(){
}

CollisionHeightfield::~CollisionHeightfield(){
  dHeightfieldDataID id = dGeomHeightfieldGetHeightfieldData(mGeom);
  dGeomHeightfieldDataDestroy(id);
}

void CollisionHeightfield::render(const CGE::Camera& cam){

}

void CollisionHeightfield::init(int16 width, int16 depth, float widthscale, float depthscale, uint8* heightdata, float heightscale){
  dHeightfieldDataID heightfield = dGeomHeightfieldDataCreate();
  dGeomHeightfieldDataBuildByte(heightfield, heightdata, 0, width*widthscale, depth*depthscale, width, depth, heightscale, 0, 1000, 0);
  mGeom = dCreateHeightfield(0, heightfield, 1);
  dGeomSetData(mGeom, this);
}
