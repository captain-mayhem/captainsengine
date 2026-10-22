//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |             The simple but powerful game engine                  |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | 2007-03-15                                                       |
// |------------------------------------------------------------------|
// | File: mesh.h                                                     |
//  ==================================================================
// based on:
//  ==================================================================
// | Medalyvis                                                        |
// |------------------------------------------------------------------|
// | Medical Data Analysis and Visualization                          |
// |------------------------------------------------------------------|
// | (c) Copyright Marco Staginski 2006-2007                          |
// |------------------------------------------------------------------|
// | File: MDAVmesh.hh                                                |
// | Created on: 15/03/2007                                           |
// |------------------------------------------------------------------|
// | Description:                                                     |
// | A render component for rendering meshes                          |
//  ==================================================================

#ifndef MESH_H
#define MESH_H

#include <cge_export.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <memory>
#include <string>

#include "triangle.h"
#include "../math/bbox.h"

namespace CGE{
class Material;
namespace Gfx{
class Buffer;
class DrawContext;
class RenderDevice;
class Texture;
}
}

namespace CGE{

//! Mesh provides storage and loading for meshes
class CGE_API Mesh{ 
public:
  enum DrawPart{
    DRAW_ALL,
    DRAW_OPAQUE,
    DRAW_TRANSPARENT,
  };
  struct CGE_API SubMesh{
    SubMesh(int off, int cnt, int mat, Material* dfltMat) : offset(off), count(cnt), material(mat), dfltMaterial(dfltMat) {}
    int offset;
    int count;
    int material;
    Material* dfltMaterial;
  };
	//! Default constructor
	Mesh();
	//! Destructor
	virtual ~Mesh();
  //! add vertex
  void addVertex(float x, float y, float z);
  //! add vertex
  void addVertex(CGE::Vec3f v);
  //! add texture coordinates
  void addTexCoord(float x, float y, float z = -1);
  //! add normal
  void addNormal(float x, float y, float z);
  //! add edge
  Edge* addEdge(int v0, int v1);
  //! add triangle
  Triangle* addTriangle(int v0, int v1, int v2, int t0 = -1, int t1 = -1, int t2 = -1, int n0 = -1, int n1 = -1, int n2 = -1);
  //! add submesh
  void addSubMesh(int triangleFrom, int triangleCount, int materialIdx);
  //! loads a mesh from a file
  bool loadFromFile(std::string filename);
  //! builds the buffers on a render device
  /*! Also loads the materials' textures onto the device. A mesh that isn't
      built gets its buffers when it is first drawn. */
  bool buildBuffers(Gfx::RenderDevice& device);
  //! draws the mesh through a render device with the context's lit shader
  /*! Sets the context's material and texture per submesh. Submeshes
      without a material are drawn white with the texture the context had,
      and that texture is the context's again afterwards. */
  void draw(Gfx::DrawContext& ctx, DrawPart part);
  //! get the name of the mesh
  std::string getName() const {return name_;}
  //! get the filename of the mesh
  std::string getFilename() const {return filename_;}
  //! the file name a scene saves for the mesh
  void setFilename(const std::string& filename) {filename_ = filename;}

  //! is the mesh visible
  bool isVisible() {return m_visible;}

  //! get the normal of a vertex (averaged over its adjacent triangles)
  CGE::Vector3D getNormal(int i);

  //! get the center of the mesh
  CGE::Vector3D getCenter() {return center_;}

  //! get the extent
  CGE::BBox getExtents() {return CGE::BBox(min_,max_);}

  //! intersects a ray and a triangle
  float intersect(const CGE::Ray& r) const;

  //! get a triangle from the mesh
  bool getTriangle(int i, CGE::Vector3D** result) const;

  //! get number of triangles
  int getNumTriangles() const {return numTriangles_;}

  //! clear an existing mesh
  void clear();

  //! sort submeshes according to material
  void materialSort();
protected:
  //! load obj-files
  bool loadOBJ(std::string filename);

  //! load off-files
  bool loadOFF(std::string filename);

  //! the submesh compare function
  static int submeshCompare(const void* p1, const void* p2);

  //! the interleaved vertices (position, texture coordinates if any, normal) and the indices
  void buildDrawData(std::vector<float>& vdata, std::vector<int>& idata);
  //! the buffers and textures on a render device
  bool createDeviceResources(Gfx::RenderDevice& device, const std::vector<float>& vdata, const std::vector<int>& idata);
  
  //! the number of vertices
  int numVertices_;

  //! the number of triangles
  int numTriangles_;

  //! the number of texture coordinates
  int numTexCoords_;

  //! the number of normals
  int mNumNormals;

  //! the vertices
  std::vector<Vertex> vertices_;

  //! the texture coordinates
  std::vector<CGE::Vector3D> texCoords_;

  //! the normals
  std::vector<Vec3f> mNormals;

  //! the edges
  EdgeList edges_;
  
  //! the triangles
  TriangleList triangles_;

  //! Extent of the mesh
  CGE::Vector3D min_, max_;

  //! The center of the mesh
  CGE::Vector3D center_;

  //! the indices
  std::vector<unsigned short> indices_;

  //! the name of the mesh
  std::string name_;

  //! the filename
  std::string filename_;

  //! should it be visible
  bool m_visible;

  //! compute edges
  bool mComputeEdges;

  //! the materials
  std::vector<Material*> mMaterials;

  //! the sub meshes
  std::vector<SubMesh> mSubmeshes;

  //! draw list
  std::vector<SubMesh> mDrawList;

  //! the vertex and index buffer on a render device
  std::unique_ptr<Gfx::Buffer> mVertices;
  std::unique_ptr<Gfx::Buffer> mIndices;
  //! the materials' textures on the render device, by material index (nullptr: untextured)
  std::vector<Gfx::Texture*> mMaterialTextures;
  std::vector<std::unique_ptr<Gfx::Texture>> mDeviceTextures;
  //! the device resources were created (or failed)
  bool mDeviceTried;
};

}

#endif
