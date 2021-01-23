//============================================================================//
// k_vector.h                                                                //
//============================================================================//

//----------------------------------------------------------------------------//
// interface                                                                  //
//----------------------------------------------------------------------------//

#ifndef K_VECTOR_H
#define K_VECTOR_H

typedef struct Vec3f {
	float x;
	float y;
	float z;
} Vec3f;

Vec3f vec3f_add(Vec3f a, Vec3f b);
Vec3f vec3f_sub(Vec3f a, Vec3f b);

Vec3f vec3f_add_scalar(Vec3f a, float b);
Vec3f vec3f_sub_scalar(Vec3f a, float b);
Vec3f vec3f_mul_scalar(Vec3f a, float b);
Vec3f vec3f_div_scalar(Vec3f a, float b);

Vec3f vec3f_normalise(Vec3f a);
float vec3f_dot(Vec3f a, Vec3f b);
Vec3f vec3f_cross(Vec3f a, Vec3f b);
float vec3f_size(Vec3f a);

Vec3f vec3f_rotate_x(Vec3f a, float angle);
Vec3f vec3f_rotate_y(Vec3f a, float angle);
Vec3f vec3f_rotate_z(Vec3f a, float angle);

//----------------------------------------------------------------------------//
// implementation                                                             //
//----------------------------------------------------------------------------//

#ifdef K_VECTOR_IMPLEMENTATION

#include <math.h>

//---- private functions -----------------------------------------------------//

//---- public functions ------------------------------------------------------//

Vec3f vec3f_add(Vec3f a, Vec3f b) {
	return (Vec3f){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3f vec3f_sub(Vec3f a, Vec3f b) {
	return (Vec3f){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3f vec3f_add_scalar(Vec3f a, float b) {
	return (Vec3f){a.x + b, a.y + b, a.z + b};
}

Vec3f vec3f_sub_scalar(Vec3f a, float b) {
	return (Vec3f){a.x - b, a.y - b, a.z - b};
}

Vec3f vec3f_mul_scalar(Vec3f a, float b) {
	return (Vec3f){a.x * b, a.y * b, a.z * b};
}

Vec3f vec3f_div_scalar(Vec3f a, float b) {
	return (Vec3f){a.x / b, a.y / b, a.z / b};
}

Vec3f vec3f_normalise(Vec3f a) {
	float size = vec3f_size(a);
	return (Vec3f){a.x / size, a.y / size, a.z / size};
}

float vec3f_dot(Vec3f a, Vec3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3f vec3f_cross(Vec3f a, Vec3f b) {
	return (Vec3f){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

float vec3f_size(Vec3f a) {
	return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

Vec3f vec3f_rotate_x(Vec3f a, float angle) {
	Vec3f b;
	b.x = a.x;
	b.y = a.y * cos(angle) - a.z * sin(angle);
	b.z = a.y * sin(angle) + a.z * cos(angle);

	return b;
}

Vec3f vec3f_rotate_y(Vec3f a, float angle) {
	Vec3f b;
	b.z = a.x * cos(angle) + a.z * sin(angle);
	b.y = a.y;
	b.z = -a.x * sin(angle) + a.z * cos(angle);

	return b;
}

Vec3f vec3f_rotate_z(Vec3f a, float angle) {
	Vec3f b;
	b.x = a.x * cos(angle) - a.y * sin(angle);
	b.y = a.x * sin(angle) + a.y * cos(angle);
	b.z = a.z;

	return b;
}

#endif

#endif
