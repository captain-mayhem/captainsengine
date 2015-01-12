//  ==================================================================
// | Medalyvis                                                        |
// |------------------------------------------------------------------|
// | Medical Data Analysis and Visualization                          |
// |------------------------------------------------------------------|
// | (c) Copyright Marco Staginski 2006-2007                          |
// |------------------------------------------------------------------|
// | File: MDAVmesh.cpp                                               |
// | Created on: 15/03/2007                                           |
// |------------------------------------------------------------------|
// | Description:                                                     |
// | A render component for rendering meshes                          |
//  ==================================================================

#include "mesh.h"

#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/vertexbuffer.h"
#include "../math/ray.h"
#include "../system/file.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <float.h>
#include <algorithm>

using std::min;
using namespace CGE;

// --------------------------------------------------------------------
//  Default constructor
// --------------------------------------------------------------------
Mesh::Mesh() : mComputeEdges(true){
  numTriangles_ = 0;
  numVertices_ = 0;
  numTexCoords_ = 0;
  mNumNormals = 0;
  name_ = "none";
  filename_ = "none";
  m_visible = true;
  m_light = true;
  for (int i = 0; i < 4; i++){
    m_color[i] = 1.0;
  }
  vb_ = NULL;
  mIB = NULL;
  min_ = Vector3D(FLT_MAX,FLT_MAX,FLT_MAX);
  max_ = Vector3D(FLT_MIN,FLT_MIN,FLT_MIN);
  center_ = Vector3D();
}

// --------------------------------------------------------------------
//  Destructor
// --------------------------------------------------------------------
Mesh::~Mesh(){
  //glDeleteBuffersARB(1, &m_vbo);
  //glDeleteBuffersARB(1, &m_vboidx);
  SAFE_DELETE(vb_);
  SAFE_DELETE(mIB);
  for (unsigned i = 0; i < triangles_.size(); i++){
    delete triangles_[i];
    triangles_[i] = NULL;
  }
  triangles_.clear();
  for (unsigned i = 0; i < edges_.size(); i++){
    delete edges_[i];
    edges_[i] = NULL;
  }
  edges_.clear();
  texCoords_.clear();
}


//! add vertex
void Mesh::addVertex(float x, float y, float z){
  addVertex(CGE::Vec3f(x,y,z));
}

//add vertex
void Mesh::addVertex(CGE::Vec3f v){
  vertices_.push_back(Vertex(v.x,v.y,v.z));
  numVertices_++;
  //extent
  if (v.x < min_.x)
    min_.x = v.x;
  if (v.y < min_.y)
    min_.y = v.y;
  if (v.z < min_.z)
    min_.z = v.z;
  if (v.x > max_.x)
    max_.x = v.x;
  if (v.y > max_.y)
    max_.y = v.y;
  if (v.z > max_.z)
    max_.z = v.z;
  //center
  center_ += v;
}

// add texture coordinates
void Mesh::addTexCoord(float x, float y, float z){
  texCoords_.push_back(Vector3D(x,y,z));
  numTexCoords_++;
}

void Mesh::addNormal(float x, float y, float z){
  mComputeEdges = false;
  mNormals.push_back(Vec3f(x, y, z));
  ++mNumNormals;
}

  
//! add edge
Edge* Mesh::addEdge(int v0, int v1){
  Edge* e = vertices_[v0].eList.getEdge(v0,v1);
  if(e)
    return e;

  //insert new edge
  e = new Edge(v0, v1);
  vertices_[v0].eList.insertEdge(e);
  vertices_[v1].eList.insertEdge(e);
  edges_.insertEdge(e);
  
  return e;
}

