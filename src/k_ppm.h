//============================================================================//
// k_ppm.h                                                                //
//============================================================================//

//----------------------------------------------------------------------------//
// interface                                                                  //
//----------------------------------------------------------------------------//

#ifndef K_PPM_H
#define K_PPM_H

typedef struct Color {
	float r;
	float g;
	float b;
} Color;

typedef struct Image {
	unsigned int width;
	unsigned int height;
	Color **data;
} Image;

Image *create_image(unsigned int width, unsigned int height, Color bgColor);
void destroy_image(Image *image);
void write_image(Image *image, char *fileName);

//----------------------------------------------------------------------------//
// implementation                                                             //
//----------------------------------------------------------------------------//

#ifdef K_PPM_IMPLEMENTATION

#include <stdio.h>

//---- private functions -----------------------------------------------------//

//---- public functions ------------------------------------------------------//

Image *create_image(unsigned int width, unsigned int height, Color bgColor) {
	Image *image = malloc(sizeof(Image));

	image->width = width;
	image->height = height;

	image->data = malloc(sizeof(Color *) * width);
	for(int i = 0; i < (int)width; i++) {
		image->data[i] = malloc(sizeof(Color) * height);
	}

	for(int i = 0; i < (int)width; i++) {
		for(int j = 0; j < (int)height; j++) {
			image->data[i][j] = bgColor;
		}
	}

	return image;
}

void destroy_image(Image *image) {
	free(image);
}

void write_image(Image *image, char *fileName) {
	FILE *fp = fopen(fileName, "wb");

	fprintf(fp, "P3 %d %d 255\n", image->width, image->height);

	for(int i = 0; i < (int)image->height; i++) {
		for(int j = 0; j < (int)image->width; j++) {
			Color *pixel = &image->data[j][i];
			fprintf(fp, "%d %d %d\n", (int)(pixel->r * 255), (int)(pixel->g * 255), (int)(pixel->b * 255));
		}
	}

	fclose(fp);

}

#endif

#endif
