#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

class ASTNode{
public:
  enum Type{
    UNDEFINED,
    IDENTIFIER,
    INTEGER,
    FUNCCALL,
  };
  ASTNode(Type t) : mType(t) {}
  virtual ~ASTNode(){}
  Type getType() {return mType;}
protected:
  Type mType;
};

class NodeList{
public:
  NodeList() : mIdx(0){}
  virtual ~NodeList(){
    for (unsigned i = 0; i < mNodes.size(); ++i){
      delete mNodes[i];
    }
  }
  void addNode(ASTNode* node){
    mNodes.push_back(node);
  }
  void reset(bool start) {
    if (start)
      mIdx = 0;
    else
      mIdx = (unsigned)mNodes.size();
  }
  bool hasNext() {return mIdx < mNodes.size();}
  bool hasPrev() {return mIdx > 0;}
  ASTNode* next() {
    if(hasNext())
      return mNodes[mIdx++];
    return NULL;
  }
  ASTNode* prev(){
    if (hasPrev())
      return mNodes[--mIdx];
    return NULL;
  };
  unsigned size(){
    return (unsigned)mNodes.size();
  }
protected:
  std::vector<ASTNode*> mNodes;
  unsigned mIdx;
};

class StmtNode : public ASTNode{
public:
  StmtNode(Type t) : ASTNode(t){}
  virtual ~StmtNode(){}
};

class IdentNode : public ASTNode{
public:
  IdentNode(const std::string& value) : ASTNode(IDENTIFIER) {mValue = value;}
  virtual ~IdentNode(){}
  std::string& value() {return mValue;}
protected:
  std::string mValue;
};

class IntNode : public ASTNode{
public:
  IntNode(int value) : ASTNode(INTEGER) {
    mValue = value;
  }
  virtual ~IntNode(){}
  int& value() {return mValue;}
protected:
  int mValue;
};

class FuncCallNode : public StmtNode{
public:
  FuncCallNode(std::string func, NodeList* args) : StmtNode(FUNCCALL) {
    mFunc = func;
    mArgs = args;
  }
  ~FuncCallNode(){
    delete mArgs;
  }
  std::string getName() {return mFunc;}
  NodeList* getArguments() {return mArgs;}
protected:
  std::string mFunc;
  NodeList* mArgs;
};

#endif
