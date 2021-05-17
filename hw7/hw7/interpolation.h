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
		const Cvec3 interTrans = lerp(rbt0.getTranslation(), rbt1.getTranslation(), alpha);
		const Quat interRot = slerp(rbt0.getRotation(), rbt1.getRotation(), alpha);
		return RigTForm(interTrans, interRot);
	}


	//! Bezier spline for vectors
	inline Cvec3 Bezier(Cvec3 c0, Cvec3 d0, Cvec3 e0, Cvec3 c1, const double& alpha) {
		const Cvec3 f = lerp(c0, d0, alpha);
		const Cvec3 g = lerp(d0, e0, alpha);
		const Cvec3 h = lerp(e0, c1, alpha);
		const Cvec3 m = lerp(f, g, alpha);
		const Cvec3 n = lerp(g, h, alpha);
		return lerp(m, n, alpha);
	}

	//! Bezier spline for quaternions
	inline Quat Bezier(Quat c0, Quat d0, Quat e0, Quat c1, const double& alpha) {
		const Quat f = slerp(c0, d0, alpha);
		const Quat g = slerp(d0, e0, alpha);
		const Quat h = slerp(e0, c1, alpha);
		const Quat m = slerp(f, g, alpha);
		const Quat n = slerp(g, h, alpha);
		return slerp(m, n, alpha);
	}


	//! Catmull-Rom interpolation of two RigTForms
	//! To interpolate using Catmull-Rom spline, one needs 4 Keyframes (-1, 0, 1, 2)
	inline RigTForm CatmullRom(const RigTForm& rbt_i, const RigTForm& rbt0, const RigTForm& rbt1, const RigTForm& rbt_f, const double& alpha) {

		// initialize translation, rotation components;
		Cvec3 interTrans = Cvec3();
		Quat interRot = Quat();

		// translation interpolation
		const Cvec3 c0 = rbt0.getTranslation();
		const Cvec3 c1 = rbt1.getTranslation();
		const Cvec3 d0_vec = (c1 - rbt_i.getTranslation()) * (1 / static_cast<double>(6)) + c0;
		const Cvec3 e0_vec = (rbt_f.getTranslation() - c0) * (-1 / static_cast<double>(6)) + c1;
		interTrans = Bezier(c0, d0_vec, e0_vec, c1, alpha);

		// rotation interpolation
		const Quat q0 = rbt0.getRotation();
		const Quat q1 = rbt1.getRotation();
		const Quat d0_quat = pow(q1 * inv(rbt_i.getRotation()), 1 / static_cast<double>(6)) * q0;
		const Quat e0_quat = pow(rbt_f.getRotation() * inv(q0), -1 / static_cast<double>(6)) * q1;

		interRot = Bezier(q0, d0_quat, e0_quat, q1, alpha);

		return RigTForm(interTrans, interRot);
	}
}
#endif