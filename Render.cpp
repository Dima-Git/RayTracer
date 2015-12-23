#include "Render.h"
#include "Body.h"

Render::Render(const Configurer & config)
	:tracer(config)
{
	this->SCREEN_WIDTH = config.window_width;
	this->SCREEN_HEIGHT = config.window_height;
	this->CAM_POSITION = config.cam_position;
	this->CAM_FORWARD = config.cam_forward;
	this->CAM_UPPY = config.cam_uppy;
	this->CAM_UP = config.cam_up;
	this->CAM_RIGHT = config.cam_right;
	this->CAM_WIDTH = config.cam_width;
	this->CAM_HEIGHT = config.cam_height;
	this->CAM_DEPTH = config.cam_depth;
	
	this->EXPOSURE = config.exposure;
	this->AA_FACTOR = config.aa_factor;
}

Vec3 Render::pixel_color(int x, int y) const
{
	double dx = (double)x / SCREEN_WIDTH;
	double dy = (double)y / SCREEN_HEIGHT;

	Vec3 col = Vec3(0.0, 0.0, 0.0);
	for (int ax = 0; ax < AA_FACTOR; ++ax) {
		for (int ay = 0; ay < AA_FACTOR; ++ay) {
			Vec3 direction = CAM_FORWARD * CAM_DEPTH + 
				CAM_RIGHT * CAM_WIDTH  * ( (dx + (double)ax / (SCREEN_WIDTH * AA_FACTOR) ) - 0.5) + 
				CAM_UP    * CAM_HEIGHT * (0.5 - (dy + (double)ay / (SCREEN_WIDTH * AA_FACTOR)));
			Ray ray = Ray(CAM_POSITION, direction.normalized());
	
			Vec3 col_part = tracer.trace(ray);

			col_part = Vec3(1.0, 1.0, 1.0) - (col_part * EXPOSURE).exp();
			
			col = col + col_part;
		}
	}
	col = col / (AA_FACTOR * AA_FACTOR);
	
	return col;
}