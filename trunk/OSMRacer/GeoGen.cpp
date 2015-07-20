
#include "GeoGen.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <mesh/mesh.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

using namespace CGE;

CGE::EventQueue<std::pair<MapChunk*,Ptr<MapChunk::Node> > > GeoGen::mQueue;
CGE::EventQueue<std::pair<Ptr<MapChunk::Node> ,MeshGeo::Model* > > GeoGen::mAnswer;

void GeoGen::generateGeometry(void* data){
  while(true){
    mQueue.waitProcess(process);
  }
}

void GeoGen::process(std::pair<MapChunk*,Ptr<MapChunk::Node> > val){
  MapChunk* map = val.first;
  Ptr<MapChunk::Node>  node =  val.second;
  GeoGen gg(map, node);
}

void GeoGen::useGeometry(){
  mAnswer.tryProcess(processAnswer);
}

void GeoGen::processAnswer(std::pair<Ptr<MapChunk::Node> ,MeshGeo::Model*> val){
  val.second->getMesh()->buildVBO();
  //val.first->mModel = val.second;
}

void GeoGen::addJob(MapChunk* map, Ptr<MapChunk::Node>  node){
  mQueue.enqueue(std::pair<MapChunk*,Ptr<MapChunk::Node> >(map,node));
}

GeoGen::GeoGen(MapChunk* map, Ptr<MapChunk::Node>  node) : mCurMap(map), mLastIdx(0){
  mCurMap = map;
  mMesh = new MeshGeo::Mesh();
  genStreets(node);
  
  mMesh->buildVBO();
  MeshGeo::Model* mdl = new MeshGeo::Model(mMesh);
  //node->mModel = mdl;
  mAnswer.enqueue(std::pair<Ptr<MapChunk::Node> ,MeshGeo::Model*>(node,mdl));
}

GeoGen::~GeoGen(){

}

void GeoGen::genStreets(Ptr<MapChunk::Node>  node){
  reorderGraph(node);
  int valence = node->succs_.size();
  if (valence == 0 /*|| valence > 3*/){
    //no streets
  }
  else if (valence == 1){
    //street beginning
    Ptr<MapChunk::Node> nd = (Ptr<MapChunk::Node>)node->succs_[0];
    Vec3f v = nd->mPos - node->mPos;
    v.normalize();
    Vec3f side = v.cross(mCurMap->getNormal());
    side.normalize();
    genStreetSegment(node->mPos,nd->mPos,side*-1,side);
  }
  else{
    Ptr<MapChunk::Node>  first = (Ptr<MapChunk::Node> )node->succs_[node->succs_.size()-1];
    Vec3f v1 = first->mPos - node->mPos;
    v1.normalize();
    Ptr<MapChunk::Node>  second = (Ptr<MapChunk::Node> )node->succs_[0];
    Vec3f v2 = second->mPos - node->mPos;
    v2.normalize();
    for (unsigned i = 1; i < node->succs_.size()+1; ++i){
      Ptr<MapChunk::Node>  third;
      if (i >= node->succs_.size())
        third = (Ptr<MapChunk::Node> )node->succs_[0];
      else
        third = (Ptr<MapChunk::Node> )node->succs_[i];
      Vec3f v3 = third->mPos - node->mPos;
      v3.normalize();

      float leftangle = getAngle(v1,v2,mCurMap->getNormal())/2;
      CGE::Matrix rot = CGE::Matrix(Matrix::Rotation,mCurMap->getNormal(),leftangle);
      Vec3f leftside = rot*v1;
      leftside.normalize();
      leftside = leftside/sin(leftangle);

      float rightangle = getAngle(v2,v3,mCurMap->getNormal())/2;
      CGE::Matrix rot2 = CGE::Matrix(Matrix::Rotation,mCurMap->getNormal(),rightangle);
      Vec3f rightside = rot2*v2;
      rightside.normalize();
      rightside = rightside/sin(rightangle);

      genStreetSegment(node->mPos, second->mPos, rightside, leftside);
      //genStreetSegment(node->mPos, second->mPos, side);
      first = second;
      v1 = v2;
      second = third;
      v2 = v3;
    }
    /*Ptr<MapChunk::Node>  third = (Ptr<MapChunk::Node> )node->succs_[0];
    Vec3f v3 = third->mPos - node->mPos;
    v3.normalize();
    Vec3f axis = v2.cross(v1);
    float angle = getAngle(v2,v1,mCurMap->getNormal());
    CGE::Matrix rot = CGE::Matrix(Matrix::Rotation,axis,angle/2);
    Vec3f side = rot*v2;
    side.normalize();
    side = side/sin(angle/2);

    Vec3f axis2 = v2.cross(v3);
    float angle2 = getAngle(v2,v3,mCurMap->getNormal());
    CGE::Matrix rot2 = CGE::Matrix(Matrix::Rotation,axis2,angle2/2);
    Vec3f side2 = rot2*v2;
    side2.normalize();
    side2 = side2/sin(angle2/2);

    genStreetSegment(node->mPos, second->mPos, side, side2);*/
  }
  /*
  else if (valence == 2){
    //normal street
     Ptr<MapChunk::Node>  nd1 = (Ptr<MapChunk::Node> )node->succs_[0];
     Ptr<MapChunk::Node>  nd2 = (Ptr<MapChunk::Node> )node->succs_[1];
     Vec3f v1 = nd1->mPos - node->mPos;
     Vec3f v2 = nd2->mPos - node->mPos;
     v1.normalize();
     v2.normalize();
     Vec3f axis = v1.cross(v2);
     float angle = acos(v1.dot(v2));
     CGE::Matrix rot = CGE::Matrix(Matrix::Rotation,axis,-angle/2);
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
    CGE::Log << "GeoGen: Valence" << valence << "\n";
    //TODO
  }*/
  //std::vector<Common::GraphNode*>::iterator iter;
  //for (iter = node->succs_.begin(); iter != node->succs_.end(); ++iter){
  //  Ptr<MapChunk::Node>  nd = (Ptr<MapChunk::Node> )*iter;
  //  genStreetSegment(node, nd);
  //}
}

