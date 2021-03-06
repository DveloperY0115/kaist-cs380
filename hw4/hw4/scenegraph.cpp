#include <algorithm>

#include "scenegraph.h"

using namespace std;

bool SgTransformNode::accept(SgNodeVisitor& visitor) {
    if (!visitor.visit(*this))
        return false;
    for (int i = 0, n = children_.size(); i < n; ++i) {
        if (!children_[i]->accept(visitor))
            return false;
    }
    return visitor.postVisit(*this);
}

void SgTransformNode::addChild(shared_ptr<SgNode> child) {
    children_.push_back(child);
}

void SgTransformNode::removeChild(shared_ptr<SgNode> child) {
    children_.erase(find(children_.begin(), children_.end(), child));
}

bool SgShapeNode::accept(SgNodeVisitor& visitor) {
    if (!visitor.visit(*this))
        return false;
    return visitor.postVisit(*this);
}

class RbtAccumVisitor : public SgNodeVisitor {
protected:
    vector<RigTForm> rbtStack_;
    SgTransformNode& target_;
    bool found_;

public:
    RbtAccumVisitor(SgTransformNode& target)
        : target_(target)
        , found_(false) 
    {
        // push identity
        rbtStack_.push_back(RigTForm());
    }

    const RigTForm getAccumulatedRbt(int offsetFromStackTop = 0) {
        int num_popped = 0;

        while (!rbtStack_.empty() && num_popped < offsetFromStackTop) {
            rbtStack_.pop_back();
            num_popped++;
        }

        return rbtStack_.back();
    }

    virtual bool visit(SgTransformNode& node) {

        rbtStack_.push_back(rbtStack_.back() * node.getRbt());

        if (target_ == node) {
            found_ = true;
            // if target found, stop searching
            return false;
        }

        // otherwise, keep searching
        return true;
    }

    virtual bool postVisit(SgTransformNode& node) {
        if (!found_) {
            // discard all nodes in the middle if target is not found
            rbtStack_.pop_back();
            return true;
        }
        else {
            return false;
        }
    }
};

RigTForm getPathAccumRbt(
    shared_ptr<SgTransformNode> source,
    shared_ptr<SgTransformNode> destination,
    int offsetFromDestination) {

    RbtAccumVisitor accum(*destination);
    source->accept(accum);
    return accum.getAccumulatedRbt(offsetFromDestination);
}
