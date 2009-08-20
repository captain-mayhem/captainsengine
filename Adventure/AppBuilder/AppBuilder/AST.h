#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

class ASTNode{
public:
  ASTNode(){}
  virtual ~ASTNode(){}
};

class NodeList{
public:
  NodeList(){}
  virtual ~NodeList(){
    for (unsigned i = 0; i < mNodes.size(); ++i){
      delete mNodes[i];
    }
  }
  void addNode(ASTNode* node){
    mNodes.push_back(node);
  }
protected:
  std::vector<ASTNode*> mNodes;
};

class StmtNode : public ASTNode{
public:
  StmtNode(){}
  virtual ~StmtNode(){}
};

class IdentNode : public ASTNode{
public:
  IdentNode(const std::string& value) {mValue = value;}
  virtual ~IdentNode(){}
  std::string& value() {return mValue;}
protected:
  std::string mValue;
};

class IntNode : public ASTNode{
public:
  IntNode(int value) {
    mValue = value;
  }
  virtual ~IntNode(){}
  int& value() {return mValue;}
protected:
  int mValue;
};

class FuncCallNode : public StmtNode{
public:
  FuncCallNode(std::string func, NodeList* args){
    mFunc = func;
    mArgs = args;
  }
  ~FuncCallNode(){
    delete mArgs;
  }
protected:
  std::string mFunc;
  NodeList* mArgs;
};

#endif
