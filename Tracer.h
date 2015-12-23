#pragma once

#ifndef _TRACER_H_
#define _TRACER_H_

#include <vector>
#include "Configurer.h"
#include "Body.h"
#include "Ray.h"

class Tracer {

public:
	
	Tracer(const Configurer & config);
	~Tracer();
	Vec3 trace(Ray ray) const;

private:
	
	std::vector<Body> bodies;
	std::function<Vec3(Vec3)> environment;
	int MAX_BOUNCE;
	int DIFFUSE_RAY_COUNT;
	int DIFFUSE_BOUNCE_VALUE;
	int REFRACTIVE_BOUNCE_VALUE;
	int REFLECTIVE_BOUNCE_VALUE;
	double BIAS;

	double get_nearest_hit(Ray ray, int * body_number = nullptr, Vec3 * hit = nullptr, Vec3 * normal = nullptr) const;
	int find_medium(Ray ray) const;
};

#endif // _TRACER_H_