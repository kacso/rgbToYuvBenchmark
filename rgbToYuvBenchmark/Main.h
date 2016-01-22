#ifndef MAIN_H
#define MAIN_H

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <ipp.h>

#ifndef HAVE_STDINT_H
#define HAVE_STDINT_H
#include <stdint.h>
#endif

#define BLOCK_WIDTH 8
#define BLOCK_HEIGHT 8
#define PI M_PI
#define OUT_DIR "out\\"

typedef struct
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
} PixelRGB;

typedef struct
{
	float Y;
	float U;
	float V;
} PixelYUV;

typedef struct
{
	int8_t Y;
	int8_t U;
	int8_t V;
}PixelQuantized;

typedef struct
{
	unsigned Height;
	unsigned Width;
	unsigned maxColorValue;
	unsigned propertiesLength;
} ImageProperties;

#endif