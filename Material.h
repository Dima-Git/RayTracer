#pragma once

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "Vec3.h"
#include <functional>

class Material
{

public:
	
	std::function<Vec3(Vec3, Vec3)> simple_diffuse_color;
	std::function<Vec3(Vec3, Vec3)> diffuse_color;
	std::function<Vec3(Vec3, Vec3)> pure_reflective_color;
	std::function<Vec3(Vec3, Vec3)> refractive_color;
	std::function<Vec3(Vec3, Vec3)> light_source_color;
	
	double refraction_index;
	Vec3 refraction_density;

	std::function<double(Vec3)> bump_mapping;

	Material() : refraction_index(1.0), refraction_density(Vec3(0.0, 0.0, 0.0)) {};

	Material & set_simple_diffuse_color(std::function<Vec3(Vec3, Vec3)> fn) { 
		simple_diffuse_color = fn; 
		return *this; 
	};
	Material & set_diffuse_color(std::function<Vec3(Vec3, Vec3)> fn) { 
		diffuse_color = fn; 
		return *this; 
	};
	Material & set_pure_reflective_color(std::function<Vec3(Vec3, Vec3)> fn) { 
		pure_reflective_color = fn; 
		return *this;
	};
	Material & set_light_source_color(std::function<Vec3(Vec3, Vec3)> fn) { 
		light_source_color = fn; 
		return *this;
	};
	Material & set_refractive_color(std::function<Vec3(Vec3, Vec3)> fn) {
		refractive_color = fn; 
		return *this;
	};
	Material & set_refraction_index(double index) { 
		refraction_index = index; 
		return *this;
	};
	Material & set_refraction_density(Vec3 density) { 
		refraction_density = density; 
		return *this; 
	};
	Material & set_bump_mapping(std::function<double(Vec3)> fn) {
		bump_mapping = fn;
		return *this;
	};

private:

};

#endif // _MATERIAL_H_