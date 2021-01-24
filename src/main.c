#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define K_IMAGE_IMPLEMENTATION
#include "k_image.h"

#define K_VECTOR_IMPLEMENTATION
#include "k_vector.h"

#define WIDTH 1000
#define HEIGHT 1000

#define PI 3.14159265358979323846
#define FOV PI / 3

#define MAX_DEPTH 5

#define EPSILON 0.001
typedef struct Material {
	Color diffuse;
	float ambient;
	float specular;
	float specularExp;
	float reflective;
	float refractive;
	float rough;

} Material;

typedef struct Sphere {
	Vec3f center;
	float radius;
	Material material;
} Sphere;

typedef struct Triangle {
	Vec3f a;
	Vec3f b;
	Vec3f c;
	Material material;
} Triangle;

typedef struct Ray {
	Vec3f origin;
	Vec3f direction;
} Ray;

typedef struct Light {
	Vec3f pos;
	float strength;
} Light;

typedef struct Scene {
	Color bgColor;

	Light *lights;
	int lightCount;

	Sphere *spheres;
	int sphereCount;

	Triangle *triangles;
	int triangleCount;

} Scene;

Scene *init_scene(Color bgColor) {
	Scene *scene = malloc(sizeof(Scene));

	scene->bgColor = bgColor;

	scene->lightCount = 0;
	scene->sphereCount = 0;
	scene->triangleCount = 0;

	return scene;
}

void destroy_scene(Scene *scene) {
	free(scene);
}

void add_light(Scene *scene, Light light) {
	scene->lightCount++;
	scene->lights = realloc(scene->lights, sizeof(Light) * scene->lightCount);

	scene->lights[scene->lightCount - 1] = light;
}

void add_sphere(Scene *scene, Sphere sphere) {
	scene->sphereCount++;
	scene->spheres = realloc(
		scene->spheres,
		sizeof(Sphere) * scene->sphereCount
	);

	scene->spheres[scene->sphereCount - 1] = sphere;
}

void add_triangle(Scene *scene, Triangle triangle) {
	scene->triangleCount++;
	scene->triangles = realloc(
		scene->triangles,
		sizeof(Triangle) * scene->triangleCount
	);

	scene->triangles[scene->triangleCount - 1] = triangle;
}

void add_square(
	Scene *scene, Vec3f a, Vec3f b, Vec3f c, Vec3f d, Material material
) {
	add_triangle(scene, (Triangle){a, b, c, material});
	add_triangle(scene, (Triangle){a, c, d, material});
}

int hit_sphere(Ray ray, Sphere sphere, float *distance) {
	Vec3f oc = vec3f_sub(ray.origin, sphere.center);
	float a = vec3f_dot(ray.direction, ray.direction);
	float b = 2.0 * vec3f_dot(oc, ray.direction);
	float c = vec3f_dot(oc,oc) - sphere.radius * sphere.radius;
	float discriminant = b * b - 4 * a * c;

	if(discriminant < 0)
		return 0;
	
	float sqrtDiscriminant = sqrt(discriminant);

	float t1 = (-b - sqrtDiscriminant) / (2 * a);
	float t2 = (-b + sqrtDiscriminant) / (2 * a);

	if(t1 < 0 && t2 < 0)
		return 0;

	if(t1 > t2) {
		float tmp = t1;
		t1 = t2;
		t2 = tmp;
	}
	
	if(t1 < 0)
		*distance = t2;
	else
		*distance = t1;

	return 1;
}

int hit_triangle(Ray ray, Triangle triangle, float *distance)  {
	Vec3f ab = vec3f_sub(triangle.b, triangle.a);
	Vec3f ac = vec3f_sub(triangle.c, triangle.a);

	Vec3f p = vec3f_cross(ac, ray.direction);
	float det = vec3f_dot(ab, p);
	
	if(fabs(det) < EPSILON)
		return 0;

	Vec3f t = vec3f_sub(ray.origin, triangle.a);
	float u = vec3f_dot(t, p) / det;

	if(u < 0 || u > 1)
		return 0;
	
	Vec3f q = vec3f_cross(ab, t);
	float v = vec3f_dot(ray.direction, q) / det;

	if(v < 0 || u + v > 1)
		return 0;
	
	*distance = vec3f_dot(ac, q) / det;

	if(*distance < 0)
		return 0;

	return 1;
}

