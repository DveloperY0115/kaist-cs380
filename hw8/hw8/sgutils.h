#ifndef SGUTILS_H
#define SGUTILS_H

#include <vector>

#include "scenegraph.h"

//!
//! RbtNodesScanner for dumping RigTForm objects handled by
//! each RbtNode in the scene graph
struct RbtNodesScanner : public SgNodeVisitor {

    std::vector<std::shared_ptr<SgRbtNode>>& nodes_;

  RbtNodesScanner(std::vector<std::shared_ptr<SgRbtNode>>& nodes) : nodes_(nodes) {
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
//! dumpFrame
//! Dump all RigTForm kept by SgRbtNodes in the SceneRbtVector into a vector 
inline void dumpFrame(std::vector<std::shared_ptr<SgRbtNode>>& rbtNodes, std::vector<RigTForm>& frame) {
    assert(frame.empty());
    for (std::vector<std::shared_ptr<SgRbtNode>>::iterator iter = rbtNodes.begin(); iter != rbtNodes.end(); ++iter) {
        frame.push_back((*iter)->getRbt());
    }
}

//!
//! setSgRbtNodes
//! Set all RbtNodes in the scene with the values stored in the given vector
inline void setSgRbtNodes(std::vector<std::shared_ptr<SgRbtNode>>& rbtNodes, const std::vector<RigTForm>& frame) {
    assert(rbtNodes.size() == frame.size());
    for (unsigned int idx = 0; idx < rbtNodes.size(); ++idx) {
        rbtNodes[idx]->setRbt(frame[idx]);
    }
}

#endif