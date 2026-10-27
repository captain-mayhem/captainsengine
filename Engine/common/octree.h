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

#include <cstdlib>
#include <list>
#include <cassert>
#include "math/vector.h"
#include "system/engine.h"
#include "renderer/camera.h"
#include "renderer/font.h"


TR_CHANNEL_EXT(CGE_Common_Octree)

namespace CGE{

template<typename index, typename value>
class Octree{
public:

  struct Node{
    Node(const CGE::Vec3<index>& idx, value val, const CGE::Vec3<index>& boxMin, const CGE::Vec3<index>& boxMax) : mIndex(idx), mParent(NULL), mIsLeaf(true), mBox(boxMin,boxMax) {
      for(int i = 0; i < 8; ++i)
        mChildren[i] = NULL;
    };
    ~Node();
    CGE::Vec3<index> mIndex;
    CGE::BBox mBox;
    std::list<value> mValue;
    Node* mChildren[8];
    Node* mParent;
    bool mIsLeaf;
  };

  Octree();
  ~Octree();
  void init(const CGE::Vec3<index>& center, const CGE::Vec3<index>& halfspan){
    mCenter = center; mSpan = halfspan;
  }
  void insert(const CGE::Vec3<index>& center, value val);
  //value& at(index x, index y, index z);
  void render(const CGE::Camera* cam);
  void setRenderCallback(void (*func)(value val)){mRenderCB = func;}
protected:

  class iterator{
    public:
      iterator() : mCurrent(NULL), mIndex(0) {}
      Node* operator*() {return mCurrent;}
    protected:
      Node* mCurrent;
      int mIndex;
  };

  Node** recurseCreate(const CGE::Vec3<index>& idx, typename Octree<index,value>::Node** node, CGE::Vec3<index> split, CGE::Vec3<index> span, int level);
  Node* recurse(Node* node, int& data1, void* data2, void (*func)(Node* node, int& data1, void* data2));
  void recurseCullAndRender(const CGE::Camera* cam, Node* node, bool passThrough);

  Node* mRoot;
  value mDefault;
  CGE::Vec3<index> mCenter;
  CGE::Vec3<index> mSpan;
  int mNodeCount;
  void (*mRenderCB)(value val);
};

//impl

template<typename index, typename value> 
Octree<index,value>::Octree() : mRoot(NULL), mDefault(0), mCenter(), mSpan(), mNodeCount(0), mRenderCB(NULL) {

}

template<typename index, typename value> 
Octree<index,value>::~Octree(){
}

//template<typename index, typename value> 
//value& Octree<index,value>::at(index x, index y, index z){
  //Node** nd = recurseCreate(x,y,z,mRoot,mCenter, mSpan);
  //return (*nd)->mValue;
//}

template<typename index, typename value> 
void Octree<index,value>::insert(const CGE::Vec3<index>& center, value val){
  TR_USE(CGE_Common_Octree)
  if((center.x < (mCenter-mSpan).x) ||
    (center.y < (mCenter-mSpan).y) ||
    (center.z < (mCenter-mSpan).z) ||
    (center.x > (mCenter+mSpan).x) ||
    (center.y > (mCenter+mSpan).y) ||
    (center.z > (mCenter+mSpan).z)){
      TR_WARN("Octree: tried to add out of bounds object");
      return;
  }
  Node** nd = recurseCreate(center,&mRoot,mCenter,mSpan,0);
  (*nd)->mValue.push_back(val);
  ++mNodeCount;
}

template<typename index, typename value> 
typename Octree<index,value>::Node* Octree<index,value>::recurse(typename Octree<index,value>::Node* node, int& data1, void* data2, void (*func)(Node* node, int& data1, void* data2)){
  if (node->mIsLeaf){
    (*func)(node,data1,data2);
  }
  for (int i = 0; i < 8; ++i){
    if (node->mChildren[i])
      recurse(node->mChildren[i],data1,data2,func);
  }
  return NULL;
}

template<typename index, typename value> 
typename Octree<index,value>::Node** Octree<index,value>::recurseCreate(const CGE::Vec3<index>& idx, typename Octree<index,value>::Node** node, CGE::Vec3<index> split, CGE::Vec3<index> span, int level){
  if (*node == NULL){
    *node = new Node(idx-mCenter,mDefault,split-span,split+span);
    return node;
  }
  if (!(span != CGE::Vec3<index>()))
    return node;
  if (!(idx != ((*node)->mIndex)) && (*node)->mIsLeaf)
    return node;
  //if (level > 5){
  //  (*node)->mIsLeaf = true;
  //  return node;
  //}
  int access;
  if (idx.x < split.x){
    access = 1 << 2;
    split.x = split.x-span.x/2;
  }
  else{
    access = 0;
    split.x = split.x+span.x/2;
  }
  if (idx.y < split.y){
    access |= 1 << 1;
    split.y = split.y-span.y/2;
  }
  else{
    split.y = split.y+span.y/2;
  }
  if (idx.z < split.z){
    access |= 1;
    split.z = split.z-span.z/2;
  }
  else{
    split.z = split.z+span.z/2;
  }
  if ((*node)->mIsLeaf){
    (*node)->mIsLeaf = false;
    Node** movednode = recurseCreate((*node)->mIndex+mCenter,node,(*node)->mBox.getCenter(),((*node)->mBox.getMax()-(*node)->mBox.getMin())/2,level);
    (*movednode)->mParent = *node;
    (*movednode)->mValue = (*node)->mValue;
    (*node)->mValue.clear();
  }
  Node** child = &(*node)->mChildren[access];
  Node** grandchild = recurseCreate(idx,child,split,span/2,level+1);
  (*grandchild)->mParent = *child;
  return grandchild;
}

template<typename index, typename value>
void Octree<index,value>::recurseCullAndRender(const CGE::Camera* cam, Node* node, bool passThrough){
  CGE::Frustum::Result res;
  //test or use result from parent
  if (passThrough)
    res = CGE::Frustum::INSIDE;
  else{
    res = cam->getFrustum().checkBox(node->mBox);

    //we can stop here or pass through
    if (res == CGE::Frustum::OUTSIDE)
      return;
    else if (res == CGE::Frustum::INSIDE){
      passThrough = true;
    }
  }
  
  //render this node
  if(mRenderCB){
    for (typename std::list<value>::iterator iter = node->mValue.begin(); iter != node->mValue.end(); ++iter){
      mRenderCB(*iter);
    }
  }

  //recurse in subnodes
  if (node->mChildren[0])
    recurseCullAndRender(cam,node->mChildren[0],passThrough);
  if (node->mChildren[1])
    recurseCullAndRender(cam,node->mChildren[1],passThrough);
  if (node->mChildren[2])  
    recurseCullAndRender(cam,node->mChildren[2],passThrough);
  if (node->mChildren[3])
    recurseCullAndRender(cam,node->mChildren[3],passThrough);
  if (node->mChildren[4])
    recurseCullAndRender(cam,node->mChildren[4],passThrough);
  if (node->mChildren[5])
    recurseCullAndRender(cam,node->mChildren[5],passThrough);
  if (node->mChildren[6])
    recurseCullAndRender(cam,node->mChildren[6],passThrough);
  if (node->mChildren[7])
    recurseCullAndRender(cam,node->mChildren[7],passThrough);
}

template<typename index, typename value>
void Octree<index,value>::render(const CGE::Camera* cam){
  recurseCullAndRender(cam, mRoot, false);
}

}