Vec3f reflection_dir(Vec3f ray, Vec3f normal) {
	return vec3f_mul_scalar(
		vec3f_normalise(
			vec3f_sub(
				vec3f_mul_scalar(normal, 2 * vec3f_dot(ray, normal)),
				ray
			)
		),
		-1
	);
}

Vec3f randomise_dir(Vec3f dir, float max) {
	float rx = (float)rand() / (float)RAND_MAX * max * PI / 2;
	float ry = (float)rand() / (float)RAND_MAX * max * PI / 2;
	float rz = (float)rand() / (float)RAND_MAX * max * PI / 2;

	dir = vec3f_rotate_x(dir, rx);
	dir = vec3f_rotate_y(dir, ry);
	dir = vec3f_rotate_z(dir, rz);

	return dir;
}

Color cast_ray(Scene *scene, Ray ray, int depth) {
	Color color = scene->bgColor;

	if(depth < MAX_DEPTH) {
		int flag = -1;
		float distance = -1;

		Sphere *sphere;
		Triangle *triangle;

		for(int i = 0; i < scene->sphereCount; i++) {
			float currentDistance;
			if(hit_sphere(ray, scene->spheres[i], &currentDistance)) {
				if(flag == -1 || currentDistance < distance) {
					distance = currentDistance;
					sphere = &scene->spheres[i];
					flag = 1;
				}
			}
		}

		for(int i = 0; i < scene->triangleCount; i++) {
			float currentDistance;
			if(hit_triangle(ray, scene->triangles[i], &currentDistance)) {
				if(flag == -1 || currentDistance < distance) {
					distance = currentDistance;
					triangle = &scene->triangles[i];
					flag = 2;
				}
			}
		}

		if(flag != -1) {
			Material material;
			Vec3f hitPoint; 
			Vec3f normal;
			
			if(flag == 1) {
				hitPoint = vec3f_add(
					ray.origin, vec3f_mul_scalar(ray.direction, distance)
				);

				normal = vec3f_normalise(
					vec3f_sub(hitPoint, sphere->center)
				);

				material = sphere->material;

			} else if(flag == 2) {
				hitPoint = vec3f_add(
					ray.origin, vec3f_mul_scalar(ray.direction, distance)
				);

				normal = vec3f_normalise(
					vec3f_cross(
						vec3f_sub(triangle->b, triangle->a),
						vec3f_sub(triangle->c, triangle->a)
					)
				);

				if(vec3f_dot(normal, ray.direction) > 0)
					normal = vec3f_mul_scalar(normal, -1);
				
				material = triangle->material;
			}

			Ray reflection = (Ray){
				vec3f_add(hitPoint, vec3f_mul_scalar(normal, EPSILON)),
				randomise_dir(
					reflection_dir(ray.direction, normal), material.rough
				)
			};

			Color reflectionColor = cast_ray(scene, reflection, depth + 1);

			float diffuseLightStrength = 0;
			float specularLightStrength = 0;

			for(int j = 0; j < scene->lightCount; j++) {
				Light light = scene->lights[j];
				Ray lightRay = (Ray){
					vec3f_add(hitPoint, vec3f_mul_scalar(normal, EPSILON)),
					vec3f_normalise(vec3f_sub(light.pos, hitPoint))
				};

				int intersect = 0;
				
				for(int k = 0; k < scene->sphereCount; k++) {
					float d;
					if(hit_sphere(lightRay, scene->spheres[k], &d)) {
						if(d > 0) {
							intersect = 1;
							break;
						}
					}
				}
				
				if(!intersect) {
					for(int k = 0; k < scene->triangleCount; k++) {
						float d;
						if(hit_triangle(lightRay, scene->triangles[k], &d)) {
							if(d > 0) {
								intersect = 1;
								break;
							}
						}
					}
				}

				if(!intersect) {
					// diffuse lighting
					Vec3f lightDirection = vec3f_normalise(
						vec3f_sub(light.pos, hitPoint)
					);
					float diffuseDot = vec3f_dot(normal, lightDirection);

					if(diffuseDot > 0) {
						diffuseLightStrength += light.strength * diffuseDot;

						// specular lighting
						Vec3f lightReflectionDir = reflection_dir(
							lightDirection, normal
						);
						float specularDot = vec3f_dot(
							lightReflectionDir, ray.direction
						);

						specularLightStrength
							+= light.strength
							* powf(specularDot, material.specularExp);
					}
				}
			}

			color = color_add(
				color_add_scalar(
					color_mul_scalar(
						color_mul_scalar(
							material.diffuse,
							1 - material.ambient
						),
						diffuseLightStrength
					),
					material.specular * specularLightStrength
				),
				color_mul_scalar(reflectionColor, material.reflective)
			);
		}
	}

	return color;
}

