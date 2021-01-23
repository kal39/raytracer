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
	Color *data;
} Image;

Image *create_image(unsigned int width, unsigned int height, Color bgColor);
void destroy_image(Image *image);
void write_image(Image *image, char *fileName);
Image *antialias_image(Image *in, int sampleSize);

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

	image->data = malloc(sizeof(Color) * width * height);

	for(int i = 0; i < (int)width * (int)height; i++) {
		image->data[i] = bgColor;
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
			Color *pixel = &image->data[j + i * image->width];

			if(pixel->r < 0)
				pixel->r = 0;
			if(pixel->g < 0)
				pixel->g = 0;
			if(pixel->b < 0)
				pixel->b = 0;
			
			if(pixel->r > 1)
				pixel->r = 1;
			if(pixel->g > 1)
				pixel->g = 1;
			if(pixel->b > 1)
				pixel->b = 1;

			fprintf(fp, "%d %d %d\n", (int)(pixel->r * 255), (int)(pixel->g * 255), (int)(pixel->b * 255));
		}
	}

	fclose(fp);

}

Image *antialias_image(Image *in, int sampleSize) {
	Image *out = create_image(in->width, in->height, (Color){0, 0, 0});

	for(int i = 0; i < (int)in->height; i++) {
		for(int j = 0; j < (int)in->width; j++) {
			int sampleCount = 0;
			Color sampleColor = (Color){0, 0, 0};

			for(int k = i - sampleSize; k < i + sampleSize; k++) {
				for(int l = j - sampleSize; l < j + sampleSize; l++) {
					if(k > 0 && k < (int)in->height && l > 0 && l < (int)in->width) {
						sampleCount++;

						sampleColor.r += in->data[l + k * in->width].r;
						sampleColor.g += in->data[l + k * in->width].g;
						sampleColor.b += in->data[l + k * in->width].b;
					}
				}
			}

			sampleColor.r = sampleColor.r / sampleCount;
			sampleColor.g = sampleColor.g / sampleCount;
			sampleColor.b = sampleColor.b / sampleCount;

			out->data[j + i * out->width] = sampleColor;
		}
	}

	return out;
}

#endif

#endif
