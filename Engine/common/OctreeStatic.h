//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |             The simple but powerful game engine                  |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | 2008-12-18                                                       |
// |------------------------------------------------------------------|
// | File: octree.h                                                   |
//  ==================================================================

#ifndef CGE_OCTREE_STATIC
#define CGE_OCTREE_STATIC

#include <cstdlib>
#include <list>
#include <cassert>
#include "math/vector.h"
#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/camera.h"
#include "renderer/font.h"
#include "system/types.h"

namespace CGE{

template<typename value>
class OctreeStatic{
public:
  enum TraversalState{
    RECURSE_FURTHER,
    RECURSE_TO_END,
    STOP_RECURSION,
  };
  enum Flags{
    PASS_THROUGH = 1,
    USER1 = 16,
    USER2 = 32,
    USER3 = 64,
    USER4 = 128,
  };
  OctreeStatic();
  ~OctreeStatic();
  void init(const CGE::Vec3f& center, const CGE::Vec3f& span, const CGE::Vec3f minspan);    
  bool insert(const CGE::BBox& where, value val);
  void setTraversalCB(TraversalState (*func)(const std::vector<value>& values, const BBox& box, uint8 flags, void* data)) {mTraversalCB = func;}
  void traverse(void* data);
protected:
  struct Node{
    Node() {
      memset(mChildren, 0, 8*sizeof(Node*));
    }
    ~Node(){
      for (int i = 0; i < 8; ++i){
        delete mChildren[i];
      }
    }
    std::vector<value> mValues;
    Node* mChildren[8];
  };

  Node* findNode(Node* prevnode, Node* startnode, const CGE::BBox& treebox, const CGE::BBox& box);
  void traverse(Node* node, const CGE::BBox& box, uint8 flags, void* data);
  //Node* recurse(Node* node, int& data1, void* data2, void (*func)(Node* node, int& data1, void* data2));
  //void recurseCullAndRender(const CGE::Camera* cam, Node* node, bool passThrough);

  //static void insertVertex(Node* node, int& data1, void* data2);
  //Node* mValues;

