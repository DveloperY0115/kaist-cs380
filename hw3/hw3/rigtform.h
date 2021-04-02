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
      // Note that a unit norm quaternion of form (1, 0, 0, 0) represents identity rotation in 3D
        assert(norm2(Quat(1,0,0,0) - r_) < CS175_EPS2);
  }

  // Constructor
  RigTForm(const Cvec3& t, const Quat& r) {
      t_ = t;
      r_ = r;
  }

  explicit RigTForm(const Cvec3& t) {
      t_ = t;
      // Note that a unit norm quaternion of form (1, 0, 0, 0) represents identity rotation in 3D
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

  /* 
  * Apply RBT represented by this object to vector 'a'
  * 
  * Input: Cvec4 object (either representing a coordinate or a vector)
  * Output: Cvec4 object (RBT applied)
  * 
  * Note:
  * - If input is a vector, translation is not applied since it's an undefined behavior
  * 
  * Exception:
  * - Throws exception when Cvec4 doesn't represent neither coordinate nor vector in Affine frame
  */
  Cvec4 operator * (const Cvec4& a) const {
      assert(a[3] == 0 || a[3] == 1);

      // if 'a' is a coordinate, translate it
      // otherwise, do nothing
      Cvec4 t_a = (*this).getRotation() * a;

      if (a[3] == 1) {
          // 'a' represents a coordinate in Affine form
          Cvec3 t_ = (*this).getTranslation();
          Cvec4 t = Cvec4(t_[0], t_[1], t_[2], 0);
          t_a += t;
      }

      return t_a;
  }

  /*
  * Calculate RigTForm object representing the compound RBT of two RBTs
  */
  RigTForm operator * (const RigTForm& a) const {
      // get t_1 and t_2 
      Cvec3 t_1_ = (*this).getTranslation();
      Cvec4 t_1 = Cvec4(t_1_[0], t_1_[1], t_1_[2], 0);
      Cvec3 t_2_ = a.getTranslation();
      Cvec4 t_2 = Cvec4(t_2_[0], t_2_[1], t_2_[2], 0);

      // get r_1 and r_2
      Quat r_1 = (*this).getRotation();
      Quat r_2 = a.getRotation();

      // Calculate translation part
      Cvec4 trans = t_1 + r_1 * t_2;
      assert(trans[3] == 0);

      Cvec3 t_ = Cvec3(trans[0], trans[1], trans[2]);

      /* 
      * Calculate rotation part
      * multiplication of two quaternion is equal to 
      * the matrix multiplication of two rotation matrices represented by these two
      */
      Quat r_ = r_1 * r_2;    

      return RigTForm(t_, r_);
  }
};

inline RigTForm inv(const RigTForm& tform) {
    // NOTE! You need to work again on this one. Refer to page 71 of textbook
    // get translation and rotation factors
    Cvec3 t_= tform.getTranslation();
    Quat r_ = tform.getRotation();

    Cvec3 inv_t_ = -t_;    // inverse of the translation, simply negate it
    Quat inv_r_ = inv(r_);    // inverse of the rotation

    return RigTForm(inv_t_, inv_r_);
}

inline RigTForm transFact(const RigTForm& tform) {
  return RigTForm(tform.getTranslation());
}

inline RigTForm linFact(const RigTForm& tform) {
  return RigTForm(tform.getRotation());
}

inline Matrix4 rigTFormToMatrix(const RigTForm& tform) {
    // get translation, rotation factors
    Cvec3 t_ = tform.getTranslation();    // T (vector)
    Matrix4 r_mat = quatToMatrix(tform.getRotation());    // R

    Matrix4 t_mat = Matrix4();

    // fill diagonal elements with 1
    t_mat(0, 0) = 1;
    t_mat(1, 1) = 1;
    t_mat(2, 2) = 1;
    t_mat(3, 3) = 1;

    t_mat(0, 3) = t_[0];    // t_x
    t_mat(1, 3) = t_[1];    // t_y
    t_mat(2, 3) = t_[2];    // t_z
    
    Matrix4 RBT_mat = t_mat * r_mat;    // TR
    assert(isAffine(RBT_mat));
    return RBT_mat;
}

#endif
