#pragma once

#ifndef _VEC3_H_
#define _VEC3_H_

#include <functional>

class Vec3 {
public:
	union { double x, r; };
	union { double y, g; };
	union { double z, b; };
	
	Vec3(double xr = 0.0, double yg = 0.0, double zb = 0.0);
	
	double length() const;
	Vec3 normalized() const;

	double sum() const;
	double dot(const Vec3 & r) const;

	Vec3 cross(const Vec3 r) const;
	Vec3 exp() const;

	Vec3 operator - () const;
	Vec3 operator + (const Vec3 & r) const;
	Vec3 operator - (const Vec3 & r) const;
	Vec3 operator * (double r) const;
	Vec3 operator * (const Vec3 & r) const;
	Vec3 operator / (const Vec3 & r) const;
	Vec3 operator / (double r) const;

	operator std::function<Vec3(Vec3, Vec3)> () const {
		Vec3 copy = *this;
		return [copy](Vec3 pos, Vec3 norm) { return copy; };
	}

};

#endif // _VEC3_H_