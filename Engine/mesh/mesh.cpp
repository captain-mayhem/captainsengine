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
#include "../renderer/drawcontext.h"
#include "../renderer/renderdevice.h"
#include "../renderer/textureloader.h"
#include "../math/ray.h"
#include "../system/file.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <float.h>
#include <algorithm>

using std::string;
using std::vector;

using std::min;
using namespace CGE;

TR_CHANNEL(CGE_Mesh);

// --------------------------------------------------------------------
//  Default constructor
// --------------------------------------------------------------------
Mesh::Mesh() : mComputeEdges(true), mDeviceTried(false){
  numTriangles_ = 0;
  numVertices_ = 0;
  numTexCoords_ = 0;
  mNumNormals = 0;
  name_ = "none";
  filename_ = "none";
  m_visible = true;
  min_ = Vector3D(FLT_MAX,FLT_MAX,FLT_MAX);
  max_ = Vector3D(FLT_MIN,FLT_MIN,FLT_MIN);
  center_ = Vector3D();
  mMaterials.push_back(nullptr);
  //vertices_.reserve(200000);
}

// --------------------------------------------------------------------
//  Destructor
// --------------------------------------------------------------------
Mesh::~Mesh(){
  //glDeleteBuffersARB(1, &m_vbo);
  //glDeleteBuffersARB(1, &m_vboidx);
  clear();
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
  texCoords_.push_back(Vector3D(x,1.0f-y,z));
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
  Edge *e0 = nullptr, *e1 = nullptr, *e2 = nullptr;
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
  TR_USE(CGE_Mesh);
  char tester;	
  float x,y,z;
  unsigned int vertexid[4], textureid[4], normalid[4];
  char line[2000];

  std::map<string, int> materialMap;
  int materialIdx = 0;
  
  char* pos;
  FILE* file = fopen(filename.c_str(), "r");
  if (!file)
    return false;
  int subMeshStart = 0;
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
        if (*mtlfile.rbegin() == '\n')
          mtlfile.resize(mtlfile.size()-1);
        string path = Filesystem::getPathComponent(filename);
        mtlfile = Filesystem::combinePath(path, mtlfile);
        unsigned from = (unsigned)mMaterials.size();
        if (!Material::loadFromMTL(mtlfile, mMaterials))
          TR_ERROR("mtlfile %s not found", mtlfile.c_str());
        for (unsigned i = from; i < mMaterials.size(); ++i){
          materialMap[mMaterials[i]->getName()] = i;
        }
      }
      else
        TR_WARN("%s unhandled", line);
      break;
	  case 'u':
		  if (memcmp(line, "usemtl", 6) == 0){
			  std::string mtl(line + 7);
        mtl.resize(mtl.size()-1);
			  addSubMesh(subMeshStart, (int)triangles_.size()-subMeshStart, materialIdx);
        subMeshStart = (int)triangles_.size();
        materialIdx = materialMap[mtl];
		  }
      else
        TR_WARN("%s unhandled", line);
		  break;
    case 'g':
      break;
    case '#':
    case '\n':
      break;
    default:
      TR_WARN("%s unhandled", line);
      break;
    }
  }
  addSubMesh(subMeshStart, (int)triangles_.size() - subMeshStart, materialIdx);
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


