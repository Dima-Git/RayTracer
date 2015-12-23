#include "Tracer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <memory>
#include "Textures.h"

Tracer::Tracer(const Configurer & config)
{
	this->MAX_BOUNCE = config.max_bounce;
	this->DIFFUSE_RAY_COUNT = config.diffuse_ray_count;
	this->DIFFUSE_BOUNCE_VALUE = config.diffuse_bounce_value;
	this->BIAS = config.tracer_bias;
	this->REFRACTIVE_BOUNCE_VALUE = config.refractive_bounce_value;
	this->REFLECTIVE_BOUNCE_VALUE = config.reflective_bounce_value;

	environment = [](Vec3 direction) {
		return Vec3(0.0, 0.0, 0.0);
	};

	this->bodies = config.bodies;
	
}

Tracer::~Tracer()
{
}

double Tracer::get_nearest_hit(Ray ray, int * body_number, Vec3 * hit, Vec3 * normal) const 
{
	double nearest_dist = -1.0, temp_dist = 0;
	if (body_number) *body_number = -1;
	Vec3 temp_hit;
	Vec3 temp_normal;

	for (int i = (int)bodies.size() - 1; i >= 0; --i) {
		Shape::HitResult result = bodies[i].shape->first_hit(ray, &temp_hit, &temp_normal, &temp_dist);
		if (result == Shape::HitResult::HIT_HIT && (temp_dist < nearest_dist || nearest_dist <= 0.0)) {
			nearest_dist = temp_dist;
			if (body_number) *body_number = i;
			if (hit) *hit = temp_hit;
			if (normal) *normal = temp_normal;
			if (ray.direction.dot(temp_normal) > 0.0) break;
		}
	}

	return nearest_dist;
}

int Tracer::find_medium(Ray ray) const
{
	Vec3 normal;
	for (int i = (int)bodies.size() - 1; i >= 0; --i) {
		Shape::HitResult result = bodies[i].shape->first_hit(ray, nullptr, &normal, nullptr);
		if (result == Shape::HitResult::HIT_HIT && ray.direction.dot(normal) > 0.0) {
			return i;
		}
	}
	return -1;
}

static Vec3 reflection(Vec3 vec, Vec3 normal)
{
	return vec - normal * 2.0 * vec.dot(normal);
}

Vec3 perpendicular(const Vec3 & v) {
	if (abs(v.x) >= abs(v.y) && abs(v.x) >= abs(v.z))
		return Vec3(-(v.y + v.z) / v.x, 1.0, 1.0).normalized();
	if (abs(v.y) >= abs(v.x) && abs(v.y) >= abs(v.z))
		return Vec3(1.0, -(v.x + v.z) / v.y, 1.0).normalized();
	if (abs(v.z) >= abs(v.x) && abs(v.z) >= abs(v.y))
		return Vec3(1.0, 1.0, -(v.x + v.y) / v.z).normalized();
}

