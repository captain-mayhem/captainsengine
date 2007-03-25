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

#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>

using namespace MeshGeo;
using Math::Vector3D;

// --------------------------------------------------------------------
//  Default constructor
// --------------------------------------------------------------------
Mesh::Mesh(){
  numTriangles_ = 0;
  numVertices_ = 0;
  numTexCoords_ = 0;
  m_name = "undefined";
  m_visible = true;
  m_light = true;
  for (int i = 0; i < 4; i++){
    m_color[i] = 1.0;
  }
}

// --------------------------------------------------------------------
//  Destructor
// --------------------------------------------------------------------
Mesh::~Mesh(){
  //glDeleteBuffersARB(1, &m_vbo);
  //glDeleteBuffersARB(1, &m_vboidx);
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
  vertices_.push_back(Vertex(x,y,z));
  numVertices_++;
}


// add texture coordinates
void Mesh::addTexCoord(float x, float y, float z){
  texCoords_.push_back(Vector3D(x,y,z));
  numTexCoords_++;
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
Triangle* Mesh::addTriangle(int v0, int v1, int v2, int t0, int t1, int t2){
  
  Edge* e0 = vertices_[v1].eList.getEdge(v1,v2);
  if (!e0)
    e0 = addEdge(v1,v2);
  
  Edge* e1 = vertices_[v1].eList.getEdge(v2,v0);
  if (!e1)
    e1 = addEdge(v2,v0);
  
  Edge* e2 = vertices_[v1].eList.getEdge(v0,v1);
  if (!e2)
    e2 = addEdge(v0,v1);
  
  Triangle* tri = new Triangle(v0, v1, v2, t0, t1, t2, e0, e1, e2);
  triangles_.insertTriangle(tri);

  e0->tList.insertTriangle(tri);
  e1->tList.insertTriangle(tri);
  e2->tList.insertTriangle(tri);
  
  numTriangles_++;
  return tri;  
}

//load from file
bool Mesh::loadFromFile(std::string filename){
  int n = filename.find_last_of(".");
  std::string ending = filename.substr(n+1);
  m_name = filename;
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
  char type;
  char tester;	
  float x,y,z;
  unsigned int vertexid[3], textureid[3], normalid[3];
  char line[2000];

  std::ifstream file(filename.c_str());
  if (!file)
    return false;
  bool bEnd = false;
  while(!file.eof() || bEnd){
    file >> type;
    if (file.eof())
      break;
    switch(type){
      case 'v':
        if (file.peek() =='t') {
          // texture coordinate
          file >> type;
          file >> x; if (file.eof()) {bEnd = true; fprintf(stderr, "end at x\n"); break;};
          file >> y; if (file.eof()) {bEnd = true; fprintf(stderr, "end at y\n"); break;};
          //addTextureVertex(x,y,0);
        }
        else if (file.peek() == 'n') {
          // normal vector
          file >> type;
          file >> x; if (file.eof()) {bEnd = true; fprintf(stderr, "end at x\n"); break;};
          file >> y; if (file.eof()) {bEnd = true; fprintf(stderr, "end at y\n"); break;};
          file >> z; if (file.eof()) {bEnd = true; fprintf(stderr, "end at z\n"); break;};
          //addNormalVertex(x,y,z);
        }
        else {
          file >> x; if (file.eof()) {bEnd = true; fprintf(stderr, "end at x\n"); break;};
          file >> y; if (file.eof()) {bEnd = true; fprintf(stderr, "end at y\n"); break;};
          file >> z; if (file.eof()) {bEnd = true; fprintf(stderr, "end at z\n"); break;};
          addVertex(x,y,z);
        }
        break;
      case 'f':
        file >> vertexid[0];
        if (file.eof()) {bEnd = true; fprintf(stderr, "end at v0\n"); break;}
        if (file.peek() == '/'){
          // read texture index
          file >> tester >> textureid[0]; 
          if (file.eof()) {bEnd = true; fprintf(stderr, "end at t0\n"); break;}
        }
        if (file.peek() == '/') {
          // read normal index
          file >> tester >> normalid[0]; 
          if (file.eof()) {bEnd = true; fprintf(stderr, "end at n0\n"); break;}
        }

        file >> vertexid[1];
        if (file.eof()) {bEnd = true; fprintf(stderr, "end at v1\n"); break;}
        if (file.peek() == '/') {
          // read texture index
          file >> tester >> textureid[1]; 
          if (file.eof()) {bEnd = true; fprintf(stderr, "end at t1\n"); break;}
        }
        if (file.peek() == '/'){
          // read normal index
          file >> tester >> normalid[1]; 
          if (file.eof()) {bEnd = true; fprintf(stderr, "end at n1\n"); break;}
        }

        file >> vertexid[2];
        if (file.eof()) {bEnd = true; fprintf(stderr, "end at v2\n"); break;}
        if (file.peek() == '/') {
          // read texture index
          file >> tester >> textureid[2]; 
          if (file.eof()) {bEnd = true; fprintf(stderr, "end at t2\n"); break;}
        }
        if (file.peek() == '/') {
          // read normal index
          file >> tester >> normalid[2]; 
          if (file.eof()) {bEnd = true; fprintf(stderr, "end at n2\n"); break;}
        }
        addTriangle(vertexid[0]-1,vertexid[1]-1,vertexid[2]-1);
        break;
      default:
        file.getline(line, 2000);
        break;
    }
  }	// while
  file.close();
  //exit(1);
  //std::cerr << "Read " << m_numTriangles << " triangles" << std::endl;
  //std::cerr << "Read " << m_numVertices << " vertices " << std::endl;
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
    //x /= 330.596;
    //y /= 390.473;
    //z /= 460.378;
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

  return normal;
}

//! build vertex buffer object
void Mesh::buildVBO(){
  float* vertices = new float[numTriangles_*3*3];
  unsigned int* indices = new unsigned int[numTriangles_*3];
  
  vb_ = System::Engine::instance()->getRenderer()->createVertexBuffer();
  if (numTexCoords_>0)
    vb_->create(VB_POSITION | VB_NORMAL | VB_TEXCOORD, numTriangles_*3, numTriangles_*3);
  else
    vb_->create(VB_POSITION | VB_NORMAL, numTriangles_*3, numTriangles_*3);
  vb_->lockVertexPointer();
  
  for (int i = 0; i < numTriangles_; i++){
    Triangle* tri = triangles_[i];
    Vector3D v0 = vertices_[tri->v0];
    Vector3D v1 = vertices_[tri->v1];
    Vector3D v2 = vertices_[tri->v2];
    vb_->setPosition(3*i+0, v0);
    vb_->setPosition(3*i+1, v1);
    vb_->setPosition(3*i+3, v2);
    Vector3D n0 = getNormal(tri->v0);
    Vector3D n1 = getNormal(tri->v1);
    Vector3D n2 = getNormal(tri->v2);
    if (numTexCoords_>0){
      Vector3D t0 = texCoords_[tri->t0];
      Vector3D t1 = texCoords_[tri->t1];
      Vector3D t2 = texCoords_[tri->t2];
      vb_->setTexCoord(3*i+0, Vec2f());
    } 
    indices[i*3+0] = i*3+0;
    indices[i*3+1] = i*3+1;
    indices[i*3+2] = i*3+2;
  }
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
  delete [] vertices;
  delete [] indices;
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
}

