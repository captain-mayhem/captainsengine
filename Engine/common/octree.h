#include <cstdlib>
#include "math/vector.h"
#include "system/engine.h"
#include "renderer/renderer.h"

template<typename index, typename value>
class Octree{
public:
  Octree();
  ~Octree();
  void init(const Math::Vec3<index>& center, const Math::Vec3<index>& halfspan){
    mCenter = center; mSpan = halfspan;
  }
  value& at(index x, index y, index z);
  void renderDebug();
protected:
  
  struct Node{
    Node(index x, index y, index z, value val) : mX(x), mY(y), mZ(z), mValue(val){
      for(int i = 0; i < 8; ++i)
        mChildren[i] = NULL;
    };
      ~Node();
      union{
        index mIndex[3];
        struct{
          index mX; 
          index mY; 
          index mZ;
        };
      };
      value mValue;
      Node* mChildren[8];
  };

  Node** recurseCreate(index x, index y, index z, Node** node);

  Node* mRoot;
  value mDefault;
  Math::Vec3<index> mCenter;
  Math::Vec3<index> mSpan;
};

//impl

template<typename index, typename value> 
Octree<index,value>::Octree() : mRoot(NULL), mDefault(0), mCenter(), mSpan() {

}

template<typename index, typename value> 
Octree<index,value>::~Octree(){

}

template<typename index, typename value> 
value& Octree<index,value>::at(index x, index y, index z){
  Node** nd = recurseCreate(x,y,z,&mRoot);
  return (*nd)->mValue;
}

template<typename index, typename value> 
typename Octree<index,value>::Node** Octree<index,value>::recurseCreate(index x, index y, index z, typename Octree<index,value>::Node** node){
  if (*node == NULL){
    *node = new Node(x,y,z,mDefault);
    return node;
  }
  if (x == (*node)->mX && y == (*node)->mY && z == (*node)->mZ)
    return node;
  int access = (x < (*node)->mX ? 0 : 1) << 2;
  access |= (y < (*node)->mY ? 0 : 1) << 1;
  access |= (z < (*node)->mZ ? 0 : 1);
  Node** child = &(*node)->mChildren[access];
  return recurseCreate(x,y,z,child);
}

template<typename index, typename value> 
void Octree<index,value>::renderDebug(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->pushMatrix();
  rend->translate(mCenter.x,mCenter.y,mCenter.z);
  Graphics::VertexBuffer* vb = rend->createVertexBuffer();
  vb->create(VB_POSITION,8,24);
  vb->lockVertexPointer();
  Vec3<index> p = mCenter-mCenter;
  vb->setPosition(0,Graphics::Vertex((float)p.x-(float)mSpan.x,(float)p.y-(float)mSpan.y,(float)p.z-(float)mSpan.z));
  vb->setPosition(1,Graphics::Vertex((float)p.x-(float)mSpan.x,(float)p.y-(float)mSpan.y,(float)p.z+(float)mSpan.z));
  vb->setPosition(2,Graphics::Vertex((float)p.x+(float)mSpan.x,(float)p.y-(float)mSpan.y,(float)p.z+(float)mSpan.z));
  vb->setPosition(3,Graphics::Vertex((float)p.x+(float)mSpan.x,(float)p.y-(float)mSpan.y,(float)p.z-(float)mSpan.z));
  vb->setPosition(4,Graphics::Vertex((float)p.x-(float)mSpan.x,(float)p.y+(float)mSpan.y,(float)p.z-(float)mSpan.z));
  vb->setPosition(5,Graphics::Vertex((float)p.x-(float)mSpan.x,(float)p.y+(float)mSpan.y,(float)p.z+(float)mSpan.z));
  vb->setPosition(6,Graphics::Vertex((float)p.x+(float)mSpan.x,(float)p.y+(float)mSpan.y,(float)p.z+(float)mSpan.z));
  vb->setPosition(7,Graphics::Vertex((float)p.x+(float)mSpan.x,(float)p.y+(float)mSpan.y,(float)p.z-(float)mSpan.z));
  vb->unlockVertexPointer();
  short* ind = vb->lockIndexPointer();
  ind[0] = 0; ind[1] = 1; ind[2] = 1; ind[3] = 2; ind[4] = 2; ind[5] = 3; ind[6] = 3; ind[7] = 0;
  ind[8] = 4; ind[9] = 5; ind[10] = 5; ind[11] = 6; ind[12] = 6; ind[13] = 7; ind[14] = 7; ind[15] = 4;
  ind[16] = 0; ind[17] = 4; ind[18] = 1; ind[19] = 5; ind[20] = 2; ind[21] = 6; ind[22] = 3; ind[23] = 7;
  vb->unlockIndexPointer();
  vb->activate();
  vb->draw(Graphics::VB_Lines);
  rend->popMatrix();
  delete vb;
}