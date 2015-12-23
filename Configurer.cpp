#include "Configurer.h"
#include "Textures.h"

Configurer::Configurer()
{
}

Configurer::~Configurer()
{
}

bool Configurer::load(std::string path)
{
	// Resolution
	window_width = 800;
	window_height = 450;

	// AppSystem
	window_title = "Hello world!";
	threads_count = 3;
	framerate = 60;

	// Render camera settings
	cam_forward = Vec3(0.0, 0.0, 1.0).normalized();
	cam_uppy = Vec3(0.0, 1.0, 0.0).normalized();
	cam_right = cam_uppy.cross(cam_forward);
	cam_up = cam_forward.cross(cam_right);
	cam_position = Vec3(0.0, 0.0, 0.0) - cam_forward * 175.0;
	cam_width = 16.0;
	cam_height = 9.0;
	cam_depth = 16.0;

	// Render settings
	exposure = -0.75;
	aa_factor = 2;

	// Tracer settings
	max_bounce = 10;
	diffuse_bounce_value = 7;
	refractive_bounce_value = 2;
	reflective_bounce_value = 2;
	diffuse_ray_count = 20;
	tracer_bias = 0.01;

	// Scene
	std::shared_ptr<Shape> left_wall = std::make_shared<ComposedShape>(std::vector<ComposedShape::MetaShape>{
		{ std::make_shared<Sphere>(Vec3(-25.0, 0.0, 0.0), 50.0), ComposedShape::MetaShape::MetaShapeType::POSITIVE },
		{ std::make_shared<Plane>(Vec3(-25.0, 0.0, 0.0), Vec3(-1.0, 0.0, 0.0)), ComposedShape::MetaShape::MetaShapeType::NEGATIVE },
	});
	std::shared_ptr<Shape> right_wall = std::make_shared<ComposedShape>(std::vector<ComposedShape::MetaShape>{
		{ std::make_shared<Sphere>(Vec3(25.0, 0.0, 0.0), 50.0), ComposedShape::MetaShape::MetaShapeType::POSITIVE },
		{ std::make_shared<Plane>(Vec3(25.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0)), ComposedShape::MetaShape::MetaShapeType::NEGATIVE },
	});
	std::shared_ptr<Shape> front_wall = std::make_shared<ComposedShape>(std::vector<ComposedShape::MetaShape>{
		{ std::make_shared<Sphere>(Vec3(0.0, 0.0, 25.0), 50.0), ComposedShape::MetaShape::MetaShapeType::POSITIVE },
		{ std::make_shared<Plane>(Vec3(0.0, 0.0, 25.0), Vec3(0.0, 0.0, 1.0)), ComposedShape::MetaShape::MetaShapeType::NEGATIVE },
	});
	std::shared_ptr<Shape> bottom_wall = std::make_shared<ComposedShape>(std::vector<ComposedShape::MetaShape>{
		{ std::make_shared<Sphere>(Vec3(0.0, -25.0, 0.0), 50.0), ComposedShape::MetaShape::MetaShapeType::POSITIVE },
		{ std::make_shared<Plane>(Vec3(0.0, -25.0, 0.0), Vec3(0.0, -1.0, 0.0)), ComposedShape::MetaShape::MetaShapeType::NEGATIVE },
	});
	std::shared_ptr<Shape> top_wall = std::make_shared<ComposedShape>(std::vector<ComposedShape::MetaShape>{
		{ std::make_shared<Sphere>(Vec3(0.0, 25.0, 0.0), 50.0), ComposedShape::MetaShape::MetaShapeType::POSITIVE },
		{ std::make_shared<Plane>(Vec3(0.0, 25.0, 0.0), Vec3(0.0, 1.0, 0.0)), ComposedShape::MetaShape::MetaShapeType::NEGATIVE },
	});

	bodies.push_back(Body(
		std::make_shared<Sphere>(Vec3(0.0, 1000.0, 0.0), 500.0),
		Material()
		.set_light_source_color(Vec3(1.0, 1.0, 1.0) * 30.0)
		));

	bodies.push_back(Body(
		left_wall,
		Material()
		.set_simple_diffuse_color(Vec3(1.0, 0.0, 0.0) * 1.0)
		.set_diffuse_color(Vec3(1.0, 0.0, 0.0) * 1.0)
		//.set_light_source_color(Vec3(1.0, 0.3, 0.3) * 5.0)
		));

	bodies.push_back(Body(
		right_wall,
		Material()
		.set_simple_diffuse_color([](Vec3 p, Vec3 n) {
		Vec3 dir = Vec3(1.0, 1.0, 1.0).normalized();
		Vec3 origin = Vec3(0.0, 0.0, 0.0);
		Vec3 pr = p - origin;
		double dist = (pr - dir * pr.dot(dir)).length() + 4.0 * turbulence(p, Vec3(-25.0, -50.0, -50.0), Vec3(100.0, 100.0, 100.0), Vec3(1.0, 1.0, 1.0) * 7.0, 5);
		double q = (sin(dist / 0.5) + 1.0) * 0.5;
		q = pow(q, 0.5);
		return Vec3(0.8, 0.5, 0.2) * q + Vec3(0.7, 0.3, 0.1) * pow(1.0 - q, 10.0);
	})
		.set_diffuse_color([](Vec3 p, Vec3 n) {
		Vec3 dir = Vec3(1.0, 1.0, 1.0).normalized();
		Vec3 origin = Vec3(0.0, 0.0, 0.0);
		Vec3 pr = p - origin;
		double dist = (pr - dir * pr.dot(dir)).length() + 5.0 * turbulence(p, Vec3(-25.0, -50.0, -50.0), Vec3(100.0, 100.0, 100.0), Vec3(1.0, 1.0, 1.0) * 7.0, 5);
		double q = (sin(dist / 0.5) + 1.0) * 0.5;
		q = pow(q, 0.5);
		return Vec3(0.8, 0.5, 0.2) * q + Vec3(0.7, 0.3, 0.1) * pow(1.0 - q, 10.0);
	})
		));

	bodies.push_back(Body(
		front_wall,
		Material()
		.set_simple_diffuse_color([](Vec3 p, Vec3 n) {
		return Vec3(0.0, 30.0, 0.0) *  vornoi(p, Vec3(-50.0, -50.0, -25.0), Vec3(100.0, 100.0, 100.0), Vec3(1.0, 1.0, 1.0) * 5.1);
	})
		.set_diffuse_color([](Vec3 p, Vec3 n) {
		return Vec3(0.0, 30.0, 0.0) *  vornoi(p, Vec3(-50.0, -50.0, -25.0), Vec3(100.0, 100.0, 100.0), Vec3(1.0, 1.0, 1.0) * 5.1);
	})
		//.set_light_source_color([](Vec3 p, Vec3 n) {	
		//	return Vec3(0.0, 2.0, 0.0) *  vornoi(p, Vec3(-50.0, -50.0, -25.0), Vec3(100.0, 100.0, 100.0), Vec3(5.0, 5.0, 5.0));
		//})
		));

	bodies.push_back(Body(
		bottom_wall,
		Material()
		.set_pure_reflective_color(TurbulentTexture(Vec3(-50.0, -75.0, -50.0), Vec3(100.0, 100.0, 100.0), Vec3(5.0, 5.0, 5.0), 5))
		//.set_simple_diffuse_color(Vec3(1.0, 1.0, 1.0) * 0.5)
		//.set_diffuse_color(Vec3(1.0, 1.0, 1.0) * 0.5)
		));

	//bodies.push_back(Body(
	//	top_wall,
	//	Material()
	//	.set_simple_diffuse_color(Vec3(1.0, 1.0, 1.0) * 0.5)
	//	.set_diffuse_color(Vec3(1.0, 1.0, 1.0) * 0.5)
	//));

	bodies.push_back(Body(
		std::make_shared<Sphere>(Vec3(-10.0, -10.0, 0.0), 10.0),
		Material()
		.set_simple_diffuse_color([](Vec3 p, Vec3 n) {
		return Vec3(0.0, 0.0, 1.0) + Vec3(0.0, 1.0, 0.0) * smooth_noise(p, Vec3(-20.0, -20.0, -10.0), Vec3(20.0, 20.0, 20.0), Vec3(1.0, 1.0, 1.0));
	})
		.set_diffuse_color(Vec3(1.0, 1.0, 1.0) * 1.0)
		.set_light_source_color([](Vec3 p, Vec3 n) {
		return Vec3(0.0, 0.0, 0.0) + Vec3(0.0, 0.0, 1.0) * smooth_noise(p, Vec3(-20.0, -20.0, -10.0), Vec3(20.0, 20.0, 20.0), Vec3(1.0, 1.0, 1.0));
	})
		.set_bump_mapping([](Vec3 p) {
		return 10.0 * smooth_noise(p, Vec3(-20.0, -20.0, -10.0), Vec3(20.0, 20.0, 20.0), Vec3(1.0, 1.0, 1.0));
	})
		));

	bodies.push_back(Body(
		std::make_shared<Sphere>(Vec3(10.0, -10.0, 0.0), 5.0),
		Material()
		.set_refractive_color(Vec3(1.0, 1.0, 1.0))
		.set_refraction_index(1.3)
		.set_refraction_density(Vec3(1.0, 0.0, 0.0) * (-0.5))
		));

	bodies.push_back(Body(
		std::make_shared<Sphere>(Vec3(10.0, -5.0, 0.0), 3.0),
		Material()
		.set_refractive_color(Vec3(1.0, 1.0, 1.0))
		.set_refraction_index(1.3)
		.set_refraction_density(Vec3(0.0, 0.0, 1.0) * (-0.5))
		));

	bodies.push_back(Body(
		std::make_shared<Sphere>(Vec3(15.0, 0.0, -30.0), 10.0),
		Material()
		.set_pure_reflective_color(Vec3(1.0, 1.0, 1.0) * 0.5)
		.set_refractive_color(Vec3(1.0, 1.0, 1.0) * 0.5)
		.set_refraction_index(1.5)
		.set_refraction_density(Vec3(0.0, 0.0, 0.0) * (-0.5))
		.set_bump_mapping([](Vec3 p) {
		return 0.25 * smooth_noise(p, Vec3(5.0, -10.0, -40.0), Vec3(20.0, 20.0, 20.0), Vec3(1.0, 1.0, 1.0));
	})
		));

	bodies.push_back(Body(
		std::make_shared<Sphere>(Vec3(15.0, 15.0, 15.0), 10.0),
		Material()
		.set_light_source_color( //NoiseTexture(Vec3(5.0, 5.0, 5.0), Vec3(20.0, 20.0, 20.0), Vec3(1.0, 1.0, 1.0)) * 
			(Vec3(1.0, 1.0, 0.8) * 20.0))
		));

	return true;
}
