#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define K_PPM_IMPLEMENTATION
#include "k_ppm.h"

#define K_VECTOR_IMPLEMENTATION
#include "k_vector.h"

#define WIDTH 1000
#define HEIGHT 1000

#define PI 3.14159265358979323846
#define FOV PI / 3

#define MAX_DEPTH 5

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

} Scene;

Scene *init_scene(Color bgColor) {
	Scene *scene = malloc(sizeof(Scene));

	scene->bgColor = bgColor;

	scene->lightCount = 0;
	scene->sphereCount = 0;

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
	scene->spheres = realloc(scene->spheres, sizeof(Sphere) * scene->sphereCount);

	scene->spheres[scene->sphereCount - 1] = sphere;
}

int ray_sphere_intersect(Ray ray, Sphere sphere, float *distance) {
	Vec3f oc = vec3f_sub(ray.origin, sphere.center);
	float a = vec3f_dot(ray.direction, ray.direction);
	float b = 2.0 * vec3f_dot(oc, ray.direction);
	float c = vec3f_dot(oc,oc) - sphere.radius * sphere.radius;
	float discriminant = b * b - 4 * a * c;
	if(discriminant < 0) {
		return 0;
	}

	float t1 = (-b - sqrt(discriminant)) / (2.0*a);
	float t2 = (-b + sqrt(discriminant)) / (2.0*a);

	
	if(t1 > t2) {
		float tmp = t1;
		t1 = t2;
		t2 = tmp;
	}

	if(t1 < 0 && t2 < 0)
		return 0;
	
	if(t1 < 0)
		*distance = t2;
	else
		*distance = t1;

	return 1;
}

Vec3f reflection_dir(Vec3f ray, Vec3f normal) {
	return vec3f_mul_scalar(vec3f_normalise(vec3f_sub(vec3f_mul_scalar(normal, 2 * vec3f_dot(ray, normal)), ray)), -1);
}

Color cast_ray(Scene *scene, Ray ray, int depth) {
	Color color = scene->bgColor;

	if(depth < MAX_DEPTH) {
		float minDistance;
		int minIndex = -1;

		for(int i = 0; i < scene->sphereCount; i++) {
			float distance;
			if(ray_sphere_intersect(ray, scene->spheres[i], &distance)) {
				if(minIndex == -1 || distance < minDistance) {
					minDistance = distance;
					minIndex = i;
				}
			}
		}

		if(minIndex != -1) {
			Sphere sphere = scene->spheres[minIndex];
			Material material = sphere.material;

			Vec3f intersectPoint = vec3f_add(ray.origin, vec3f_mul_scalar(ray.direction, minDistance));
			Vec3f normal = vec3f_normalise(vec3f_sub(intersectPoint, sphere.center));

			Vec3f reflectionDir = reflection_dir(ray.direction, normal);
			Ray reflection = (Ray){vec3f_add(intersectPoint, vec3f_mul_scalar(reflectionDir, 0.001)), reflectionDir};

			Color reflectionColor = cast_ray(scene, reflection, depth + 1);

			float diffuseLightStrength = 0;
			float specularLightStrength = 0;

			for(int j = 0; j < scene->lightCount; j++) {
				Light light = scene->lights[j];

				Ray intersectionToLight = (Ray){intersectPoint, vec3f_normalise(vec3f_sub(light.pos, intersectPoint))};

				int intersect = 0;
				for(int k = 0; k < scene->sphereCount; k++) {
					float d;
					if(ray_sphere_intersect(intersectionToLight, scene->spheres[k], &d)) {
						if(k != minIndex && d > 0) {
							intersect = 1;
							break;
						}
					}
				}

				if(!intersect) {
					Vec3f lightDirection = vec3f_normalise(vec3f_sub(light.pos, intersectPoint));
					float diffuseDot = vec3f_dot(normal, lightDirection);

					if(diffuseDot > 0)
						diffuseLightStrength += light.strength * diffuseDot;
					
					Vec3f lightReflectionDir = reflection_dir(lightDirection, normal);

					float specularDot = vec3f_dot(lightReflectionDir, ray.direction);

					if(specularDot > 0)
						specularLightStrength += light.strength * powf(specularDot, material.specularExp);
				}
				
			}

			if(diffuseLightStrength > 1)
				diffuseLightStrength = 1;
			
			if(specularLightStrength > 1)
				specularLightStrength = 1;

			color.r = material.diffuse.r * (1 - material.ambient) * diffuseLightStrength + material.specular * specularLightStrength + material.reflective * reflectionColor.r;
			color.g = material.diffuse.g * (1 - material.ambient) * diffuseLightStrength + material.specular * specularLightStrength + material.reflective * reflectionColor.g;
			color.b = material.diffuse.b * (1 - material.ambient) * diffuseLightStrength + material.specular * specularLightStrength + material.reflective * reflectionColor.b;

			if(color.r > 1)
				color.r = 1;

			if(color.g > 1)
				color.g = 1;

			if(color.b > 1)
				color.b = 1;

		}

	}

	return color;
}

void render(Scene *scene, Image *image) {
	float aspectRatio = (float)image->width / (float)image->height;
	float tanFOV = tan(FOV / 2);

	for(int i = 0; i < (int)image->height; i++) {
		for(int j = 0; j < (int)image->width; j++) {
			float x = (2 * (j + 0.5) / image->width - 1) * tanFOV * aspectRatio;
			float y = - (2 * (i + 0.5) / image->height - 1) * tanFOV;
			Vec3f dir = vec3f_normalise((Vec3f){x, y, -1});
			Ray ray = (Ray){(Vec3f){0, 0, 0}, dir};

			image->data[j][i] = cast_ray(scene, ray, 0);
		}
	}
}

int main(void) {
	Image *image = create_image(WIDTH, HEIGHT, (Color){0, 0, 0});

	Scene *scene = init_scene((Color){0.1, 0.1, 0.1});

	Material mirror = (Material){(Color){0, 0, 0}, 0.1, 1, 40, 0.8, 0, 0};
	Material red = (Material){(Color){1, 0, 0}, 0.1, 1, 40, 0.1, 0, 0};
	Material green = (Material){(Color){0, 1, 0}, 0.1, 1, 40, 0.01, 0, 0};
	
	add_sphere(scene, (Sphere){(Vec3f){4, 3, -10}, 4, mirror});
	add_sphere(scene, (Sphere){(Vec3f){1, 0, -5}, 1, green});
	add_sphere(scene, (Sphere){(Vec3f){-0.8, 0, -4}, 1, red});
	add_sphere(scene, (Sphere){(Vec3f){0.5, -1, -2}, 0.5, red});

	add_light(scene, (Light){(Vec3f){-10, 10, 0}, 1});
	add_light(scene, (Light){(Vec3f){-10, 10, -15}, 1});

	render(scene, image);

	write_image(image, "test.ppm");

	destroy_image(image);

	return(0);
}