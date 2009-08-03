#ifndef GRAPH_H
#define GRAPH_H

#include <set>

class Graph{
public:
  class Node;
  
  class Connection{
  public:
    Connection(Node* from, Node* to);
    ~Connection();
    Node* getFrom() {return mFrom;}
    Node* getTo() {return mTo;}
  protected:
    Node* mFrom;
    Node* mTo;
  };
  
  class Node{
    friend class Connection;
  public:
    Node();
    virtual ~Node();
    Node* getPredecessor() {return (*mPreds.begin())->getFrom();}
    const std::set<Connection*>& getConnections() const {return mSuccs;}
  protected:
    std::set<Connection*> mSuccs;
    std::set<Connection*> mPreds;
  };
  
  Graph();
  ~Graph();
  void setRoot(Node* node);
  void addNode(Node* node);
  void addConnection(Connection* conn);
  Node* getRoot() const {return mRoot;}
protected:
  std::set<Node*> mNodes;
  std::set<Connection*> mConnections;
  Node* mRoot;
};

#endif
