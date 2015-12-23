#pragma once

#ifndef _CONFIGURER_H_
#define _CONFIGURER_H_

#include <string>
#include "Body.h"

class Configurer {

public:

	Configurer();
	~Configurer();

	bool load(std::string path);

	// Resolution
	int window_width;
	int window_height;

	// AppSystem
	std::string window_title;
	int threads_count;
	int framerate;

	// Render camera settings
	Vec3 cam_position;
	Vec3 cam_forward;
	Vec3 cam_uppy;
	Vec3 cam_up;
	Vec3 cam_right;
	double cam_width;
	double cam_height;
	double cam_depth;

	// Render settings
	double exposure;
	int aa_factor;

	// Tracer settings
	int max_bounce;
	int diffuse_bounce_value;
	int refractive_bounce_value;
	int reflective_bounce_value;
	int diffuse_ray_count;
	double tracer_bias;

	// Scene
	std::vector<Body> bodies;

private:

};

#endif // _CONFIGURER_H_