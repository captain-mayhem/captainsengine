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

#ifdef WIN32
#include <windows.h>
#endif

#include <string>

#include "triangle.h"

namespace Graphics{
class VertexBuffer;
}

namespace MeshGeo{

//! Mesh provides storage and loading for meshes
class Mesh{
      
public:
	//! Default constructor
	Mesh();
	//! Destructor
	~Mesh();
  //! add vertex
  void addVertex(float x, float y, float z);
  //! add texture coordinates
  void addTexCoord(float x, float y, float z = -1);
  //! add edge
  Edge* addEdge(int v0, int v1);
  //! add triangle
  Triangle* addTriangle(int v0, int v1, int v2, int t0=-1, int t1=-1, int t2=-1);
  //! loads a mesh from a file
  bool loadFromFile(std::string filename);
  //! build vertex buffer object
  void buildVBO();
	//! draw the mesh
	void draw();
  //! get the name of the mesh
  std::string getName() {return m_name;}

  //! is the mesh visible
  bool isVisible() {return m_visible;}

  //! is the mesh lighted
  bool isLighted() {return m_light;}

  //! the mesh color
  float* getColor() {return m_color;}

  //! get the normal of a triangle
  float* getNormal(int i);

protected:
  //! load obj-files
  bool loadOBJ(std::string filename);

  //! load off-files
  bool loadOFF(std::string filename);
  
  //! the number of vertices
  int numVertices_;

  //! the number of triangles
  int numTriangles_;

  //! the number of texture coordinates
  int numTexCoords_;

  //! the vertices
  std::vector<Vertex> vertices_;

  //! the texture coordinates
  std::vector<Math::Vector3D> texCoords_;

  //! the edges
  EdgeList edges_;
  
  //! the triangles
  TriangleList triangles_;

  //! the indices
  std::vector<unsigned short> indices_;

  //! the vertex buffer
  Graphics::VertexBuffer* vb_;
  
  //! vertex buffer object
  //GLuint m_vbo;

  //! index buffer for vbo
  //GLuint m_vboidx;

  //! the name of the mesh
  std::string m_name;

  //! should it be visible
  bool m_visible;

  //! lighting?
  bool m_light;

  // color
  float m_color[4];

};

}

#endif // MDAV_MESH_RENDER_COMPONENT_HH