  CGE::BBox mBox;
  CGE::Vec3f mMinSpan;
  CGE::Vec3i mNodeCount;
  Node* mRoot;
  TraversalState (*mTraversalCB)(const std::vector<value>& values, const BBox& box, uint8 flags, void* data);
};

//impl

template<typename value> 
OctreeStatic<value>::OctreeStatic() : mBox(), mMinSpan(), mNodeCount(), mRoot(NULL), mTraversalCB(NULL) {

}

template<typename value> 
OctreeStatic<value>::~OctreeStatic(){
  delete mRoot;
}

template<typename value>
void OctreeStatic<value>::init(const CGE::Vec3f& center, const CGE::Vec3f& span, const CGE::Vec3f minspan){
  mBox = CGE::BBox(center-span/2,center+span/2);
  mMinSpan = minspan;
  mRoot = new Node();
}

template<typename value> 
bool OctreeStatic<value>::insert(const CGE::BBox& box, value val){
  Node* node = findNode(NULL, mRoot, mBox, box);
  if (node == NULL)
    return false;
  node->mValues.push_back(val);
  return true;
}

template<typename value>
typename OctreeStatic<value>::Node* OctreeStatic<value>::findNode(typename OctreeStatic<value>::Node* prevnode, typename OctreeStatic<value>::Node* startnode, const CGE::BBox& treebox, const CGE::BBox& box){
  Vec3f boxcenter = box.getCenter();
  Vec3f boxspan = box.getSpan();
  Vec3f treecenter = treebox.getCenter();
  Vec3f treespan = treebox.getSpan();

  if (treespan.x < boxspan.x || treespan.y < boxspan.y || treespan.z < boxspan.z)
    return prevnode;
  if (treespan.x < mMinSpan.x && treespan.y < mMinSpan.y && treespan.z < mMinSpan.z)
    return startnode;

  int access;
  if (boxcenter.x < treecenter.x){
    access = 1 << 2;
    treecenter.x = treecenter.x-treespan.x/2;
  }
  else{
    access = 0;
    treecenter.x = treecenter.x+treespan.x/2;
  }
  if (boxcenter.y < treecenter.y){
    access |= 1 << 1;
    treecenter.y = treecenter.y-treespan.y/2;
  }
  else{
    treecenter.y = treecenter.y+treespan.y/2;
  }
  if (boxcenter.z < treecenter.z){
    access |= 1;
    treecenter.z = treecenter.z-treespan.z/2;
  }
  else{
    treecenter.z = treecenter.z+treespan.z/2;
  }

  Node** newnode = startnode->mChildren+access;
  if (*newnode == NULL)
    *newnode = new Node();
  return findNode(startnode, *newnode, BBox(treecenter-treespan/2, treecenter+treespan/2), box);
}

template<typename value>
void OctreeStatic<value>::traverse(void* data){
  traverse(mRoot, mBox, 0, data);
}

template<typename value>
void OctreeStatic<value>::traverse(Node* node, const CGE::BBox& box, uint8 flags, void* data){
  TraversalState state = STOP_RECURSION;
  if (mTraversalCB){
    state = mTraversalCB(node->mValues, box, flags, data);
  }
  if (state == RECURSE_FURTHER){
    Vec3f treecenter = box.getCenter();
    Vec3f treespan = box.getSpan()/2;
    Vec3f newcenter;
    //update boxes
    if (node->mChildren[0]){
      newcenter.x = treecenter.x + treespan.x;
      newcenter.y = treecenter.y + treespan.y;
      newcenter.z = treecenter.z + treespan.z;
      traverse(node->mChildren[0], CGE::BBox(newcenter-treespan, newcenter+treespan),0, data);
    }
    if (node->mChildren[1]){
      newcenter.x = treecenter.x + treespan.x;
      newcenter.y = treecenter.y + treespan.y;
      newcenter.z = treecenter.z - treespan.z;
      traverse(node->mChildren[1], CGE::BBox(newcenter-treespan, newcenter+treespan),0, data);
    }
    if (node->mChildren[2]){
      newcenter.x = treecenter.x + treespan.x;
      newcenter.y = treecenter.y - treespan.y;
      newcenter.z = treecenter.z + treespan.z;
      traverse(node->mChildren[2], CGE::BBox(newcenter-treespan, newcenter+treespan),0, data);
    }
    if (node->mChildren[3]){
      newcenter.x = treecenter.x + treespan.x;
      newcenter.y = treecenter.y - treespan.y;
      newcenter.z = treecenter.z - treespan.z;
      traverse(node->mChildren[3], CGE::BBox(newcenter-treespan, newcenter+treespan),0, data);
    }
    if (node->mChildren[4]){
      newcenter.x = treecenter.x - treespan.x;
      newcenter.y = treecenter.y + treespan.y;
      newcenter.z = treecenter.z + treespan.z;
      traverse(node->mChildren[4], CGE::BBox(newcenter-treespan, newcenter+treespan),0, data);
    }
    if (node->mChildren[5]){
      newcenter.x = treecenter.x - treespan.x;
      newcenter.y = treecenter.y + treespan.y;
      newcenter.z = treecenter.z - treespan.z;
      traverse(node->mChildren[5], CGE::BBox(newcenter-treespan, newcenter+treespan),0, data);
    }
    if (node->mChildren[6]){
      newcenter.x = treecenter.x - treespan.x;
      newcenter.y = treecenter.y - treespan.y;
      newcenter.z = treecenter.z + treespan.z;
      traverse(node->mChildren[6], CGE::BBox(newcenter-treespan, newcenter+treespan),0, data);
    }
    if (node->mChildren[7]){
      newcenter.x = treecenter.x - treespan.x;
      newcenter.y = treecenter.y - treespan.y;
      newcenter.z = treecenter.z - treespan.z;
      traverse(node->mChildren[7], CGE::BBox(newcenter-treespan, newcenter+treespan),0, data);
    }
  }
  if (state == RECURSE_TO_END){
    if (node->mChildren[0]){
      traverse(node->mChildren[0], box, flags | PASS_THROUGH, data);
    }
    if (node->mChildren[1]){
      traverse(node->mChildren[1], box, flags | PASS_THROUGH, data);
    }
    if (node->mChildren[2]){
      traverse(node->mChildren[2], box, flags | PASS_THROUGH, data);
    }
    if (node->mChildren[3]){
      traverse(node->mChildren[3], box, flags | PASS_THROUGH, data);
    }
    if (node->mChildren[4]){
      traverse(node->mChildren[4], box, flags | PASS_THROUGH, data);
    }
    if (node->mChildren[5]){
      traverse(node->mChildren[5], box, flags | PASS_THROUGH, data);
    }
    if (node->mChildren[6]){
      traverse(node->mChildren[6], box, flags | PASS_THROUGH, data);
    }
    if (node->mChildren[7]){
      traverse(node->mChildren[7], box, flags | PASS_THROUGH, data);
    }
  }
}


}

#endif