void render(Scene *scene, Image *image) {
	float aspectRatio = (float)image->width / (float)image->height;
	float tanFOV = tan(FOV / 2);

	for(unsigned int i = 0; i < image->height; i++) {
		for(unsigned int j = 0; j < image->width; j++) {
			float x = (2 * (j + 0.5) / image->width - 1) * tanFOV * aspectRatio;
			float y = - (2 * (i + 0.5) / image->height - 1) * tanFOV;
			Vec3f dir = vec3f_normalise((Vec3f){x, y, -1});
			// Ray ray = (Ray){(Vec3f){0, 0, 0}, dir};

			image->data[j + i * image->width] = cast_ray(
				scene, (Ray){(Vec3f){0, 0, 0}, dir}, 0
			);
		}

		printf("\r%d/%d lines (%f%%)", i, image->height, (float)i / (float)image->height * 100);
		fflush(stdout);
	}
	printf("\r%d/%d lines (%f%%)\n", image->height, image->height, (float)100);
}

int main(void) {
	Image *image = create_image(WIDTH, HEIGHT, (Color){0, 0, 0});

	Scene *scene = init_scene((Color){0.5, 0.5, 1});

	Material mirror = (Material){(Color){0, 0, 0}, 0.1, 1, 40, 0.7, 0, 0};
	// Material fuzzyMirror = (Material){(Color){0, 0, 0}, 0.1, 1, 40, 0.8, 0, 0.03};
	Material shinyRed = (Material){(Color){1, 0, 0}, 0.1, 1, 20, 0.2, 0, 0};
	Material shinyBlue = (Material){(Color){0, 0, 1}, 0.1, 1, 20, 0.2, 0, 0};
	Material dullRed = (Material){(Color){1, 0, 0}, 0.1, 1, 20, 0.01, 0, 0};
	Material green = (Material){(Color){0, 1, 0}, 0.1, 1, 40, 0.01, 0, 0};

	add_sphere(scene, (Sphere){(Vec3f){1, 0, -9}, 1, mirror});
	add_sphere(scene, (Sphere){(Vec3f){-1, 0, -10}, 0.5, dullRed});
	add_sphere(scene, (Sphere){(Vec3f){-1, -1, -6}, 0.5, green});

	add_square(scene, (Vec3f){4, -2, -4}, (Vec3f){4, -2, -12}, (Vec3f){-4, -2, -12}, (Vec3f){-4, -2, -4}, shinyRed);
	add_square(scene, (Vec3f){4, -2, -12}, (Vec3f){4, 4, -12}, (Vec3f){-4, 4, -12}, (Vec3f){-4, -2, -12}, shinyBlue);

	add_triangle(scene, (Triangle){(Vec3f){4, 2, -8.5}, (Vec3f){2, 2, -11.5}, (Vec3f){2, 4, -8.5}, mirror});
	
	add_light(scene, (Light){(Vec3f){-2, 5, -6}, 1});
	add_light(scene, (Light){(Vec3f){2, 5, -6}, 0.5});

	render(scene, image);

	destroy_scene(scene);

	write_image(image, "test.ppm");

	destroy_image(image);

	return(0);
}
