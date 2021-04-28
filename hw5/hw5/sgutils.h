#ifndef SGUTILS_H
#define SGUTILS_H

#include <vector>

#include "scenegraph.h"

//!
//! RbtNodesScanner for dumping RigTForm objects handled by
//! each RbtNode in the scene graph
struct RbtNodesScanner : public SgNodeVisitor {
  typedef std::vector<std::shared_ptr<SgRbtNode>> SgRbtNodes;

  SgRbtNodes& nodes_;

  RbtNodesScanner(SgRbtNodes& nodes) : nodes_(nodes) {
      assert(nodes_.empty());
  }

  virtual bool visit(SgTransformNode& node) {
    using namespace std;
    shared_ptr<SgRbtNode> rbtPtr = dynamic_pointer_cast<SgRbtNode>(node.shared_from_this());
    if (rbtPtr)
      nodes_.push_back(rbtPtr);
    return true;
  }
};

//!
//! dumpSgRbtNodes
//! Dump all RbtNodes in the scene into a vector
inline void dumpSgRbtNodes(std::shared_ptr<SgNode> root, std::vector<std::shared_ptr<SgRbtNode>>& rbtNodes) {
  RbtNodesScanner scanner(rbtNodes);
  root->accept(scanner);
}

//!
//! setSgRbtNodes
//! Set all RbtNodes in the scene with the values stored in the given vector

using Frame = std::vector<RigTForm>;

inline void setSgRbtNodes(std::vector<std::shared_ptr<SgRbtNode>>& rbtNodes, Frame& frame) {
    assert(rbtNodes.size() == frame.size());
    for (int idx = 0; idx < rbtNodes.size(); ++idx) {
        rbtNodes[idx]->setRbt(frame[idx]);
    }
}

#endif