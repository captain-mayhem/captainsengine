
#include "Graph.h"

#include <cstdlib>

Graph::Connection::Connection(Node* from, Node* to){
  mFrom = from;
  mTo = to;
  mFrom->mSuccs.insert(this);
  mTo->mPreds.insert(this);
}

Graph::Connection::~Connection(){

}

Graph::Node::Node(){

}

Graph::Node::~Node(){

}

Graph::Graph() : mRoot(NULL){

}

Graph::~Graph(){

}

void Graph::setRoot(Node* node){
  addNode(node);
  mRoot = node;
}

void Graph::addNode(Node* node){
  mNodes.insert(node);
}

void Graph::addConnection(Connection* conn){
  addNode(conn->getFrom());
  addNode(conn->getTo());
  mConnections.insert(conn);
}
