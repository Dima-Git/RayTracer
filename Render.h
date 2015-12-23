#pragma once

#ifndef _RENDER_H_
#define _RENDER_H_

#include "Configurer.h"
#include "Vec3.h"
#include "Tracer.h"

class Render {

public:

	Render(const Configurer & config);
	Vec3 pixel_color(int x, int y) const;

private:

	Tracer tracer;

	int SCREEN_WIDTH;
	int SCREEN_HEIGHT;
	Vec3 CAM_POSITION;
	Vec3 CAM_FORWARD;
	Vec3 CAM_UPPY;
	Vec3 CAM_UP;
	Vec3 CAM_RIGHT;
	double CAM_WIDTH;
	double CAM_HEIGHT;
	double CAM_DEPTH;

	double EXPOSURE;
	int AA_FACTOR;

};

#endif // _RENDER_H_