//! add triangle
Triangle* Mesh::addTriangle(int v0, int v1, int v2, int t0, int t1, int t2, int n0, int n1, int n2){
  Edge *e0 = NULL, *e1 = NULL, *e2 = NULL;
  if (mComputeEdges){
    e0 = vertices_[v1].eList.getEdge(v1, v2);
    if (!e0)
      e0 = addEdge(v1, v2);

    e1 = vertices_[v1].eList.getEdge(v2, v0);
    if (!e1)
      e1 = addEdge(v2, v0);

    e2 = vertices_[v1].eList.getEdge(v0, v1);
    if (!e2)
      e2 = addEdge(v0, v1);
  }
  
  Triangle* tri = new Triangle(v0, v1, v2, t0, t1, t2, n0, n1, n2, e0, e1, e2);
  triangles_.insertTriangle(tri);

  if (mComputeEdges){
    e0->tList.insertTriangle(tri);
    e1->tList.insertTriangle(tri);
    e2->tList.insertTriangle(tri);
  }
  
  numTriangles_++;
  return tri;  
}

//load from file
bool Mesh::loadFromFile(std::string filename){
  filename_ = filename;
  std::string name = Filesystem::getFileComponent(filename);
  std::string ending = Filesystem::getExtension(name);
  name_ = name;
  if (ending == "nofile"){
    addVertex(0.5,0.5,0.5);
    addVertex(1.0,0.5,0.5);
    addVertex(0.75,0.5,1.0);
    addTriangle(0,1,2);
    return true;
  }
  else if (ending == "obj"){
    bool success = loadOBJ(filename);
    return success;
  }
  else if (ending == "off"){
    bool success = loadOFF(filename);
    return success;
  }
  return false;  
}


// load obj-files
bool Mesh::loadOBJ(std::string filename){
  char tester;	
  float x,y,z;
  unsigned int vertexid[4], textureid[4], normalid[4];
  char line[2000];
  
  char* pos;
  FILE* file = fopen(filename.c_str(), "r");
  if (!file)
    return false;
  while (fgets(line, 2000, file)){
    switch (line[0]){
    case 'v':
      if (line[1] == 't'){
        // texture coordinate
        pos = line + 2;
        x = strtof(pos, &pos);
        y = strtof(pos, &pos);
        z = 0.0f;
        //do we have 3D coords?
        tester = pos[1];
        if (tester >= 48 && tester <= 57){
          z = strtof(pos, &pos);
        }
        addTexCoord(x, y, z);
      }
      else if (line[1] == 'n'){
        // normal vector
        pos = line + 2;
        x = strtof(pos, &pos);
        y = strtof(pos, &pos);
        z = strtof(pos, &pos);
        addNormal(x, y, z);
      }
      else{
        pos = line + 1;
        x = strtof(pos, &pos);
        y = strtof(pos, &pos);
        z = strtof(pos, &pos);
        addVertex(x, y, z);
      }
      break;
    case 'f':
      pos = line + 1;
      for (int i = 0; i < 3; ++i){
        vertexid[i] = strtoul(pos, &pos, 10);
        if (pos[0] == '/'){
          // read texture index
          textureid[i] = strtoul(pos + 1, &pos, 10);
        }
        if (pos[0] == '/'){
          // read normal index
          normalid[i] = strtoul(pos + 1, &pos, 10);
        }
      }

      addTriangle(vertexid[0] - 1, vertexid[1] - 1, vertexid[2] - 1, textureid[0] - 1, textureid[1] - 1, textureid[2] - 1, normalid[0] - 1, normalid[1] - 1, normalid[2] - 1);

      //do we have a quad?
      tester = pos[1];
      if (tester >= 48 && tester <= 57){
        vertexid[3] = strtoul(pos, &pos, 10);
        if (pos[0] == '/'){
          // read texture index
          textureid[3] = strtoul(pos + 1, &pos, 10);
        }
        if (pos[0] == '/'){
          // read normal index
          normalid[3] = strtoul(pos + 1, &pos, 10);
        }
        addTriangle(vertexid[0] - 1, vertexid[2] - 1, vertexid[3] - 1, textureid[0] - 1, textureid[2] - 1, textureid[3] - 1, normalid[0] - 1, normalid[2] - 1, normalid[3] - 1);
      }
      break;
    case 'm':
      if (memcmp(line, "mtllib", 6) == 0){
        std::string mtlfile(line + 7);
        if (mtlfile.back() == '\n')
          mtlfile.pop_back();
        string path = Filesystem::getPathComponent(filename);
        mtlfile = Filesystem::combinePath(path, mtlfile);
        break;
      }
      break;
    default:
      break;
    }
  }
  fclose(file);
  return true;
}

