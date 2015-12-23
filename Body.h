#pragma once

#ifndef _BODY_H_
#define _BODY_H_

#include "Vec3.h"
#include "Ray.h"
#include "Material.h"
#include <math.h>
#include <vector>
#include <memory>

class Shape {

public:
	
	Vec3 center;
	double radius;

	Shape() {}
	virtual ~Shape() {}

	enum class HitResult {
		HIT_HIT,
		HIT_MISS,
		HIT_OUT
	};

	virtual HitResult first_hit(Ray ray, Vec3 * hit = nullptr, Vec3 * normal = nullptr, double * distance = nullptr) const = 0;

private:

};

class Body {

public:
	
	std::shared_ptr<Shape> shape;
	Material material;

	Body(std::shared_ptr<Shape> shape, const Material & material) : shape(shape), material(material) { }

private:

};

class Sphere : public Shape {

public:

	Sphere(Vec3 center, double radius) : radius2(radius*radius) {
		this->center = center;
		this->radius = radius;
	}

	HitResult first_hit(Ray ray, Vec3 * hit = nullptr, Vec3 * normal = nullptr, double * distance = nullptr) const {
		Vec3 RC = center - ray.origin;
		double d = RC.dot(ray.direction);
		
		Vec3 RP = ray.direction * d;
		Vec3 CP = RP - RC;

		if (CP.dot(CP) > radius2)
			return HitResult::HIT_MISS;

		double q2 = radius2 - CP.dot(CP);
		double q = sqrt(q2);

		Vec3 HP = ray.direction * q;

		Vec3 RH = RP - HP;

		if (RH.dot(ray.direction) < 0.0)
			RH = RP + HP;

		if (RH.dot(ray.direction) < 0.0)
			return HitResult::HIT_MISS;
		
		if (hit) *hit = ray.origin + RH;
		if (normal) *normal = (RH - RC).normalized();
		if (distance) *distance = RH.length();
		return HitResult::HIT_HIT;
	}

private:

	double radius2;
};

class Plane : public Shape {

public:

	Plane(Vec3 center, Vec3 normal) {
		this->center = center;
		this->radius = 1.0; // shouldn't be used
		this->normal = normal.normalized();
	}

	HitResult first_hit(Ray ray, Vec3 * hit = nullptr, Vec3 * normal = nullptr, double * distance = nullptr) const {
		Vec3 RC = center - ray.origin;
		double rcn = this->normal.dot(RC);
		double rdn = this->normal.dot(ray.direction);

		if (rcn > 0.0) { // inside
			if (rdn <= 0.0) {
				if (normal) *normal = ray.direction;
				return HitResult::HIT_OUT;
			}
		}
		else { // outside
			if (rdn >= 0.0) {
				return HitResult::HIT_MISS;
			}
		}

		Vec3 RH = ray.direction * (rcn / rdn);

		if (hit) *hit = ray.origin + RH;
		if (normal) *normal = this->normal;
		if (distance) *distance = RH.length();

		return HitResult::HIT_HIT;
	}

private:

	Vec3 normal;

};

// TODO: create torus, cone, etc

class ComposedShape : public Shape {

public:

	struct MetaShape {
		std::shared_ptr<Shape> shape;
		enum class MetaShapeType {POSITIVE, NEGATIVE} type;
	};

	ComposedShape(const std::vector<MetaShape> & metashapes) {
		this->metashapes = metashapes;
		if (metashapes.empty()) throw "Empty metabodies";
		Vec3 lbn = metashapes[0].shape->center, rtf = lbn;
		for (size_t i = 0; i < metashapes.size(); ++i) {
			if (metashapes[i].type == MetaShape::MetaShapeType::POSITIVE) {
				if (lbn.x > metashapes[i].shape->center.x - metashapes[i].shape->radius) lbn.x = metashapes[i].shape->center.x - metashapes[i].shape->radius;
				if (lbn.y > metashapes[i].shape->center.y - metashapes[i].shape->radius) lbn.y = metashapes[i].shape->center.y - metashapes[i].shape->radius;
				if (lbn.z > metashapes[i].shape->center.z - metashapes[i].shape->radius) lbn.z = metashapes[i].shape->center.z - metashapes[i].shape->radius;
				if (rtf.x < metashapes[i].shape->center.x + metashapes[i].shape->radius) rtf.x = metashapes[i].shape->center.x + metashapes[i].shape->radius;
				if (rtf.y < metashapes[i].shape->center.y + metashapes[i].shape->radius) rtf.y = metashapes[i].shape->center.y + metashapes[i].shape->radius;
				if (rtf.z < metashapes[i].shape->center.z + metashapes[i].shape->radius) rtf.z = metashapes[i].shape->center.z + metashapes[i].shape->radius;
			}
		}
		Vec3 octet = (rtf - lbn) * 0.5;
		this->center = lbn + octet;
		this->radius = octet.length();
	}

	HitResult first_hit(Ray ray, Vec3 * hit = nullptr, Vec3 * normal = nullptr, double * distance = nullptr) const {

		const double bias = 0.0001;
		Vec3 chit, cnormal;
		Vec3 mhit, mnormal;
		double cdist = 0.0;
		int initial_tl = -1;
		int last_tl = -1;
		bool first_time = true;
		Vec3 init_origin = ray.origin;

		for (;;) {
			// search for shape we are in, -1 if in no one
			for (initial_tl = (int)metashapes.size() - 1; initial_tl >= 0; --initial_tl) {
				HitResult result = metashapes[initial_tl].shape->first_hit(ray, nullptr, &cnormal, nullptr);
				if ( (result == HitResult::HIT_HIT || result == HitResult::HIT_OUT) && ray.direction.dot(cnormal) > 0.0)
						break;
			}

			if (!first_time) {
				bool found_edge = 
					(initial_tl != -1 && last_tl == -1 && metashapes[initial_tl].type == MetaShape::MetaShapeType::POSITIVE) ||
					(initial_tl == -1 && last_tl != -1 && metashapes[last_tl].type == MetaShape::MetaShapeType::POSITIVE) ||
					(initial_tl != -1 && last_tl != -1 && metashapes[last_tl].type != metashapes[initial_tl].type);

				if (found_edge) {
					if (hit) *hit = mhit;
					int tl = last_tl;
					if (initial_tl > tl) tl = initial_tl;
					if (normal) *normal = mnormal * (tl != -1 && metashapes[tl].type == MetaShape::MetaShapeType::NEGATIVE ? -1.0 : 1.0);
					if (distance) *distance = (init_origin - mhit).length();
					return HitResult::HIT_HIT;
				}
			}

			double min_dist = -1.0;
			for (int i = (int)metashapes.size() - 1; i >= initial_tl && i >= 0; --i) {
				HitResult result = metashapes[i].shape->first_hit(ray, &chit, &cnormal, &cdist);
				if (result == HitResult::HIT_HIT && (cdist < min_dist || min_dist < 0.0)) {
					mhit = chit;
					mnormal = cnormal;
					min_dist = cdist;
				}
			}

			// TODO: check cycling if < instead of <=
			if (min_dist <= 0.0) 
				return HitResult::HIT_MISS;

			first_time = false;
			last_tl = initial_tl;
			ray.origin = mhit + ray.direction * bias;

		}

	}

private:

	std::vector<MetaShape> metashapes;
};


#endif // _BODY_H_