//! get the normal of a vertex (averaged over its adjacent triangles)
Vector3D Mesh::getNormal(int i){
  if (!mComputeEdges)
    return Vector3D(0,0,1);

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

void Mesh::buildDrawData(std::vector<float>& vdata, std::vector<int>& idata){
  materialSort();
  mDrawList.clear();
  std::map<Vec3i, int> distinctVertices;
  vdata.clear();
  vdata.reserve(numVertices_*(3 + 3 + 2));
  idata.clear();
  idata.reserve(numTriangles_ * 3);
  int vidx = 0;
  
  unsigned offset = (unsigned)idata.size();
  int idxcount = 0;
  int matid = mSubmeshes[0].material;

  for (unsigned mc = 0; mc < mSubmeshes.size(); ++mc){
    SubMesh& sm = mSubmeshes[mc];
    if (sm.material != matid){
      mDrawList.push_back(SubMesh(offset, idxcount, matid, mMaterials[matid]));
      offset = (unsigned)idata.size();
      matid = sm.material;
      idxcount = 0;
    }

    idxcount += sm.count;

    for (int i = sm.offset / 3; i < sm.offset / 3 + sm.count / 3; ++i){
      Triangle* tri = triangles_[i];
      for (int j = 0; j < 3; ++j){
        Vec3i idx(tri->data[j], tri->texdata[j], tri->normdata[j]);
        int di = distinctVertices[idx];
        if (di == 0){
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
  }
  mDrawList.push_back(SubMesh(offset, idxcount, matid, mMaterials[matid]));
}

bool Mesh::buildBuffers(Gfx::RenderDevice& device){
  std::vector<float> vdata;
  std::vector<int> idata;
  buildDrawData(vdata, idata);
  center_ = center_ / (float)numVertices_;
  return createDeviceResources(device, vdata, idata);
}

namespace{

//! the vertex layouts of buildDrawData's vertices
const Gfx::VertexLayout& meshLayout(bool texCoords){
  using namespace Gfx;
  static const VertexLayout withTexCoords = {
    {
      {ATTR_POSITION, VertexFormat::Float3, 0, 0},
      {ATTR_TEXCOORD, VertexFormat::Float2, 0, 12},
      {ATTR_NORMAL, VertexFormat::Float3, 0, 20},
    },
    {{(3+2+3)*(int)sizeof(float), false}},
  };
  static const VertexLayout withoutTexCoords = {
    {
      {ATTR_POSITION, VertexFormat::Float3, 0, 0},
      {ATTR_NORMAL, VertexFormat::Float3, 0, 12},
    },
    {{(3+3)*(int)sizeof(float), false}},
  };
  return texCoords ? withTexCoords : withoutTexCoords;
}

}

bool Mesh::createDeviceResources(Gfx::RenderDevice& device, const std::vector<float>& vdata, const std::vector<int>& idata){
  using namespace Gfx;
  mDeviceTried = true;
  mVertices = device.createBuffer({BufferType::Vertex, Usage::Immutable, vdata.size()*sizeof(float)}, vdata.data());
  mIndices = device.createBuffer({BufferType::Index, Usage::Immutable, idata.size()*sizeof(int)}, idata.data());
  //the materials' textures, each file once
  mMaterialTextures.assign(mMaterials.size(), nullptr);
  mDeviceTextures.clear();
  std::map<std::string, Gfx::Texture*> loaded;
  for (unsigned i = 0; i < mMaterials.size(); ++i){
    Material* mat = mMaterials[i];
    if (!mat)
      continue;
    //the diffuse texture wins, as with the old renderers
    bool opacity = mat->getDiffuseTexFile().empty();
    std::string file = opacity ? mat->getOpacityTexFile() : mat->getDiffuseTexFile();
    if (file.empty())
      continue;
    std::string key = (opacity ? "opacity:" : "diffuse:")+file;
    if (loaded.find(key) == loaded.end()){
      std::unique_ptr<Gfx::Texture> texture = Gfx::loadTexture(device, file, opacity);
      loaded[key] = texture.get();
      if (texture)
        mDeviceTextures.push_back(std::move(texture));
    }
    mMaterialTextures[i] = loaded[key];
  }
  return mVertices && mIndices;
}

void Mesh::draw(Gfx::DrawContext& ctx, DrawPart part){
  //a mesh that isn't built gets its buffers when it's first drawn
  if (!mDeviceTried && !mSubmeshes.empty())
    buildBuffers(ctx.getDevice());
  if (!mVertices || !mIndices)
    return;
  const Gfx::VertexLayout& layout = meshLayout(numTexCoords_ > 0);
  //submeshes without a material keep the texture the caller set
  Gfx::Texture* texture = ctx.getTexture();
  Gfx::Sampler* sampler = ctx.getSampler();
  for (const SubMesh& msh : mDrawList){
    Material* mat = mMaterials[msh.material];
    //no material is assumed opaque
    bool transparent = mat && mat->isTransparent();
    if ((transparent && part == DRAW_OPAQUE) || (!transparent && part == DRAW_TRANSPARENT))
      continue;
    if (mat){
      ctx.setMaterial(*mat);
      ctx.setTexture(msh.material < (int)mMaterialTextures.size() ? mMaterialTextures[msh.material] : nullptr);
    }
    else{
      ctx.resetMaterial();
      ctx.setTexture(texture, sampler);
    }
    ctx.drawIndexed(Gfx::BuiltinShader::Lit, mVertices.get(), layout, mIndices.get(), Gfx::IndexType::UInt32,
      Gfx::PrimitiveType::Triangles, msh.count, msh.offset);
  }
  ctx.setTexture(texture, sampler);
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
  indices_.clear();
  name_ = "none";
  filename_ = "none";
  for (unsigned i = 0; i < triangles_.size(); i++){
    delete triangles_[i];
  }
  triangles_.clear();
  for (unsigned i = 0; i < edges_.size(); i++){
    delete edges_[i];
    edges_[i] = nullptr;
  }
  edges_.clear();
  mVertices.reset();
  mIndices.reset();
  mMaterialTextures.clear();
  mDeviceTextures.clear();
  mDeviceTried = false;
  min_ = Vector3D(FLT_MAX,FLT_MAX,FLT_MAX);
  max_ = Vector3D(FLT_MIN,FLT_MIN,FLT_MIN);
  center_ = Vector3D();
  mComputeEdges = true;
  for (unsigned i = 0; i < mMaterials.size(); ++i){
    delete mMaterials[i];
  }
  mMaterials.clear();
  mSubmeshes.clear();
  mDrawList.clear();
}

void Mesh::addSubMesh(int triangleFrom, int triangleCount, int materialIdx){
  if (triangleCount == 0)
    return;
  TR_USE(CGE_Mesh);
  TR_DEBUG("%i %i, material %i", triangleFrom, triangleCount, materialIdx);
  mSubmeshes.push_back(SubMesh(triangleFrom*3, triangleCount*3, materialIdx, mMaterials[materialIdx]));
}

//! sort submeshes according to material
void Mesh::materialSort(){
  qsort(mSubmeshes.data(), mSubmeshes.size(), sizeof(SubMesh), submeshCompare);
}

int Mesh::submeshCompare(const void* p1, const void* p2){
  SubMesh* m1 = (SubMesh*)p1;
  SubMesh* m2 = (SubMesh*)p2;
  int cmpm1 = m1->material;
  int cmpm2 = m2->material;
  if (m1->dfltMaterial && m1->dfltMaterial->isTransparent())
    cmpm1 += 10000;
  if (m2->dfltMaterial && m2->dfltMaterial->isTransparent())
    cmpm2 += 10000;
  if (cmpm1 < cmpm2)
    return -1;
  if (cmpm1 == cmpm2)
    return 0;
  if (cmpm1 > cmpm2)
    return 1;
  return 0;
}