// load off-files
bool Mesh::loadOFF(std::string filename){
  std::ifstream file(filename.c_str());
  if (!file)
    return false;

  char string[5];
  file >> string;

  int numV = 0, numT = 0, numE = 0;

  file >> numV >> numT >> numE;
  
  //vertices
  float x,y,z;
  for (int i = 0; i < numV; i++){
    file >> x >> y >> z;
    addVertex(x,y,z);
  }

  //triangles
  int num_vs, v0, v1, v2, v3, v4, v5;
  for (int i = 0; i < numT; i++){
    file >> num_vs;
    if (num_vs == 3){
      file >> v0 >> v1 >> v2;
      addTriangle(v0,v1,v2);
    }
    else if (num_vs == 4){
      file >> v0 >> v1 >> v2 >> v3;
      addTriangle(v0,v1,v2);
      addTriangle(v0,v2,v3);
    }
    else if (num_vs == 5){
      file >> v0 >> v1 >> v2 >> v3 >> v4;
      addTriangle(v0,v1,v2);
      addTriangle(v2,v3,v4);
      addTriangle(v2,v4,v0);
    }
    else if (num_vs == 6){
      file >> v0 >> v1 >> v2 >> v3 >> v4 >> v5;
      addTriangle(v0,v1,v2);
      addTriangle(v2,v3,v4);
      addTriangle(v2,v4,v5);
      addTriangle(v2,v5,v0);
    }
    else if (num_vs > 6){
      int* vs = new int[num_vs];
      for (int i = 0; i < num_vs; i++){
        file >> vs[i];
      }
      addTriangle(vs[0],vs[1],vs[2]);
      for (int i = 0; i < num_vs-4; i++){
        addTriangle(vs[2], vs[i+3], vs[i+4]);
      }
      addTriangle(vs[2],vs[num_vs-1],vs[0]);
      delete [] vs;
    }
  }

  if (numE != 0){
    std::cerr << "This mesh loader is not capable of reading edges";
    file.close();
    return false;
  }

  file.close();
  return true;
}


//! get the normal of a triangle
float* Mesh::getNormal(int i){
  if (!mComputeEdges)
    return NULL;

  Vertex v = vertices_[i];

  Vector3D normal = Vector3D(0,0,0);
  
  for (unsigned int i = 0; i < v.eList.size(); i++){
    for (unsigned int j = 0; j < v.eList[i]->tList.size(); j++){
      Triangle* tri = v.eList[i]->tList[j];
      Vector3D v0(vertices_[tri->v0].data);
      Vector3D v1(vertices_[tri->v1].data);
      Vector3D v2(vertices_[tri->v2].data);
      Vector3D tnorm = (v0-v1).cross(v0-v2);
      normal += tnorm;
    }
  }

  if (normal.magnitudeSquared() < 0.00001)
    normal = Vector3D(0,0,1);

  normal.normalize();

  return normal.data;
}

