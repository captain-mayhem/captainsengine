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

#include <vector>
#include <set>

#include <common/RefCountedObject.h>

namespace CGE{

class GraphVisitor;

class GraphNode : public RefCountedObject{
  friend class Graph;
public:
  GraphNode();
  ~GraphNode();
//protected:
  std::vector<Ptr<GraphNode> > preds_;
  std::vector<Ptr<GraphNode> > succs_;
};

class Graph{
public:
  Graph();
  ~Graph();
  void addSingleNode(Ptr<GraphNode> node) {roots_.insert(node);}
  void connect(Ptr<GraphNode> from, Ptr<GraphNode> to);
  void visit(GraphVisitor* visitor);
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

