#include "Vec3.h"
#include <math.h>

Vec3::Vec3(double x, double y, double z) 
	: x(x), y(y), z(z) 
{
}

double Vec3::length() const
{
	return sqrt(this->dot(*this));
}

Vec3 Vec3::normalized() const
{
	double len = this->length();
	if (abs(len) < 1e-6) return Vec3(1.0, 0.0, 0.0);
	return Vec3(x/len, y/len, z/len);
}

double Vec3::sum() const { 
	return x + y + z; 
}

double Vec3::dot(const Vec3 & r) const { 
	return (*this * r).sum(); 
}

Vec3 Vec3::cross(const Vec3 r) const
{
	return Vec3(y*r.z - z*r.y, z*r.x - x*r.z, x*r.y - y*r.x);
}

Vec3 Vec3::exp() const
{
	return Vec3(::exp(x), ::exp(y), ::exp(z));
}

Vec3 Vec3::operator - () const { 
	return Vec3(-x, -y, -z); 
}

Vec3 Vec3::operator + (const Vec3 & r) const { 
	return Vec3(x + r.x, y + r.y, z + r.z); 
}

Vec3 Vec3::operator - (const Vec3 & r) const { 
	return *this + -r; 
}

Vec3 Vec3::operator * (double r) const { 
	return Vec3(x * r, y * r, z * r); 
}

Vec3 Vec3::operator * (const Vec3 & r) const { 
	return Vec3(x * r.x, y * r.y, z * r.z); 
}

Vec3 Vec3::operator / (const Vec3 & r) const {
	return Vec3(x / r.x, y / r.y, z / r.z);
}

Vec3 Vec3::operator / (double r) const { 
	return Vec3(x / r, y / r, z / r); 
}
