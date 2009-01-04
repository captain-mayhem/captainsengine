
#include "GeoGen.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <mesh/mesh.h>

System::Event GeoGen::mEvent;
System::Mutex GeoGen::mMutex;
std::queue<std::pair<MapChunk*,MapChunk::Node*> > GeoGen::mQ;

void GeoGen::generateGeometry(void* data){
  while(true){
    mEvent.wait();
    mMutex.lock();
    while(!mQ.empty()){
      MapChunk* map = mQ.front().first;
      MapChunk::Node* node =  mQ.front().second;
      mMutex.unlock();
      GeoGen gg(map, node);
      mMutex.lock();
      mQ.pop();
    }
    mMutex.unlock();
  }
}

void GeoGen::addJob(MapChunk* map, MapChunk::Node* node){
  mMutex.lock();
  mQ.push(std::pair<MapChunk*,MapChunk::Node*>(map,node));
  mEvent.signal();
  mMutex.unlock();
}

GeoGen::GeoGen(MapChunk* map, MapChunk::Node* node) : mCurMap(map), mLastIdx(0){
  mCurMap = map;
  mMesh = new MeshGeo::Mesh();
  genStreets(node);
  //for (iter = node->preds_.begin(); iter != node->preds_.end(); ++iter){
  //  MapChunk::Node* nd = (MapChunk::Node*)*iter;
  //  genStreetSegment(node, nd);
  //}
  mMesh->buildVBO();
  MeshGeo::Model* mdl = new MeshGeo::Model(mMesh);
  node->mModel = mdl;
}

GeoGen::~GeoGen(){

}

void GeoGen::genStreets(MapChunk::Node* node){
  int valence = node->succs_.size();
  if (valence == 0){
    //no streets
  }
  else if (valence == 1){
    //street beginning
    MapChunk::Node* nd = (MapChunk::Node*)node->succs_[0];
    Vec3f v = nd->mPos - node->mPos;
    v.normalize();
    Vec3f side = v.cross(mCurMap->getNormal());
    genStreetSegment(node->mPos,nd->mPos,side);
  }
  else if (valence == 2){
    //normal street
     MapChunk::Node* nd1 = (MapChunk::Node*)node->succs_[0];
     MapChunk::Node* nd2 = (MapChunk::Node*)node->succs_[1];
     Vec3f v1 = nd1->mPos - node->mPos;
     Vec3f v2 = nd2->mPos - node->mPos;
     v1.normalize();
     v2.normalize();
     Vec3f axis = v1.cross(v2);
     float angle = acos(v1.dot(v2));
     Math::Matrix rot = Math::Matrix(Matrix::Rotation,axis,-angle/2);
     Vec3f side = rot*v2;
     side.normalize();
     side = side/sin(angle/2);
     genStreetSegment(node->mPos, nd1->mPos, side);
     genStreetSegment(node->mPos, nd2->mPos, side);
  }
  else if (valence == 3){
    //
  }
  else if (valence == 4){
    //crossroads
  }
  else{
    System::Log << "GeoGen: Valence" << valence << "\n";
    //TODO
  }
  //std::vector<Common::GraphNode*>::iterator iter;
  //for (iter = node->succs_.begin(); iter != node->succs_.end(); ++iter){
  //  MapChunk::Node* nd = (MapChunk::Node*)*iter;
  //  genStreetSegment(node, nd);
  //}
}

void GeoGen::genStreetSegment(Vec3f start, Vec3f end, Vec3f side){
  Vec3f startmiddle = start;
  Vec3f streetdir = end - start;
  float segmentlength = streetdir.length()/2;
  streetdir.normalize();
  Vec3f endmiddle = startmiddle+streetdir*segmentlength;
  Vec3f updir = mCurMap->getNormal();
  Vec3f sidedir = updir.cross(streetdir);
  
  float streetthickness = 3;
  Vec3f p1 = startmiddle-side*streetthickness;
  Vec3f p2 = startmiddle+side*streetthickness;
  Vec3f p3 = endmiddle+sidedir*streetthickness;
  Vec3f p4 = endmiddle-sidedir*streetthickness;
  mMesh->addVertex(p1);
  mMesh->addVertex(p2);
  mMesh->addVertex(p3);
  mMesh->addVertex(p4);
  if (mCurMap->getNormal().dot(Plane(p3,p2,p1).getNormal()) > 0){
    mMesh->addTriangle(mLastIdx+2,mLastIdx+1,mLastIdx+0);
    if (mCurMap->getNormal().dot(Plane(p1,p4,p3).getNormal()) > 0){
      mMesh->addTriangle(mLastIdx+3,mLastIdx+2,mLastIdx+0);
    }
    else{
      mMesh->addTriangle(mLastIdx+0,mLastIdx+2,mLastIdx+3);
    }
  }
  else{
    mMesh->addTriangle(mLastIdx+0,mLastIdx+1,mLastIdx+2);
    if (mCurMap->getNormal().dot(Plane(p2,p4,p3).getNormal()) > 0){
      mMesh->addTriangle(mLastIdx+1,mLastIdx+3,mLastIdx+2);
    }
    else{
      mMesh->addTriangle(mLastIdx+2,mLastIdx+3,mLastIdx+1);
    }
  }
  mLastIdx+=4;

  /*float streetthickness = 3;
  mMesh->addVertex(start-side*streetthickness);
  mMesh->addVertex(start+side*streetthickness);
  mMesh->addVertex(end+side*streetthickness);
  mMesh->addVertex(end-side*streetthickness);
  mMesh->addTriangle(mLastIdx,mLastIdx+1,mLastIdx+2);
  mMesh->addTriangle(mLastIdx+2,mLastIdx+3,mLastIdx);
  mLastIdx+=4;*/
}
