#ifndef CGE_SCENENODE_H
#define CGE_SCENENODE_H

#include <cge_export.h>

#include <math/matrix.h>
#include <mesh/mesh.h>
#include <string>
#include <vector>

namespace CGE{

namespace Gfx{
class DrawContext;
}

//! A node in a hierarchy: a name, a transformation relative to the parent, and children
/*! A node owns its children. Model is the node with a mesh; plain nodes
    (GROUP) group others or mark places, e.g. the sockets of a model file
    that other models are attached to. */
class CGE_API SceneNode{
public:
  enum Type{
    MESH,
    //! a node without a mesh of its own
    GROUP,
  };
  //! Constructor
  SceneNode(Type type);
  //! copies the node and, as copies, its children
  /*! The copy has no parent and a new id. */
  SceneNode(const SceneNode& node);
  SceneNode& operator=(const SceneNode&) = delete;
  //! Destructor; deletes the children
  virtual ~SceneNode();
  //! a copy of the node and its children (see the copy constructor)
  virtual SceneNode* clone() const;
  //! set transformation
  inline void setTrafo(const CGE::Matrix& mat) { trafo_ = mat; }
  //! get transformation
  inline CGE::Matrix const& getTrafo() const { return trafo_; }
  //! the transformation into the root's space: the parents' transformations times the node's
  CGE::Matrix getWorldTrafo() const;
  //! get id
  inline unsigned int getID() const { return id_; }
  //! set id
  void setID(unsigned id) { id_ = id; }
  //! set id count
  static void setIDCount(unsigned start) { idCount_ = start; }
  //! get id count
  static unsigned getIDCount(){ return idCount_; }
  //! get the type
  Type getType() const { return mType; }
  //! the name (from a model file); names need not be unique
  void setName(const std::string& name) { mName = name; }
  const std::string& getName() const { return mName; }
  //! adds a child at the end and takes it over; it leaves its former parent
  void addChild(SceneNode* child);
  //! removes a child without deleting it; the caller owns it then
  /*! Returns false if it isn't a child of this node. */
  bool removeChild(SceneNode* child);
  //! the children in their order
  const std::vector<SceneNode*>& getChildren() const { return mChildren; }
  //! the parent, nullptr for a root
  SceneNode* getParent() const { return mParent; }
  //! the first node of that name: this node or one below it, depth first
  /*! nullptr if there is none. */
  SceneNode* findNode(const std::string& name);
  //! renders the node and its children in the context's model matrix times the node's transformation
  /*! A plain node draws nothing itself; Model draws its mesh. */
  virtual void render(Gfx::DrawContext& ctx, Mesh::DrawPart part);
protected:
  //! draws what the node itself shows, in its own space
  virtual void renderSelf(Gfx::DrawContext& ctx, Mesh::DrawPart part) {}
  //! the node type
  Type mType;
  //! The id counter
  static unsigned int idCount_;
  //! The object id
  unsigned int id_;
  //! The transformation
  CGE::Matrix trafo_;
  //! the name
  std::string mName;
  //! the parent, not owned
  SceneNode* mParent;
  //! the children, owned
  std::vector<SceneNode*> mChildren;
};

}

#endif
