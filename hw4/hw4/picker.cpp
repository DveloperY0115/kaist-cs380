#include <GL/glew.h>

#include "picker.h"

using namespace std;

Picker::Picker(const RigTForm& initialRbt, const ShaderState& curSS, std::vector<std::shared_ptr<SgRbtNode>> robots)
  : drawer_(initialRbt, curSS), idCounter_(0), isRobot(false), srgbFrameBuffer_(!g_Gl2Compatible) {
    robot_1 = robots[0];
    robot_2 = robots[1];
}

bool Picker::visit(SgTransformNode& node) {
    if (node.shared_from_this() == robot_1 || node.shared_from_this() == robot_2) {
        isRobot = true;
    }

    if (isRobot) {
        // add node to stack only if it belongs to robot graph
        nodeStack_.push_back(node.shared_from_this());
    }

    return drawer_.visit(node);
}

bool Picker::postVisit(SgTransformNode& node) {
    if (isRobot) {
        nodeStack_.pop_back();
    }

    if (node.shared_from_this() == robot_1 || node.shared_from_this() == robot_2) {
        isRobot = false;
    }

    return drawer_.postVisit(node);
}

/*
* Picker::visit
* When reached ShapeNode, find its parent node containing rigid body transform
* associated with it. Then map its unique ID and reference to the 'idToRbtNode'.
*/
bool Picker::visit(SgShapeNode& node) {
    
    if (isRobot) {
        // map its parent representing RBT
        idCounter_ += 1;
        std::shared_ptr<SgRbtNode> parent = std::dynamic_pointer_cast<SgRbtNode>(nodeStack_.back());
        addToMap(idCounter_, parent);

        // set the color of the geometry uniquely
        Cvec3 color = idToColor(idCounter_);
        safe_glUniform3f(drawer_.getCurSS().h_uIdColor, color(0), color(1), color(2));
    }
    return drawer_.visit(node);
}

/*
* Picker::postVisit
* Clean up uniform variable after draw call
*/
bool Picker::postVisit(SgShapeNode& node) {
    safe_glUniform3f(drawer_.getCurSS().h_uIdColor, 0, 0, 0);
    return drawer_.postVisit(node);
}

shared_ptr<SgRbtNode> Picker::getRbtNodeAtXY(int x, int y) {
    PackedPixel color;
    glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &color);
    int id = colorToId(color);

    std::cout << "Picked color: " << (unsigned int)color.r << " " << (unsigned int)color.g << " " << (unsigned int)color.b << "||";
    std::cout << "ID: " << id << "\n";
    describeMap();

    shared_ptr<SgRbtNode> rbt_node = find(id);
    if (rbt_node == nullptr) {
        std::cout << "Something's wrong. It's not a registered node!\n";
    }

    return rbt_node;
}

//------------------
// Helper functions
//------------------
//

/*
* describeStack
* 
* Describes the current stack (used for debugging)
* 
* Prints out the ID and associated nodes in the stack
*/
void Picker::describeStack() {
    for (auto& node : nodeStack_) {
        std::cout << "ID: " << idCounter_ << node.get() << "\n";
    }
}

/*
* describeMap
* 
* Describes the current status of map (ID - SgRbtNode)
*/
void Picker::describeMap() {
    for (auto const& x : idToRbtNode_) {
        std::cout << "ID: " << x.first << "||";
        std::cout << "Color: " << idToColor(x.first)(0) << " " << idToColor(x.first)(1) << " " << idToColor(x.first)(2) << "\n";
    }
}

void Picker::addToMap(int id, shared_ptr<SgRbtNode> node) {
  idToRbtNode_[id] = node;
}

shared_ptr<SgRbtNode> Picker::find(int id) {
  IdToRbtNodeMap::iterator it = idToRbtNode_.find(id);
  if (it != idToRbtNode_.end())
    return it->second;
  else
    return shared_ptr<SgRbtNode>(); // set to null
}

// encode 2^4 = 16 IDs in each of R, G, B channel, for a total of 16^3 number of objects
static const int NBITS = 4, N = 1 << NBITS, MASK = N-1;

Cvec3 Picker::idToColor(int id) {
  assert(id > 0 && id < N * N * N);
  Cvec3 framebufferColor = Cvec3(id & MASK, (id >> NBITS) & MASK, (id >> (NBITS+NBITS)) & MASK);
  framebufferColor = framebufferColor / N + Cvec3(0.5/N);

  if (!srgbFrameBuffer_)
    return framebufferColor;
  else {
    // if GL3 is used, the framebuffer will be in SRGB format, and the color we supply needs to be in linear space
    Cvec3 linearColor;
    for (int i = 0; i < 3; ++i) {
      linearColor[i] = framebufferColor[i] <= 0.04045 ? framebufferColor[i]/12.92 : pow((framebufferColor[i] + 0.055)/1.055, 2.4);
    }
    return linearColor;
  }
}

int Picker::colorToId(const PackedPixel& p) {
  const int UNUSED_BITS = 8 - NBITS;
  int id = p.r >> UNUSED_BITS;
  id |= ((p.g >> UNUSED_BITS) << NBITS);
  id |= ((p.b >> UNUSED_BITS) << (NBITS+NBITS));
  return id;
}
