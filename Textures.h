#pragma once

#ifndef _TEXTURES_H_
#define _TEXTURES_H_

#include "Vec3.h"
#include <math.h>

double noise_function(Vec3 v);

double zoomed_noise(Vec3 p, Vec3 origin, Vec3 size, Vec3 patch_size);

double smooth_noise(Vec3 p, Vec3 origin, Vec3 size, Vec3 patch_size);

double turbulence(Vec3 p, Vec3 origin, Vec3 size, Vec3 patch_size, int depth);

Vec3 noise_vector(Vec3 v);

double vornoi(Vec3 p, Vec3 origin, Vec3 size, Vec3 patch_size);


class NoiseTexture {

public:

	NoiseTexture(Vec3 origin, Vec3 size, Vec3 patch_size) {
		this->origin = origin;
		this->size = size;
		this->patch_size = patch_size;
		this->color = Vec3(1.0, 1.0, 1.0);
	}

	Vec3 operator() (Vec3 p, Vec3 n) {
		return color * zoomed_noise(p, origin, size, patch_size);
	}

	NoiseTexture operator * (Vec3 col) {
		NoiseTexture clone(*this);
		clone.color = clone.color * col;
		return clone;
	}

private:

	Vec3 color;
	Vec3 origin;
	Vec3 size;
	Vec3 patch_size;

};

class SmoothNoiseTexture {

public:

	SmoothNoiseTexture(Vec3 origin, Vec3 size, Vec3 patch_size) {
		this->origin = origin;
		this->size = size;
		this->patch_size = patch_size;
		this->color = Vec3(1.0, 1.0, 1.0);
	}

	Vec3 operator() (Vec3 p, Vec3 n) {
		return color * smooth_noise(p, origin, size, patch_size);
	}

	SmoothNoiseTexture operator * (Vec3 col) {
		SmoothNoiseTexture clone(*this);
		clone.color = clone.color * col;
		return clone;
	}

private:

	Vec3 color;
	Vec3 origin;
	Vec3 size;
	Vec3 patch_size;

};

class TurbulentTexture {

public:

	TurbulentTexture(Vec3 origin, Vec3 size, Vec3 patch_size, int depth) {
		this->origin = origin;
		this->size = size;
		this->patch_size = patch_size;
		this->color = Vec3(1.0, 1.0, 1.0);
		this->depth = depth;
	}

	Vec3 operator() (Vec3 p, Vec3 n) {
		return color * turbulence(p, origin, size, patch_size, depth);
	}

	TurbulentTexture operator * (Vec3 col) {
		TurbulentTexture clone(*this);
		clone.color = clone.color * col;
		return clone;
	}

private:

	Vec3 color;
	Vec3 origin;
	Vec3 size;
	Vec3 patch_size;
	int depth;

};

#endif // _TEXTURES_H_