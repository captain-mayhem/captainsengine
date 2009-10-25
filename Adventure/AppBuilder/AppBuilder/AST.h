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
    EVENT,
    CONDITION,
    VARIABLE,
    RELATIONAL,
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
  IdentNode(const std::string& value) : ASTNode(IDENTIFIER) {append(value.c_str());}
  virtual ~IdentNode(){}
  void append(const char* str) {
    std::string tmp(str);
    if (!mValue.empty() || tmp != " ")
      mValue += str;
  }
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

class EventNode : public StmtNode{
public:
  EventNode(const std::string& event, NodeList* eventblock) : StmtNode(EVENT){
    mEvent = event;
    mBlock = eventblock;
  }
  ~EventNode(){
    delete mBlock;
  }
  std::string getEvent() {return mEvent;}
  NodeList* getBlock() {return mBlock;}
protected:
  std::string mEvent;
  NodeList* mBlock;
};

class CondNode : public StmtNode{
public:
  CondNode(bool negation) : StmtNode(CONDITION), mNegation(negation){
  }
  ~CondNode(){
    delete mArgs;
    delete mIfBlock;
  }
  void setCondition(const std::string& condition) {mCondition = condition;}
  void setCondArgs(NodeList* args) {mArgs = args;}
  void setIfBlock(NodeList* ifblock) {mIfBlock = ifblock;}
  std::string getCondFuncName() {return "if_"+mCondition;}
  NodeList* getArguments() {return mArgs;}
  NodeList* getIfBlock() {return mIfBlock;}
  bool isNegated() {return mNegation;}
protected:
  bool mNegation;
  std::string mCondition;
  NodeList* mArgs;
  NodeList* mIfBlock;
};

class VariableNode : public ASTNode{
public:
  VariableNode(const std::string& name) : ASTNode(VARIABLE) {mName = name;}
  virtual ~VariableNode(){}
  std::string& name() {return mName;}
protected:
  std::string mName;
};

class RelationalNode : public ASTNode{
public:
  enum Type{
    REL_EQUAL,
    REL_LESS,
    REL_GREATER,
    REL_PLUS,
    REL_MINUS,
  };
  RelationalNode() : ASTNode(RELATIONAL), mType(REL_EQUAL), mChild(NULL){}
  virtual ~RelationalNode(){
    delete mChild;
  }
  Type& type() {return mType;}
  ASTNode*& child() {return mChild;}
protected:
  Type mType;
  ASTNode* mChild;
};

#endif