void GeoGen::genStreetSegment(Vec3f start, Vec3f end, Vec3f rightside, Vec3f leftside){
  Vec3f startmiddle = start;
  Vec3f streetdir = end - start;
  float segmentlength = streetdir.length()/2;
  streetdir.normalize();
  Vec3f endmiddle = startmiddle+streetdir*segmentlength;
  Vec3f updir = mCurMap->getNormal();
  Vec3f sidedir = updir.cross(streetdir); //points to right side (right-handed coord sys)
  
  float streetthickness = 3;
  Vec3f startright = startmiddle+rightside*streetthickness;
  Vec3f startleft = startmiddle+leftside*streetthickness;
  Vec3f endright = endmiddle+sidedir*streetthickness;
  Vec3f endleft = endmiddle-sidedir*streetthickness;
  mMesh->addVertex(startmiddle);
  mMesh->addVertex(startleft);
  mMesh->addVertex(endleft);
  mMesh->addVertex(endmiddle);
  mMesh->addVertex(endright);
  mMesh->addVertex(startright);
  mMesh->addTriangle(mLastIdx+0,mLastIdx+1,mLastIdx+2);
  mMesh->addTriangle(mLastIdx+0,mLastIdx+2,mLastIdx+3);
  mMesh->addTriangle(mLastIdx+0,mLastIdx+3,mLastIdx+4);
  mMesh->addTriangle(mLastIdx+0,mLastIdx+4,mLastIdx+5);
  /*if (mCurMap->getNormal().dot(Plane(p3,p2,p1).getNormal()) > 0){
    mMesh->addTriangle(mLastIdx+2,mLastIdx+1,mLastIdx+0);
    if (mCurMap->getNormal().dot(Plane(p1,p4,p3).getNormal()) > 0){
      mMesh->addTriangle(mLastIdx+3,mLastIdx+2,mLastIdx+0);
    }
    else{
      mMesh->addTriangle(mLastIdx+0,mLastIdx+2,mLastIdx+3);
    }
  }
  else{*/
    ////////mMesh->addTriangle(mLastIdx+0,mLastIdx+1,mLastIdx+2);
    //if (mCurMap->getNormal().dot(Plane(p2,p4,p3).getNormal()) > 0){
      ////////mMesh->addTriangle(mLastIdx+1,mLastIdx+3,mLastIdx+2);
    //}
    //else{
      //mMesh->addTriangle(mLastIdx+2,mLastIdx+3,mLastIdx+1);
    //}
  //}
  mLastIdx+=6;

  /*float streetthickness = 3;
  mMesh->addVertex(start-side*streetthickness);
  mMesh->addVertex(start+side*streetthickness);
  mMesh->addVertex(end+side*streetthickness);
  mMesh->addVertex(end-side*streetthickness);
  mMesh->addTriangle(mLastIdx,mLastIdx+1,mLastIdx+2);
  mMesh->addTriangle(mLastIdx+2,mLastIdx+3,mLastIdx);
  mLastIdx+=4;*/
}

static int compare(const void* a, const void* b){
  std::pair<float,Ptr<MapChunk::Node> >* pa = (std::pair<float,Ptr<MapChunk::Node> >*)a;
  std::pair<float,Ptr<MapChunk::Node> >* pb = (std::pair<float,Ptr<MapChunk::Node> >*)b;
  if (pa < pb)
    return -1;
  if (pa > pb)
    return 1;
  return 0;
}

void GeoGen::reorderGraph(Ptr<MapChunk::Node>  node){
  if (node->succs_.empty())
    return;
  std::pair<float,Ptr<MapChunk::Node> >* nodes = new std::pair<float,Ptr<MapChunk::Node> >[node->succs_.size()];
  Ptr<MapChunk::Node>  first = (Ptr<MapChunk::Node> )node->succs_[0];
  Vec3f v1 = first->mPos - node->mPos;
  v1.normalize();
  nodes[0] = std::pair<float,Ptr<MapChunk::Node> >(0,first);
  //generate angles
  for (unsigned i = 1; i < node->succs_.size(); ++i){
    Ptr<MapChunk::Node>  second = (Ptr<MapChunk::Node> )node->succs_[i];
    Vec3f v2 = second->mPos - node->mPos;
    v2.normalize();
    float angle = getAngle(v1,v2,mCurMap->getNormal());
    nodes[i] = std::pair<float,Ptr<MapChunk::Node> >(angle,second);
  }
  qsort(nodes, node->succs_.size(), sizeof(std::pair<float,Ptr<MapChunk::Node> >), compare);
  for (unsigned i = 1; i < node->succs_.size(); ++i){
    node->succs_[i] = nodes[i].second;
  }
}

float GeoGen::getAngle(const Vec3f& v1, const Vec3f& v2, const Vec3f& normal){
  float angle = acos(v1.dot(v2));
  Vec3f axis = v1.cross(v2);
  if (axis.dot(normal) < 0){
    angle = (float)(2*M_PI-angle);
  }
  return angle;
}