//! build vertex buffer object
void Mesh::buildVBO(){
  //float* vertices = new float[numTriangles_*3*3];
  //unsigned int* indices = new unsigned int[numTriangles_*3];
  std::map<Vec3i, int> distinctVertices;
  std::vector<float> vdata;
  vdata.reserve(numVertices_*(3 + 3 + 2));
  std::vector<int> idata;
  idata.reserve(numTriangles_ * 3);
  int vidx = 0;
  int numVertices = 0;
  for (int i = 0; i < numTriangles_; i++){
    Triangle* tri = triangles_[i];
    for (int j = 0; j < 3; ++j){
      Vec3i idx(tri->data[j], tri->texdata[j], tri->normdata[j]);
      int di = distinctVertices[idx];
      if (di == 0){
        ++numVertices;
        Vec3f pos = vertices_[tri->data[j]];
        vdata.push_back(pos.x); vdata.push_back(pos.y); vdata.push_back(pos.z);
        if (numTexCoords_ > 0){
          Vec3f tex = texCoords_[tri->texdata[j]];
          vdata.push_back(tex.x); vdata.push_back(tex.y);
        }
        Vec3f norm;
        if (mNumNormals > 0)
          norm = mNormals[tri->normdata[j]];
        else
          norm = getNormal(tri->data[j]);
        vdata.push_back(norm.x); vdata.push_back(norm.y); vdata.push_back(norm.z);
        idata.push_back(vidx);
        distinctVertices[idx] = ++vidx;
      }
      else{
        idata.push_back(di - 1);
      }
    }
  }

  vb_ = CGE::Engine::instance()->getRenderer()->createVertexBuffer();
  if (numTexCoords_ > 0)
    vb_->create(VB_POSITION | VB_NORMAL | VB_TEXCOORD, numVertices);
  else
    vb_->create(VB_POSITION | VB_NORMAL, numVertices);
  void* vdat = vb_->lockVertexPointer();
  memcpy(vdat, vdata.data(), vdata.size()*sizeof(float));
  vb_->unlockVertexPointer();
  /*
  vb_ = CGE::Engine::instance()->getRenderer()->createVertexBuffer();
  if (numTexCoords_ > 0)
    vb_->create(VB_POSITION | VB_NORMAL | VB_TEXCOORD, numTriangles_*3);
  else
    vb_->create(VB_POSITION | VB_NORMAL, numTriangles_*3);
  vb_->lockVertexPointer();
  
  for (int i = 0; i < numTriangles_; i++){
    Triangle* tri = triangles_[i];
    Vector3D v0 = vertices_[tri->v0];
    Vector3D v1 = vertices_[tri->v1];
    Vector3D v2 = vertices_[tri->v2];
    vb_->setPosition(3*i+0, v0);
    vb_->setPosition(3*i+1, v1);
    vb_->setPosition(3*i+2, v2);
    Vector3D n0, n1, n2;
    if (mNumNormals == 0){
      n0 = getNormal(tri->v0);
      n1 = getNormal(tri->v1);
      n2 = getNormal(tri->v2);
    }
    else{
      n0 = mNormals[tri->n0];
      n1 = mNormals[tri->n1];
      n2 = mNormals[tri->n2];
    }
    vb_->setNormal(3*i+0, n0);
    vb_->setNormal(3*i+1, n1);
    vb_->setNormal(3*i+2, n2);
    if (numTexCoords_ > 0){
      Vector3D t0 = texCoords_[tri->t0];
      Vector3D t1 = texCoords_[tri->t1];
      Vector3D t2 = texCoords_[tri->t2];
      vb_->setTexCoord(3*i+0, Vec2f(t0.x,t0.y));
      vb_->setTexCoord(3*i+1, Vec2f(t1.x,t1.y));
      vb_->setTexCoord(3*i+2, Vec2f(t2.x,t2.y));
    }
    //indices[i*3+0] = i*3+0;
    //indices[i*3+1] = i*3+1;
    //indices[i*3+2] = i*3+2;
  }*/
/*
  vb_->unlockVertexPointer();

  short* indi = vb_->lockIndexPointer();
  for (int i = 0; i < numTriangles_*3; i++){
    indi[i] = i;
  }
  vb_->unlockIndexPointer();*/
/*
  glGenBuffersARB(1, &m_vbo);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_numVertices*(3+3)*sizeof(float), vertices, GL_STATIC_DRAW_ARB);

  glGenBuffersARB(1, &m_vboidx);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_vboidx);
  glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_numTriangles*3*sizeof(unsigned int), indices,GL_STATIC_DRAW_ARB);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
*/
  //delete [] vertices;
  //delete [] indices;
  mIB = CGE::Engine::instance()->getRenderer()->createIndexBuffer(IndexBuffer::IB_UINT, idata.size());
  void* data = mIB->lockIndexPointer();
  memcpy(data, idata.data(), idata.size()*sizeof(int));
  mIB->unlockIndexPointer();

  //compute real center
  center_ = center_ / (float)numVertices_;
}

