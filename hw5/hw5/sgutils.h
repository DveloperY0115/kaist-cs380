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
//! RbtNodesSetter for setting the value of RigTForm objects 
//! kept by RbtNodes in the scene
struct RbtNodesSetter : public SgNodeVisitor {
    typedef std::vector<RigTForm> Rbts;

    Rbts& rbts_;
    int idx_;

    RbtNodesSetter(Rbts& rbts) : rbts_(rbts) {
        assert(!rbts_.empty());
        idx_ = 0;
    }

    virtual bool visit(SgTransformNode& node) {
        std::shared_ptr<SgRbtNode> rbtPtr = std::dynamic_pointer_cast<SgRbtNode>(node.shared_from_this());
        if (rbtPtr) {
            rbtPtr->setRbt(rbts_[idx_]);
            idx_++;
        }
        return true;
    }
};

//!
//! setSgRbtNodes
//! Set all RbtNodes in the scene with the values stored in the given vector
inline void setSgRbtNodes(std::shared_ptr<SgNode> root, std::vector<RigTForm>& rbts) {
    RbtNodesSetter setter(rbts);
    root->accept(setter);
}

#endif