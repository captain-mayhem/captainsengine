#include "graph.h"

using namespace CGE;

GraphNode::GraphNode(){

}

GraphNode::~GraphNode(){

}

//--------------------------

Graph::Graph(){

}

Graph::~Graph(){
  std::set<Ptr<GraphNode> >::iterator iter = roots_.begin();
  while(iter != roots_.end()){
    *iter = NULL;
    ++iter;
  }
}

void Graph::connect(Ptr<GraphNode> from, Ptr<GraphNode> to){
  //check if to is a root, then to is no longer root
  roots_.erase(to);
  //check if from has predecessors, otherwise it is a root
  if (from->preds_.size() == 0){
    roots_.insert(from);
  }
  //already connected?
  bool unconnected = true;
  for (unsigned i = 0; i < to->preds_.size(); ++i){
    if (to->preds_[i] == from){
      unconnected = false;
    }
  }
  //connect the two nodes
  if (unconnected){
    from->succs_.push_back(to);
    to->preds_.push_back(from);
  }
}

void Graph::visit(GraphVisitor* visitor){
  std::set<Ptr<GraphNode> >::iterator iter;
  for (iter = roots_.begin(); iter != roots_.end(); ++iter){
    visitor->visit(*iter);
  }
}

//--------------------------

GraphVisitor::GraphVisitor(){

}

GraphVisitor::~GraphVisitor(){

}

