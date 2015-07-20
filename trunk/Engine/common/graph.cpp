#include "graph.h"

using namespace CGE;

GraphNode::GraphNode(){

}

GraphNode::GraphNode(const GraphNode& node){
  succs_ = std::vector<Ptr<GraphNode> >(node.succs_);
  preds_ = std::vector<Ptr<GraphNode> >(node.preds_);
}

GraphNode::~GraphNode(){

}

//--------------------------

Graph::Graph(){

}

Graph::~Graph(){
  std::set<Ptr<GraphNode> >::iterator iter = roots_.begin();
  while(iter != roots_.end()){
    //TODO check correctness
    Ptr<GraphNode> ptr = *iter;
    ptr = NULL;
    ++iter;
  }
}

bool Graph::connect(Ptr<GraphNode> from, Ptr<GraphNode> to){
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
    //do we have a loop
    if (to->succs_.size() != 0){
      from->succs_.push_back(Ptr<GraphNode>(to,true));
    }
    else{
      from->succs_.push_back(to);
      roots_.erase(to);
    }
    to->preds_.push_back(from);
  }
  return unconnected;
}

void Graph::visit(GraphVisitor* visitor){
  std::set<Ptr<GraphNode> >::iterator iter;
  for (iter = roots_.begin(); iter != roots_.end(); ++iter){
    visitor->visit(*iter);
  }
}

//--------------------------

Graph::Iterator::Iterator(const Graph& grph){
  std::set<Ptr<GraphNode> >::const_iterator iter;
  for (iter = grph.roots_.begin(); iter != grph.roots_.end(); ++iter){
    mNodes.push(*iter);
  }
}

Graph::Iterator::~Iterator(){
}

bool Graph::Iterator::hasNext(){
  return !mNodes.empty();
}

Ptr<GraphNode> Graph::Iterator::next(){
  Ptr<GraphNode> result = mNodes.front();
  mNodes.pop();
  for (unsigned i = 0; i < result->succs_.size(); ++i){
    Ptr<GraphNode> succ = result->succs_[i];
    if (!succ.isWeak())
      mNodes.push(succ);
  }
  return result;
}

//--------------------------

GraphVisitor::GraphVisitor(){

}

GraphVisitor::~GraphVisitor(){

}

