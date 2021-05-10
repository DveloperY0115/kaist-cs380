#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include "rigtform.h"

namespace Interpolation {

	//! Linear interpolation of two coordinate vectors
	inline Cvec3 lerp(const Cvec3& c0, const Cvec3& c1, const double& alpha) {
		return c0 * (1 - alpha) + c1 * alpha;
	}

	//! Spherical interpolation of two quaternions
	inline Quat slerp(const Quat& q0, const Quat& q1, const double& alpha) {
		Quat base = q1 * inv(q0);

		if (base(0) < 0) {
			// conditionally negate q1 * inv(q0) if the first
			// element is negative
			base *= -1;
		}

		return pow(base, alpha) * q0;
	}

	//! Linear interpolation of two RigTForms
	inline RigTForm Linear(const RigTForm& rbt0, const RigTForm& rbt1, const double& alpha) {
		Cvec3 interTrans = lerp(rbt0.getTranslation(), rbt1.getTranslation(), alpha);
		Quat interRot = slerp(rbt0.getRotation(), rbt1.getRotation(), alpha);
		return RigTForm(interTrans, interRot);
	}
}
#endif