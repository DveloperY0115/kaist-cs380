#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include "rigtform.h"

namespace Interpolation {

	//! Linear interpolation of two coordinate vectors
	inline Cvec3 lerp(Cvec3 c0, Cvec3 c1, double alpha) {
		return c0 * (1 - alpha) + c1 * alpha;
	}

	//! Spherical interpolation of two quaternions
	inline Quat slerp(Quat q0, Quat q1, double alpha) {
		Quat base = q1 * inv(q0);

		if (base(0) < 0) {
			// conditionally negate q1 * inv(q0) if the first
			// element is negative
			base *= -1;
		}

		return pow(base, alpha) * q0;
	}
}
#endif