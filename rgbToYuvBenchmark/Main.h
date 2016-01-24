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
#include <stdbool.h>
#include <stdint.h>

// #include <ipp.h>
#define PI M_PI
#define OUT_DIR "out\\"
#define DCT_BLOCK_DIM 8
#define DCT_BLOCK_SIZE 64

typedef struct
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
} PixelRGB;

typedef struct
{
	uint8_t Y;
	uint8_t U;
	uint8_t V;
} PixelYUV;

typedef struct
{
	float Y;
	float U;
	float V;
} PixelYUV_F;

typedef struct
{
	unsigned Height;
	unsigned Width;
	unsigned maxColorValue;
	unsigned propertiesLength;
} ImageProperties;


typedef uint8_t Image_block[DCT_BLOCK_SIZE];
typedef Image_block* BlockRow;

typedef struct {
	BlockRow  Y_blocks;
	BlockRow  U_blocks;
	BlockRow  V_blocks;
	int numberOfBlocks;
	int numberOfColumns;
	int numberOfRows;
} block_struct;


#define MEMZERO(target)	memset((target), 0,(sizeof(target)))
#define ERREXIT(msg) { perror(msg); exit(EXIT_FAILURE); }

//Loading & Saving
ImageProperties readImageProperties(FILE *imgFile);
PixelRGB *loadRGBImage(FILE *imgFile, ImageProperties imgProp);
void saveImgAsppm(char* fileName, PixelRGB *blocks, ImageProperties imgProp);

//Analysis
void ComparePictures(PixelRGB* img1, ImageProperties* prop_img1, PixelRGB* img2, ImageProperties* prop_img2, FILE* outFile);

//Format Conversion
PixelYUV * standardRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp);
PixelYUV * shiftRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp);
PixelYUV * optimizedShiftRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp);
//PixelYUV * ippRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp);
PixelRGB* yuvToRgb(PixelYUV *yuvImage, ImageProperties imgProp);

//Structure Conversion
block_struct* convert_pixel_array_to_blocks(PixelYUV *yuvImg, ImageProperties *imgProp);
PixelYUV* convert_block_to_pixel_array(block_struct *blocks);

//DCT
block_struct* dct_1(block_struct* blocks);
block_struct* dct_2(block_struct* blocks);
block_struct* dct_3(block_struct* blocks);

//IDCT
block_struct* idct_1(block_struct* blocks);

//Quantization
block_struct* quantization(block_struct* blocks);

//Entropy Coding
void encode_picture(block_struct *imageBlocks, ImageProperties *imgProp, FILE *outFile);

//Internal
typedef void *(*transform_block)(Image_block oldBlock, Image_block newblock);
void translateValuesOfBlock(BlockRow rowOfBlocks, int numOfBlocks, int correction);
#endif