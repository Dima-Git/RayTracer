#include "Textures.h"

double noise_function(Vec3 v) {
	return fmod(fmod(sin(v.dot(Vec3(12.9898, 78.233, 31.4159))) * 43758.5453, 1.0) + 1.0, 1.0);
}

double zoomed_noise(Vec3 p, Vec3 origin, Vec3 size, Vec3 patch_size) {
	Vec3 rel_patch_size = patch_size / size;
	Vec3 pos = (p - origin) / size;
	pos.x = pos.x - fmod(pos.x, rel_patch_size.x);
	pos.y = pos.y - fmod(pos.y, rel_patch_size.y);
	pos.z = pos.z - fmod(pos.z, rel_patch_size.z);
	return noise_function(pos);
}

double smooth_noise(Vec3 p, Vec3 origin, Vec3 size, Vec3 patch_size) {
	Vec3 rel_patch_size = patch_size / size;
	Vec3 pos = (p - origin) / size;
	double xf = fmod(pos.x, rel_patch_size.x);
	double yf = fmod(pos.y, rel_patch_size.y);
	double zf = fmod(pos.z, rel_patch_size.z);
	pos.x = pos.x - xf;
	pos.y = pos.y - yf;
	pos.z = pos.z - zf;

	xf = xf / (rel_patch_size.x);
	yf = yf / (rel_patch_size.y);
	zf = zf / (rel_patch_size.z);

	return
		(1.0 - xf) * (1.0 - yf) * (1.0 - zf) * noise_function(pos + Vec3(0.0, 0.0, 0.0) * rel_patch_size) +
		(1.0 - xf) * (1.0 - yf) * (zf)* noise_function(pos + Vec3(0.0, 0.0, 1.0) * rel_patch_size) +
		(1.0 - xf) * (yf)* (1.0 - zf) * noise_function(pos + Vec3(0.0, 1.0, 0.0) * rel_patch_size) +
		(1.0 - xf) * (yf)* (zf)* noise_function(pos + Vec3(0.0, 1.0, 1.0) * rel_patch_size) +
		(xf)* (1.0 - yf) * (1.0 - zf) * noise_function(pos + Vec3(1.0, 0.0, 0.0) * rel_patch_size) +
		(xf)* (1.0 - yf) * (zf)* noise_function(pos + Vec3(1.0, 0.0, 1.0) * rel_patch_size) +
		(xf)* (yf)* (1.0 - zf) * noise_function(pos + Vec3(1.0, 1.0, 0.0) * rel_patch_size) +
		(xf)* (yf)* (zf)* noise_function(pos + Vec3(1.0, 1.0, 1.0) * rel_patch_size);
}

double turbulence(Vec3 p, Vec3 origin, Vec3 size, Vec3 patch_size, int depth) {
	double k = 0.5;
	double n = 0.0;
	double t = 0.0;
	for (int i = 0; i < depth; ++i) {
		n = smooth_noise(p, origin, size, patch_size) * k;
		t += n;
		k = k * 0.5;
		patch_size = patch_size * 0.5;
	}
	t += n;
	return t;
}

Vec3 noise_vector(Vec3 v) {
	return Vec3(
		fmod(fmod(sin(v.dot(Vec3(12.9898, 78.233, 31.4159))) * 43758.5453, 1.0) + 1.0, 1.0),
		fmod(fmod(sin(v.dot(Vec3(13.9898, 68.233, 41.4159))) * 42758.5453, 1.0) + 1.0, 1.0),
		fmod(fmod(sin(v.dot(Vec3(14.9898, 58.233, 51.4159))) * 41758.5453, 1.0) + 1.0, 1.0));
}

double vornoi(Vec3 p, Vec3 origin, Vec3 size, Vec3 patch_size) {
	Vec3 rel_patch_size = patch_size / size;
	Vec3 pos = (p - origin) / size;
	Vec3 off = Vec3(
		pos.x - fmod(pos.x, rel_patch_size.x),
		pos.y - fmod(pos.y, rel_patch_size.y),
		pos.z - fmod(pos.z, rel_patch_size.z));
	Vec3 fp = off + noise_vector(off) * rel_patch_size;
	double min_dist = 3.0;
	for (double dx = -1.0; dx < 1.1; dx += 1.0)
		for (double dy = -1.0; dy < 1.1; dy += 1.0)
			for (double dz = -1.0; dz < 1.1; dz += 1.0) {
				Vec3 p = pos + Vec3(dx, dy, dz) * rel_patch_size;
				Vec3 off = Vec3(
					p.x - fmod(p.x, rel_patch_size.x),
					p.y - fmod(p.y, rel_patch_size.y),
					p.z - fmod(p.z, rel_patch_size.z));
				Vec3 fp = off + noise_vector(off) * rel_patch_size;
				double dist = (fp - pos).length();
				if (min_dist > dist) min_dist = dist;
			}
	return min_dist / sqrt(3.0);
}
