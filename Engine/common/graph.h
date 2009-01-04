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

namespace Common{

class GraphVisitor;

class GraphNode{
  friend class Graph;
public:
  GraphNode();
  ~GraphNode();
//protected:
  std::vector<GraphNode*> preds_;
  std::vector<GraphNode*> succs_;
};

class Graph{
public:
  Graph();
  ~Graph();
  void addSingleNode(GraphNode* node) {roots_.insert(node);}
  void connect(GraphNode* from, GraphNode* to);
  void visit(GraphVisitor* visitor);
protected:
  std::set<GraphNode*> roots_;

};

class GraphVisitor{
public:
  GraphVisitor();
  virtual ~GraphVisitor();
  virtual void* visit(GraphNode* node)=0;
protected:
};

}