#ifndef RIGTFORM_H
#define RIGTFORM_H

#include <iostream>
#include <cassert>

#include "matrix4.h"
#include "quat.h"

class RigTForm {
  Cvec3 t_; // translation component
  Quat r_;  // rotation component represented as a quaternion

public:
  RigTForm() : t_(0) {
    assert(norm2(Quat(1,0,0,0) - r_) < CS175_EPS2);
  }

  // Constructor
  RigTForm(const Cvec3& t, const Quat& r) {
      t_ = t;
      r_ = r;
  }

  explicit RigTForm(const Cvec3& t) {
      t_ = t;
      assert(norm2(Quat(1, 0, 0, 0) - r_) < CS175_EPS2);
  }

  explicit RigTForm(const Quat& r) {
      t_ = Cvec3();    // zero vector in 3D
      r_ = r;
  }

  Cvec3 getTranslation() const {
    return t_;
  }

  Quat getRotation() const {
    return r_;
  }

  RigTForm& setTranslation(const Cvec3& t) {
    t_ = t;
    return *this;
  }

  RigTForm& setRotation(const Quat& r) {
    r_ = r;
    return *this;
  }

  Cvec4 operator * (const Cvec4& a) const {
    // TODO -> What's the behavior of this operation?
    
  }

  RigTForm operator * (const RigTForm& a) const {
    // TODO
  }
};

inline RigTForm inv(const RigTForm& tform) {
    // get translation and rotation factors
    Cvec3 t_= tform.getTranslation();
    Quat r_ = tform.getRotation();

    Cvec3 inv_t_ = -t_;    // inverse of the translation, simply negate it
    Quat inv_r_ = Quat(r_[0], -r_[1], -r_[2], -r_[3]);    // inverse of the rotation, negate the axis

    return RigTForm(inv_t_, inv_r_);
}

inline RigTForm transFact(const RigTForm& tform) {
  return RigTForm(tform.getTranslation());
}

inline RigTForm linFact(const RigTForm& tform) {
  return RigTForm(tform.getRotation());
}

inline Matrix4 rigTFormToMatrix(const RigTForm& tform) {
  // TODO
  return Matrix4();
}

#endif
