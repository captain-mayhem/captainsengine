//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |             The simple but powerful game engine                  |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | 2008-03-25                                                       |
// |------------------------------------------------------------------|
// | File: graph.h                                                    |
//  ==================================================================

#ifndef CGE_GRAPHNODE_H
#define CGE_GRAPHNODE_H

#include <vector>
#include <set>
#include <queue>

#include <common/RefCountedObject.h>

namespace CGE{

class GraphVisitor;

class GraphNode : public RefCountedObject{
  friend class Graph;
public:
  GraphNode();
  GraphNode(const GraphNode& node);
  virtual ~GraphNode();
//protected:
  std::vector<Ptr<GraphNode> > preds_;
  std::vector<Ptr<GraphNode> > succs_;
};

class Graph{
public:
  class Iterator{
  public:
    Iterator(const Graph& grph);
    ~Iterator();
    bool hasNext();
    Ptr<GraphNode> next();
  protected:
    std::queue<Ptr<GraphNode> > mNodes;
  };
  Graph();
  ~Graph();
  void addSingleNode(Ptr<GraphNode> node) {roots_.insert(node);}
  bool connect(Ptr<GraphNode> from, Ptr<GraphNode> to);
  void visit(GraphVisitor* visitor);
  Iterator getIterator();
protected:
  std::set<Ptr<GraphNode> > roots_;

};

class GraphVisitor{
public:
  GraphVisitor();
  virtual ~GraphVisitor();
  virtual void* visit(Ptr<GraphNode> node)=0;
protected:
};

}

#endif
