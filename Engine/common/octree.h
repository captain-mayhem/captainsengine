#include <cstdlib>

template<typename index, typename value>
class Octree{
public:
  Octree();
  ~Octree();
  value& at(index x, index y, index z);
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
};

//impl

template<typename index, typename value> 
Octree<index,value>::Octree() : mRoot(NULL), mDefault(0) {

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