Vec3 Tracer::trace(Ray ray) const
{
	if (ray.bounce > MAX_BOUNCE) {
		return Vec3(0.0, 0.0, 0.0);
	}

	Vec3 color_sum = Vec3(0.0, 0.0, 0.0);
	Vec3 normal, hit;
	int nearest_body;
	double nearest_dist = get_nearest_hit(ray, &nearest_body, &hit, &normal);
	int current_medium = find_medium(ray);

	if (nearest_body != -1) {
		// BUMP MAPPING
		if (bodies[nearest_body].material.bump_mapping) {
			Vec3 p1 = perpendicular(normal);
			Vec3 p2 = normal.cross(p1);
			double h0 = bodies[nearest_body].material.bump_mapping(hit);
			double h1 = bodies[nearest_body].material.bump_mapping(hit + p1 * BIAS);
			double h2 = bodies[nearest_body].material.bump_mapping(hit + p2 * BIAS);
			double d1 = h0 - h1;
			double d2 = h0 - h2;
			Vec3 v1 = (p1 * BIAS + normal * d1);
			Vec3 v2 = (p2 * BIAS + normal * d2);

			Vec3 bump_normal = v1.cross(v2).normalized();
			if (ray.direction.dot(normal) * ray.direction.dot(bump_normal) > 0.0) {
				normal = bump_normal;
			}
		}
		// LIGHT SOURCE
		if (bodies[nearest_body].material.light_source_color) {
			color_sum = color_sum + bodies[nearest_body].material.light_source_color(hit, normal);
		}
		// SIMPLE DIFFUSE
		if (bodies[nearest_body].material.simple_diffuse_color) {
			for (size_t dst_body_number = 0; dst_body_number < bodies.size(); ++dst_body_number) {
				if (bodies[dst_body_number].material.light_source_color) {
					Vec3 shadow_vec = (bodies[dst_body_number].shape->center - hit).normalized();
					double r = bodies[dst_body_number].shape->radius;
					double d = (bodies[dst_body_number].shape->center - hit).length();
					double hemi_part = (r > d) ? 1.0 : 1.0 - sqrt(1.0 - r*r / (d*d));

					int dst_nearest_number = -1;
					Vec3 dst_hit, dst_normal;
					get_nearest_hit(Ray(hit + shadow_vec * BIAS, shadow_vec), &dst_nearest_number, &dst_hit, &dst_normal);
					if (dst_nearest_number == dst_body_number && shadow_vec.dot(normal) > 0.0) {
						double lambert = shadow_vec.dot(normal);
						// TODO: remove 2.0
						color_sum = color_sum + bodies[nearest_body].material.simple_diffuse_color(hit, normal) * bodies[dst_body_number].material.light_source_color(dst_hit, dst_normal) * hemi_part * lambert;
					}
				}
			}
		}
		// DIFFUSE
		if (bodies[nearest_body].material.diffuse_color && ray.bounce + DIFFUSE_BOUNCE_VALUE <= MAX_BOUNCE) {
			//TODO: enviroment diffuse
			Vec3 color_part = Vec3(0.0, 0.0, 0.0);
			for (size_t dst_body_number = 0; dst_body_number < bodies.size(); ++dst_body_number) {
				// Solid angle
				double r = bodies[dst_body_number].shape->radius;
				double d = (bodies[dst_body_number].shape->center - hit).length();
				double hemi_part = (r > d) ? 1.0 : 1.0 - sqrt(1.0 - r*r / (d*d));
				
				// Build base vector depending on where the object is (everywhere or on shadowvec direction)
				Vec3 base_vec = (hemi_part == 1.0) ? normal : (bodies[dst_body_number].shape->center - hit).normalized();
				// Vector basis
				Vec3 per1 = perpendicular(base_vec);
				Vec3 per2 = base_vec.cross(per1);
			
				Vec3 color_object_part = Vec3(0.0, 0.0, 0.0);
				for (int i = 0; i < DIFFUSE_RAY_COUNT; ++i) {
					// Random vector to an object
					double cos_phi = 1.0 - ((double)rand() / RAND_MAX) * hemi_part;
					double sin_phi = sqrt(1.0 - cos_phi*cos_phi);
					double alpha = 2.0 * M_PI * rand() / RAND_MAX;
					Vec3 shifted_vec = base_vec * cos_phi + per1 * sin_phi * sin(alpha) + per2 * sin_phi * cos(alpha);
					if (shifted_vec.dot(normal) > 0.0) {
						int dst_nearest_number = -1;
						get_nearest_hit(Ray(hit + shifted_vec * BIAS, shifted_vec), &dst_nearest_number, nullptr, nullptr);
						if (dst_nearest_number == dst_body_number) {
							color_object_part = color_object_part + trace(Ray(hit + shifted_vec * BIAS, shifted_vec, ray.bounce + DIFFUSE_BOUNCE_VALUE));
						}
					}
				}
				color_part = color_part + color_object_part * hemi_part / DIFFUSE_RAY_COUNT;
			}
			color_sum = color_sum + color_part * bodies[nearest_body].material.diffuse_color(hit, normal);
		}
		// PURE REFLECTIVE
		if (bodies[nearest_body].material.pure_reflective_color && ray.bounce + REFLECTIVE_BOUNCE_VALUE <= MAX_BOUNCE) {
			Vec3 reflected_vec = reflection(ray.direction, normal);
			color_sum = color_sum + trace(Ray(hit + reflected_vec * BIAS, reflected_vec, ray.bounce + REFLECTIVE_BOUNCE_VALUE)) * bodies[nearest_body].material.pure_reflective_color(hit, normal);
		}
		// REFRACTIVE
		if (bodies[nearest_body].material.refractive_color && ray.bounce + REFRACTIVE_BOUNCE_VALUE <= MAX_BOUNCE) {
			Vec3 refracted_vec = ray.direction;
			
			int next_medium = find_medium(Ray(hit + ray.direction * BIAS, ray.direction));
			
			double current_index = (current_medium == -1) ? 1.0 : bodies[current_medium].material.refraction_index;
			double next_index = (next_medium == -1) ? 1.0 : bodies[next_medium].material.refraction_index;
			double c = - ray.direction.dot(normal);
			double r = current_index / next_index;
			double s2 = r * r * (1.0 - c * c);
			
			if (s2 > 1.0) // reflection (critical angle)
				refracted_vec = ray.direction + normal * 2.0 * c;
			else // refraction
				refracted_vec = (ray.direction * r + normal * (r*c - sqrt(1.0 - s2)*(c > 0.0 ? 1.0 : -1.0)));
			// TODO: fresnel
			color_sum = color_sum + trace(Ray(hit + refracted_vec * BIAS, refracted_vec, ray.bounce + REFRACTIVE_BOUNCE_VALUE)) * bodies[nearest_body].material.refractive_color(hit, normal);
		}
	}
	else { // No object intersection
		// Background color
		color_sum = environment(ray.direction);
	}

	// Medium absorption
	Vec3 density = Vec3(-0.001, -0.001, -0.001) * 0.0;
	if (current_medium != -1) {
		density = bodies[current_medium].material.refraction_density;
	}
	
	color_sum = color_sum * (density * nearest_dist).exp();

	return color_sum;
}
