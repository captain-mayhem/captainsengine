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
#include "renderer/renderer.h"
#include "renderer/indexbuffer.h"
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
  void buildDebugVertexBuffer();
  void renderDebug();
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

  static void insertVertex(Node* node, int& data1, void* data2);

  Node* mRoot;
  value mDefault;
  CGE::Vec3<index> mCenter;
  CGE::Vec3<index> mSpan;
  CGE::VertexBuffer* mVB;
  int mNodeCount;
  void (*mRenderCB)(value val);
};

//impl

template<typename index, typename value> 
Octree<index,value>::Octree() : mRoot(NULL), mDefault(0), mCenter(), mSpan(), mVB(NULL), mNodeCount(0), mRenderCB(NULL) {

}

template<typename index, typename value> 
Octree<index,value>::~Octree(){
  delete mVB;
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
void Octree<index,value>::insertVertex(Node* node, int& data1, void* data2){
  CGE::VertexBuffer* vb = (CGE::VertexBuffer*)data2;
  vb->setPosition(data1,CGE::Vec3f((float)node->mIndex.x,(float)node->mIndex.y,(float)node->mIndex.z));
  ++data1;
}

template<typename index, typename value>
void Octree<index,value>::buildDebugVertexBuffer(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  mVB = rend->createVertexBuffer();
  mVB->create(VB_POSITION,mNodeCount);
  mVB->lockVertexPointer();
  int vbpos = 0;
  recurse(mRoot,vbpos,mVB,insertVertex);
  mVB->unlockVertexPointer();
}

template<typename index, typename value> 
void Octree<index,value>::renderDebug(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->pushMatrix();
  rend->translate((float)mCenter.x,(float)mCenter.y,(float)mCenter.z);
  CGE::VertexBuffer* vb = rend->createVertexBuffer();
  CGE::IndexBuffer* indbuf = rend->createIndexBuffer(IndexBuffer::IB_USHORT, 24);
  vb->create(VB_POSITION,8);
  vb->lockVertexPointer();
  CGE::Vec3<index> p = mCenter-mCenter;
  vb->setPosition(0,CGE::Vec3f((float)p.x-(float)mSpan.x,(float)p.y-(float)mSpan.y,(float)p.z-(float)mSpan.z));
  vb->setPosition(1,CGE::Vec3f((float)p.x-(float)mSpan.x,(float)p.y-(float)mSpan.y,(float)p.z+(float)mSpan.z));
  vb->setPosition(2,CGE::Vec3f((float)p.x+(float)mSpan.x,(float)p.y-(float)mSpan.y,(float)p.z+(float)mSpan.z));
  vb->setPosition(3,CGE::Vec3f((float)p.x+(float)mSpan.x,(float)p.y-(float)mSpan.y,(float)p.z-(float)mSpan.z));
  vb->setPosition(4,CGE::Vec3f((float)p.x-(float)mSpan.x,(float)p.y+(float)mSpan.y,(float)p.z-(float)mSpan.z));
  vb->setPosition(5,CGE::Vec3f((float)p.x-(float)mSpan.x,(float)p.y+(float)mSpan.y,(float)p.z+(float)mSpan.z));
  vb->setPosition(6,CGE::Vec3f((float)p.x+(float)mSpan.x,(float)p.y+(float)mSpan.y,(float)p.z+(float)mSpan.z));
  vb->setPosition(7,CGE::Vec3f((float)p.x+(float)mSpan.x,(float)p.y+(float)mSpan.y,(float)p.z-(float)mSpan.z));
  vb->unlockVertexPointer();
  short* ind = (short*)indbuf->lockIndexPointer();
  ind[0] = 0; ind[1] = 1; ind[2] = 1; ind[3] = 2; ind[4] = 2; ind[5] = 3; ind[6] = 3; ind[7] = 0;
  ind[8] = 4; ind[9] = 5; ind[10] = 5; ind[11] = 6; ind[12] = 6; ind[13] = 7; ind[14] = 7; ind[15] = 4;
  ind[16] = 0; ind[17] = 4; ind[18] = 1; ind[19] = 5; ind[20] = 2; ind[21] = 6; ind[22] = 3; ind[23] = 7;
  indbuf->unlockIndexPointer();
  vb->activate();
  vb->draw(CGE::VB_Lines, indbuf);
  mVB->activate();
  mVB->draw(CGE::VB_Points, NULL);
  rend->popMatrix();
  delete vb;
  delete indbuf;
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
