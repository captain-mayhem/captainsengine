#ifndef GRAPHNODES_H
#define GRAPHNODES_H

#include "Graph.h"

class TextNode : public Graph::Node{
public:
  TextNode(std::string label) : mLabel(label) {}
  virtual ~TextNode() {}
  const std::string& getLabel() const {return mLabel;}
protected:
  std::string mLabel;
};

#endif
