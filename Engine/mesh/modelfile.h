#ifndef CGE_MODELFILE_H
#define CGE_MODELFILE_H

#include <cge_export.h>

#include <memory>
#include <string>
#include <vector>

#include "mesh.h"
#include "../math/bbox.h"

namespace CGE{

class SceneNode;
namespace Gfx{
class DrawContext;
class RenderDevice;
}

//! A model loaded from a file: its meshes with their materials, and its node hierarchy
/*! Reads glTF 2.0 (.gltf with external or embedded buffers and images, and
    .glb), and the formats Mesh reads (.obj, .off) as a single node. Files
    are assets (see Assets), so the same code works on every platform.

    glTF: each mesh becomes a Mesh, its primitives submeshes with their
    materials. The nodes become SceneNodes with their names and
    transformations (Model for nodes with a mesh), below a root node, so a
    game can find a node by its name and attach things to it (a socket).
    Positions, normals (as they are; flat normals for primitives without),
    the first texture coordinates, the first vertex colours and the indices
    (8, 16 and 32 bit) are read; quantized attributes
    (KHR_mesh_quantization) and sparse accessors too. The metallic-roughness
    material is mapped onto Material: the base colour (factor and texture)
    as the diffuse colour, the roughness as the shininess, the specular
    colour from the metalness; the alpha modes and double-sided materials
    are kept. The colour factors and vertex colours, which glTF keeps
    linear, are converted to sRGB like the textures' colours. Cameras,
    lights, skins, animations, morph targets, tangents, normal and other
    textures are left out; getWarnings lists what a file had of them.
    glTF's coordinate system (y up, front faces counter-clockwise) and
    column-major matrices are the engine's.

    The meshes belong to the model file. The nodes refer to them, so copies
    of the nodes (SceneNode::clone) must not outlive it. */
class CGE_API ModelFile{
public:
  ~ModelFile();
  ModelFile(const ModelFile&) = delete;
  ModelFile& operator=(const ModelFile&) = delete;
  //! loads a model file (an asset); nullptr if it can't, with the reason in error
  static std::unique_ptr<ModelFile> load(const std::string& path, std::string* error = nullptr);
  //! the root: a plain node (SceneNode::GROUP) without a name, with the file's top nodes as children
  SceneNode* getRoot() const {return mRoot.get();}
  //! the first node of that name (SceneNode::findNode); nullptr if there is none
  SceneNode* findNode(const std::string& name) const;
  //! the meshes, in the file's order
  const std::vector<std::unique_ptr<Mesh>>& getMeshes() const {return mMeshes;}
  //! what the file had that was left out or couldn't be read, one message each
  const std::vector<std::string>& getWarnings() const {return mWarnings;}
  //! builds the meshes' buffers and textures on a device; otherwise that happens when they're first drawn
  bool buildBuffers(Gfx::RenderDevice& device);
  //! renders the node hierarchy in the context's model matrix (see SceneNode::render)
  void render(Gfx::DrawContext& ctx, Mesh::DrawPart part = Mesh::DRAW_ALL);
  //! the bounds of the meshes in the root's space (the boxes of the meshes, transformed)
  BBox getBounds() const;
private:
  ModelFile();
  friend class GltfReader;
  std::unique_ptr<SceneNode> mRoot;
  std::vector<std::unique_ptr<Mesh>> mMeshes;
  std::vector<std::string> mWarnings;
};

}

#endif
