
#include "CollisionObject.h"

using namespace CGE;

CollisionObject::CollisionObject() : mGeom(NULL){
}

CollisionObject::~CollisionObject(){
  dGeomDestroy(mGeom);
}

void CollisionObject::setPosition(const Vec3f& pos){
  dGeomSetPosition(mGeom, pos.x, pos.y, pos.z);
}

Vec3f CollisionObject::getPosition(){
  const dReal* tmp = dGeomGetPosition(mGeom);
  return Vec3f((float)tmp[0],(float)tmp[1],(float)tmp[2]);
}

void CollisionObject::setOrientation(const Matrix& orientation){
  dMatrix3 mat;
  mat[0] = orientation.at(0);
  mat[1] = orientation.at(4);
  mat[2] = orientation.at(8);
  mat[3] = orientation.at(12);
  mat[4] = orientation.at(1);
  mat[5] = orientation.at(5);
  mat[6] = orientation.at(9);
  mat[7] = orientation.at(13);
  mat[8] = orientation.at(2);
  mat[9] = orientation.at(6);
  mat[10] = orientation.at(10);
  mat[11] = orientation.at(14);
  dGeomSetRotation(mGeom, mat);
}

Matrix CollisionObject::getOrientation(){
  const dReal* tmp = dGeomGetRotation(mGeom);
  Matrix mat = Matrix(Matrix::Identity);
  mat.at(0) = (float)tmp[0];
  mat.at(4) = (float)tmp[1];
  mat.at(8) = (float)tmp[2];
  mat.at(12) = (float)tmp[3];
  mat.at(1) = (float)tmp[4];
  mat.at(5) = (float)tmp[5];
  mat.at(9) = (float)tmp[6];
  mat.at(13) = (float)tmp[7];
  mat.at(2) = (float)tmp[8];
  mat.at(6) = (float)tmp[9];
  mat.at(10) = (float)tmp[10];
  mat.at(14) = (float)tmp[11];
  return mat;
}
