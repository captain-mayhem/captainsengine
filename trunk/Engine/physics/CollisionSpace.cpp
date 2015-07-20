
#include "CollisionSpace.h"

#include "CollisionObject.h"

using namespace CGE;

std::map<dSpaceID,CollisionSpace*> CollisionSpace::mSpaceMap;

CollisionSpace::CollisionSpace(CollisionSpace* parent, Type t, const Vec3f& center, const Vec3f& extents, const int depth){
  dSpaceID prnt = 0;
  if (parent)
    prnt = parent->mSpace;
  switch (t){
    case Hash:
      mSpace = dHashSpaceCreate(prnt);
      break;
    case Simple:
      mSpace = dSimpleSpaceCreate(prnt);
      break;
    case QuadTree:{
      dVector3 cntr;
      cntr[0] = center.x; cntr[1] = center.y; cntr[2] = center.z;
      dVector3 ext;
      ext[0] = center.x; ext[1] = center.y; ext[2] = center.z;
      mSpace = dQuadTreeSpaceCreate(prnt, cntr, ext, depth);
      }
      break;
  }
  dSpaceSetCleanup(mSpace, 0);
  mSpaceMap[mSpace] = this;
}

CollisionSpace::~CollisionSpace(){
  mSpaceMap.erase(mSpace);
  dSpaceDestroy(mSpace);
}

void CollisionSpace::add(CollisionObject* object){
  dSpaceAdd(mSpace, object->mGeom);
}