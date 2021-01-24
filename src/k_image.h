//============================================================================//
// k_image.h                                                                //
//============================================================================//

//----------------------------------------------------------------------------//
// interface                                                                  //
//----------------------------------------------------------------------------//

#ifndef K_IMAGE_H
#define K_IMAGE_H

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

Color color_add(Color a, Color b);
Color color_sub(Color a, Color b);
Color color_add_scalar(Color a, float b);
Color color_sub_scalar(Color a, float b);
Color color_mul_scalar(Color a, float b);
Color color_div_scalar(Color a, float b);
Image *create_image(unsigned int width, unsigned int height, Color bgColor);
void destroy_image(Image *image);
Image *antialias_image(Image *in, int sampleSize);
void write_image(Image *image, char *fileName);

//----------------------------------------------------------------------------//
// implementation                                                             //
//----------------------------------------------------------------------------//

#ifdef K_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

//---- private functions -----------------------------------------------------//

static void _write_hex(FILE *fp, char *s) {
	unsigned int i, c;
	char hex[3];

	for (i = 0; i < strlen(s); i += 2) {
		hex[0] = s[i];
		hex[1] = s[i+1];
		hex[2] = '\0';
		sscanf(hex,"%X",&c);
		putc(c,fp);
	}
}

//---- public functions ------------------------------------------------------//

Color color_add(Color a, Color b) {
	return (Color){a.r + b.r, a.g + b.g, a.b + b.b};
}

Color color_sub(Color a, Color b) {
	return (Color){a.r - b.r, a.g - b.g, a.b - b.b};
}

Color color_add_scalar(Color a, float b) {
	return (Color){a.r + b, a.g + b, a.b + b};
}

Color color_sub_scalar(Color a, float b) {
	return (Color){a.r - b, a.g - b, a.b - b};
}

Color color_mul_scalar(Color a, float b) {
	return (Color){a.r * b, a.g * b, a.b * b};
}

Color color_div_scalar(Color a, float b) {
	return (Color){a.r / b, a.g / b, a.b / b};
}

Image *create_image(unsigned int width, unsigned int height, Color bgColor) {
	Image *image = malloc(sizeof(Image));

	image->width = width;
	image->height = height;

	image->data = malloc(sizeof(Color) * width * height);

	for(unsigned int i = 0; i < width * height; i++) {
		image->data[i] = bgColor;
	}

	return image;
}

void destroy_image(Image *image) {
	free(image);
}

Image *antialias_image(Image *in, int sampleSize) {
	Image *out = create_image(in->width, in->height, (Color){0, 0, 0});

	for(unsigned int i = 0; i < in->height; i++) {
		for(unsigned int j = 0; j < in->width; j++) {
			int sampleCount = 0;
			Color sampleColor = (Color){0, 0, 0};

			for(unsigned int k = i - sampleSize; k < i + sampleSize; k++) {
				for(unsigned int l = j - sampleSize; l < j + sampleSize; l++) {
					if(k > 0 && k < in->height && l > 0 && l < in->width) {
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

/*
void write_image(Image *image, char *fileName) {
	FILE *fp = fopen(fileName, "wb");

	// write header
	_write_hex(fp, "4d4d002a");
	char offset = image->width * image->height * 3 + 8;
	putc((offset & 0xff000000) / 16777216,fp);
	putc((offset & 0x00ff0000) / 65536,fp);
	putc((offset & 0x0000ff00) / 256,fp);
	putc((offset & 0x000000ff),fp);

	// write data
	for (unsigned int i = 0; i < image->height; i++) {
		for (unsigned int j = 0; j < image->width; j++) {
			Color color = color_mul_scalar(image->data[j + i * image->width], 255);

			fputc(color.r, fp);
			fputc(color.g, fp);
			fputc(color.b, fp);
		}
	}

	// write the footer
	_write_hex(fp, "000e");

	// width
	_write_hex(fp, "0100000300000001");
	fputc((image->width & 0xff00) / 256, fp);
	fputc((image->width & 0x00ff), fp);
	_write_hex(fp, "0000");

	//height
	_write_hex(fp, "0101000300000001");
	fputc((image->height & 0xff00) / 256, fp);
	fputc((image->height & 0x00ff), fp);
	_write_hex(fp, "0000");

	// bits per sample
	_write_hex(fp, "0102000300000003");
	offset = image->width * image->height * 3 + 182;
	putc((offset & 0xff000000) / 16777216, fp);
	putc((offset & 0x00ff0000) / 65536, fp);
	putc((offset & 0x0000ff00) / 256, fp);
	putc((offset & 0x000000ff), fp);

	// compression
	_write_hex(fp, "010300030000000100010000");

	// photometric interpolation
	_write_hex(fp, "010600030000000100020000");

	// strip offset
	_write_hex(fp, "011100040000000100000008");

	// orientation
	_write_hex(fp, "011200030000000100010000");

	// samples per pixel
	_write_hex(fp, "011500030000000100030000");

	// rows per strip
	_write_hex(fp, "0116000300000001");
	fputc((image->height & 0xff00) / 256, fp);
	fputc((image->height & 0x00ff), fp);
	_write_hex(fp, "0000");

	// strip byte count
	_write_hex(fp, "0117000400000001");
	offset = image->width * image->height * 3;
	putc((offset & 0xff000000) / 16777216, fp);
	putc((offset & 0x00ff0000) / 65536, fp);
	putc((offset & 0x0000ff00) / 256, fp);
	putc((offset & 0x000000ff), fp);

	// minimum sample value
	_write_hex(fp, "0118000300000003");
	offset = image->width * image->height * 3 + 188;
	putc((offset & 0xff000000) / 16777216, fp);
	putc((offset & 0x00ff0000) / 65536, fp);
	putc((offset & 0x0000ff00) / 256, fp);
	putc((offset & 0x000000ff), fp);

	// maximum sample value
	_write_hex(fp, "0119000300000003");
	offset = image->width * image->height * 3 + 194;
	putc((offset & 0xff000000) / 16777216, fp);
	putc((offset & 0x00ff0000) / 65536, fp);
	putc((offset & 0x0000ff00) / 256, fp);
	putc((offset & 0x000000ff), fp);

	// planar configuration
	_write_hex(fp, "011c00030000000100010000");

	// sample format tag
	_write_hex(fp, "0153000300000003");
	offset = image->width * image->height * 3 + 200;
	putc((offset & 0xff000000) / 16777216, fp);
	putc((offset & 0x00ff0000) / 65536, fp);
	putc((offset & 0x0000ff00) / 256, fp);
	putc((offset & 0x000000ff), fp);

	// end of the directory entry
	_write_hex(fp, "00000000");

	// bits for each colour channel
	_write_hex(fp, "000800080008");

	// minimum value for each component
	_write_hex(fp, "000000000000");

	// maximum value per channel
	_write_hex(fp, "00ff00ff00ff");

	// samples per pixel for each channel
	_write_hex(fp, "000100010001");

	fclose(fp);

}
*/

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

#endif

#endif
