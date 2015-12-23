#pragma once

#ifndef _RAY_H_
#define _RAY_H_

#include "Vec3.h"

class Ray {

public:

	Ray(Vec3 origin, Vec3 direction, int bounce = 0);
	
	int bounce;
	Vec3 origin;
	Vec3 direction;

private:

};

#endif // _RAY_H_