// --------------------------------------------------------------------
//  draw the mesh
// --------------------------------------------------------------------
void Mesh::draw(){/*
  if (!m_visible)
    return;
  if (m_light){
    glEnable(GL_LIGHTING);
    float ambient[4] = { m_color[0]*0.5, m_color[1]*0.5, m_color[2]*0.5, m_color[3]};
    float specular[4] = {0.5,0.5,0.5,m_color[3]};
    float shininess[1] = {20.0};
    //float diffuse[4] = { 0.8, 0.4, 0.4, 1.0 };
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, ambient );
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, m_color );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specular );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, shininess );
    if(shader)
      shader->Texture1i("lighting", 1, 0);
  }
  else{
    if(shader)
      shader->Texture1i("lighting", 0, 0);
    glDisable(GL_LIGHTING);
  }
  glColor4fv(m_color);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_vboidx);
  glVertexPointer(3, GL_FLOAT, (3+3)*sizeof(float), NULL);
  glNormalPointer(GL_FLOAT, (3+3)*sizeof(float), (void*)(3*sizeof(float)));
  glDrawElements(GL_TRIANGLES, m_numTriangles*3, GL_UNSIGNED_INT, NULL);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);*/
  //CGE::Engine::instance()->getRenderer()->multiplyMatrix(trafo_);
  vb_->activate();
  vb_->draw(CGE::VB_Triangles, mIB);
}

//! intersects a ray and a triangle
float Mesh::intersect(const CGE::Ray& r) const{
  float intersection = FLT_MAX;
  //go over all triangles
  for (unsigned i = 0; i < triangles_.size(); i++){
    Vector3D v0 = vertices_[triangles_[i]->v0];
    Vector3D e1 = vertices_[triangles_[i]->v1] - v0;
    Vector3D e2 = vertices_[triangles_[i]->v2] - v0;
    Vector3D p = r.getDirection().cross(e2);
    float a = e1.dot(p);
    if (a > -1e-7 && a < 1e-7)
      continue;
    float f = 1.0f/a;
    Vector3D s = r.getOrigin()-v0;
    float u = f * s.dot(p);
    if (u < 0.0f || u > 1.0f){
      continue;
    }
    Vector3D q = s.cross(e1);
    float v = f * r.getDirection().dot(q);
    if (v < 0.0 || u+v > 1.0f){
      continue;
    }
    float t = f * e2.dot(q);
    if (t >= 0){
      intersection = min(t, intersection);
    }
  }
  return intersection;
}

bool Mesh::getTriangle(int i, Vector3D** result) const{
  if (i >= numTriangles_)
    return false;
  (*result)[0] = vertices_[triangles_[i]->v0];
  (*result)[1] = vertices_[triangles_[i]->v1];
  (*result)[2] = vertices_[triangles_[i]->v2];
  return true;
}

void Mesh::clear(){
  numVertices_ = 0;
  numTriangles_ = 0;
  numTexCoords_ = 0;
  mNumNormals = 0;
  vertices_.clear();
  texCoords_.clear();
  mNormals.clear();
  edges_.clear();
  triangles_.clear();
  indices_.clear();
  name_ = "none";
  filename_ = "none";
  if (vb_)
    SAFE_DELETE(vb_);
  if (mIB)
    SAFE_DELETE(mIB);
  min_ = Vector3D(FLT_MAX,FLT_MAX,FLT_MAX);
  max_ = Vector3D(FLT_MIN,FLT_MIN,FLT_MIN);
  center_ = Vector3D();
  mComputeEdges = true;
}

