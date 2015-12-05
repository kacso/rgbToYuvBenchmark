#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

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
	unsigned Height;
	unsigned Width;
	unsigned maxColorValue;
	unsigned propertiesLength;
} ImageProperties;

typedef void(*conversionFunction)(PixelRGB *rgbImage, ImageProperties imgProp, PixelYUV *yuvImage);



void printSomething(char* title, PixelRGB* out, int x, int y) {
	int j, k;
	printf("\n%s\n", title);
	for (j = 0; j < x; ++j) {
		for (k = 0; k < y; ++k) {
			printf("%0x %0x %0x ", out[j*x + k].R, out[j*x + k].G, out[j*x + k].B);
		}
		printf("\n");
	}
}

char* concat(char *s1, char *s2)
{
	int stringLen = strlen(s1) + strlen(s2) + 1;
	char *result = malloc(strlen(s1) + strlen(s2) + 1);//+1 for the zero-terminator
													   //in real code you would check for errors in malloc here
	strcpy_s(result, stringLen, s1);
	strcat_s(result, stringLen, s2);
	return result;
}

/**Reads properties from header of image*/
ImageProperties readImageProperties(FILE *imgFile) {
	char readedLine[10];
	ImageProperties imgProp;
	fseek(imgFile, 0L, SEEK_SET);

	/**Read type of file*/
	if (fscanf_s(imgFile, "%s", readedLine, 10) == EOF) {
		perror("EOF");
		exit(-2);
	}
	/**Check if type is P6*/
	if (!(readedLine[0] == 'P' && readedLine[1] == '6' && strlen(readedLine) == 2)) {
		perror("Wrong file type.");
		exit(-4);
	}

	/**Read image width, height and maximum color value*/
	do {
		if (fscanf_s(imgFile, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	imgProp.Width = (unsigned)atoi(readedLine);

	do {
		if (fscanf_s(imgFile, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	imgProp.Height = (unsigned)atoi(readedLine);

	do {
		if (fscanf_s(imgFile, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	imgProp.maxColorValue = (unsigned)atoi(readedLine);

	/**Read \n*/
	fscanf_s(imgFile, "%c", readedLine, 1);

	//fscanf_s(img, "%c", readedLine, 1);
	imgProp.propertiesLength = ftell(imgFile);
	return imgProp;
}

PixelRGB *loadRGBImage(FILE *imgFile, ImageProperties imgProp)
{
	PixelRGB *image = malloc(imgProp.Height * imgProp.Width * sizeof(PixelRGB));
	if (image == NULL) perror("Malloc error");
	fseek(imgFile, imgProp.propertiesLength, SEEK_SET);
	fread(image, sizeof(PixelRGB), imgProp.Height * imgProp.Width, imgFile);
	return image;
}

/**Converts from rgb to yuv
Values are returned through variables Y, Cb and Cr */
void standardRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp, PixelYUV *yuvImage) {

	unsigned int index, i, j;
	for (i = 0; i < imgProp.Height; i++) {
		for (j = 0; j < imgProp.Width; j++) {
			index = i * imgProp.Width + j;
			yuvImage[index].Y = 0.257f * rgbImage[index].R +
				0.504f * rgbImage[index].G + 0.098f * rgbImage[index].B + 16;
			yuvImage[index].U = -0.148f * rgbImage[index].R -
				0.291f * rgbImage[index].G + 0.439f * rgbImage[index].B + 128;
			yuvImage[index].V = 0.439f * rgbImage[index].R -
				0.368f * rgbImage[index].G - 0.071f * rgbImage[index].B + 128;
		}
	}
	// return yuv
}

void shiftRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp, PixelYUV *yuvImage) {
	unsigned int index, i, j;
	for (i = 0; i < imgProp.Height; i++) {
		for (j = 0; j < imgProp.Width; j++) {
			index = i * imgProp.Width + j;
			yuvImage[index].Y = ((66 * rgbImage[index].R +
				129 * rgbImage[index].G + 25 * rgbImage[index].B + 128) >> 8) + 16;
			yuvImage[index].U = ((-38 * rgbImage[index].R -
				74 * rgbImage[index].G + 112 * rgbImage[index].B + 128) >> 8) + 128;
			yuvImage[index].V = ((112 * rgbImage[index].R -
				94 * rgbImage[index].G - 18 * rgbImage[index].B + 128) >> 8) + 128;
		}
	}
}

void optimizedShiftRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp, PixelYUV *yuvImage) {
	unsigned int index, i, j;
	for (i = 0; i < imgProp.Height; i++) {
		for (j = 0; j < imgProp.Width; j++) {
			index = i * imgProp.Width + j;
			yuvImage[index].Y = ((66 * rgbImage[index].R +
				4 * 32 * rgbImage[index].G + 25 * rgbImage[index].B + 128) >> 8) + 16;
			yuvImage[index].U = ((-38 * rgbImage[index].R -
				74 * rgbImage[index].G + 112 * rgbImage[index].B + 128) >> 8) + 128;
			yuvImage[index].V = ((3 * 38 * rgbImage[index].R -
				3 * 32 * rgbImage[index].G - 18 * rgbImage[index].B + 128) >> 8) + 128;
		}
	}
}

/**Converts from rgb to yuv
Values are returned through variables Y, Cb and Cr */
PixelRGB* yuvToRgb(PixelYUV *yuvImage, ImageProperties imgProp) {
	PixelRGB *rgbImage = malloc(imgProp.Height * imgProp.Width * sizeof(PixelRGB));
	unsigned int index, i, j;
	for (i = 0; i < imgProp.Height; i++) {
		for (j = 0; j < imgProp.Width; j++) {
			index = i * imgProp.Width + j;

			rgbImage[index].R = 1.164f * (yuvImage[index].Y - 16) +
				1.596f * (yuvImage[index].V - 128);

			//rgbImage[index].R = 1;
			rgbImage[index].G = 1.164f * (yuvImage[index].Y - 16) -
				0.813f * (yuvImage[index].V - 128)
				- 0.391f * (yuvImage[index].U - 128);

			rgbImage[index].B = 1.164f * (yuvImage[index].Y - 16) +
				2.018f * (yuvImage[index].U - 128);
		}
	}
	return rgbImage;	
}


/**Reads one block of 8x8
Prerequest: *img is in front of right row
*/
//void readBlock(FILE *img, unsigned rowOffset, unsigned columnOffset, unsigned rowWidth, struct rgbPixel *readedBlock) {
//	struct rgbPixel *readedLine = malloc(rowWidth * sizeof(struct rgbPixel));
//	unsigned i, j;
//	if (readedLine == NULL) {
//		perror("Malloc error");
//	}
//
//	fseek(img, propertiesLength + rowOffset * rowWidth, SEEK_SET);
//
//	for (i = 0; i < BLOCK_HEIGHT; i++) {
//		fread(readedLine, sizeof(struct rgbPixel), rowWidth, img);
//		for (j = 0; j < BLOCK_WIDTH; j++) { 
//			readedBlock[i * BLOCK_WIDTH + j] = readedLine[columnOffset + j];
//		}
//	}
//	printSomething("Readed block", readedBlock, 8, 8);
//	free(readedLine);
//}

void saveHeaderOfppm(char* fileName, ImageProperties imgProp)
{
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "wb") != 0) {
		printf("Error while opening output file.");
		exit(-2);
	}

	fprintf(output, "P6\n%d %d\n%d\n", imgProp.Width, imgProp.Height, imgProp.maxColorValue);
	//fprintf(output, "P6");
	//fwrite(0x0a, sizeof(char), 1, output);
	//fprintf(output, "%d %d", imgWidth, imgHeight);
	//fwrite(0x0a, sizeof(char), 1, output);
	//fprintf(output, "%d", maxColorValue);
	//fwrite(0x0a, sizeof(char), 1, output);
	fclose(output);
}

void saveImgAsppm(char* fileName, PixelRGB *blocks, ImageProperties imgProp)
{
	//struct rgbPixel *readedLine = malloc(rowWidth * sizeof(struct rgbPixel));
	unsigned i, j;
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "wb") != 0) {
		printf("Error while opening output file.");
		exit(-2);
	}

	fprintf(output, "P6\n%d %d\n%d\n", imgProp.Width, imgProp.Height, imgProp.maxColorValue);

	for (i = 0; i < imgProp.Height; ++i) {
		for (j = 0; j < imgProp.Width; ++j) {
			fwrite(&blocks[i * imgProp.Width + j].R, sizeof(uint8_t), 1, output);
			fwrite(&blocks[i * imgProp.Width + j].G, sizeof(uint8_t), 1, output);
			fwrite(&blocks[i * imgProp.Width + j].B, sizeof(uint8_t), 1, output);
		}
	}
	fclose(output);
}

void saveBlocksToppm(char* fileName, PixelRGB **blocks, unsigned imageWidth)
{
	//struct rgbPixel *readedLine = malloc(rowWidth * sizeof(struct rgbPixel));
	unsigned i, j, k;
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "a") != 0 || output == NULL) {
		perror("Error while opening output file.");
		exit(-2);
	}

	fseek(output, 0, SEEK_END);

	for (i = 0; i < BLOCK_HEIGHT; ++i) {
		for (j = 0; j < imageWidth / BLOCK_WIDTH; ++j) {//do broja blokova u retku
			for (k = 0; k < BLOCK_WIDTH; ++k) {
				fwrite(&blocks[j][i * BLOCK_WIDTH + k].R, sizeof(uint8_t), 1, output);
				fwrite(&blocks[j][i * BLOCK_WIDTH + k].G, sizeof(uint8_t), 1, output);
				fwrite(&blocks[j][i * BLOCK_WIDTH + k].B, sizeof(uint8_t), 1, output);
			}
		}
		//fprintf(output, "\n");
		/*fread(readedLine, sizeof(struct rgbPixel), rowWidth, img);
		for (j = 0; j < BLOCK_WIDTH; j++) {
			readedBlock[i * BLOCK_WIDTH + j] = readedLine[columnOffset + j];
		}*/
	}
	fclose(output);
}

float TestConversion(PixelRGB *rgbImage, ImageProperties imgProp, PixelYUV *yuvImage, char* fileName, conversionFunction function)
{
	clock_t startTime, endTime;
	startTime = clock();
	function(rgbImage, imgProp, yuvImage);
	endTime = clock();
	PixelRGB *optimizedShiftRgbImage = yuvToRgb(yuvImage, imgProp);

	saveImgAsppm(fileName, optimizedShiftRgbImage, imgProp);
	free(optimizedShiftRgbImage);
	return endTime - startTime;
}

/**Input arguments
** 1. -> image name
*/
int main(int argc, char *argv[]) {
	float standardTime = 0, shiftTime = 0, optimizedShiftTime = 0;

	/**Check if 3 arguments are entered*/
	if (argc != 2) {
		perror("Wrong number of arguments.");
		return -1;
	}
	clock_t start = clock();

	/**Read input arguments*/
	char *imgDir;
	imgDir = argv[1];
	char* path = NULL;

	/* Get files from dir */
	DIR *pdir = opendir(imgDir);
	struct dirent *pent = NULL;
	if (pdir == NULL) // if pdir wasn't initialised correctly
	{ // print an error message and exit the program
		printf("\nERROR! pdir could not be initialised correctly\n");
		return; // exit the function
	}

	while (pent = readdir(pdir)) // while there is still something in the directory to list
	{
		if (pent == NULL) // if pent has not been initialised correctly
		{ // print an error message, and exit the program
			printf("\nERROR! pent could not be initialised correctly\n");
			return; // exit the function
		}
		// otherwise, it was initialised correctly. let's print it on the console:
		//printf("%s\n", pent->d_name);

		if (!strcmp(pent->d_name, "."))
			continue;
		if (!strcmp(pent->d_name, ".."))
			continue;

		path = concat(imgDir, "\\");
		path = concat(path, pent->d_name);
		printf("Running: %s\n", path);
		/**Open an image file*/

		FILE *imgFile = NULL;
		if (fopen_s(&imgFile, path, "rb") != 0) {
			perror("Error while opening file.\n");
			continue;
		}
		free(path);

		ImageProperties imgProp = readImageProperties(imgFile);
		PixelRGB *originalImage = loadRGBImage(imgFile, imgProp);
		fclose(imgFile);

		PixelYUV *yuvImage = malloc(imgProp.Height * imgProp.Width * sizeof(PixelYUV));

		char *outputFileName = concat("out\\original_", pent->d_name);
		saveImgAsppm(outputFileName, originalImage, imgProp);
		free(outputFileName);
		
		//clock_t startTime, endTime;
		/**Convert from RGB to yuv */

		/* Standard */
		standardTime += TestConversion(originalImage, imgProp, yuvImage, concat("out\\standardRgb_", pent->d_name), &standardRgbToYuv);

		/*startTime = clock();
		standardRgbToYuv(originalImage, imgProp, yuvImage);
		endTime = clock();
		standardTime += endTime - startTime;
		PixelRGB *standardRgbImage = yuvToRgb(yuvImage, imgProp);

		outputFileName = concat("out\\standardRgb_", pent->d_name);
		saveImgAsppm(outputFileName, standardRgbImage, imgProp);
		free(outputFileName);
		free(standardRgbImage);*/


		/* Shift */
		shiftTime += TestConversion(originalImage, imgProp, yuvImage, concat("out\\shiftRgb_", pent->d_name), &shiftRgbToYuv);

		/*startTime = clock();
		shiftRgbToYuv(originalImage, imgProp, yuvImage);
		endTime = clock();
		shiftTime += endTime - startTime;
		PixelRGB *shiftRgbImage = yuvToRgb(yuvImage, imgProp);

		outputFileName = concat("out\\shiftRgb_", pent->d_name);
		saveImgAsppm(outputFileName, shiftRgbImage, imgProp);
		free(outputFileName);
		free(shiftRgbImage);*/

		/* Optimized Shift */
		optimizedShiftTime += TestConversion(originalImage, imgProp, yuvImage, concat("out\\optimizedShiftRgb_", pent->d_name), &optimizedShiftRgbToYuv);
		
		/*startTime = clock();
		optimizedShiftRgbToYuv(originalImage, imgProp, yuvImage);
		endTime = clock();
		optimizedShiftTime += endTime - startTime;
		PixelRGB *optimizedShiftRgbImage = yuvToRgb(yuvImage, imgProp);

		outputFileName = concat("out\\optimizedShiftRgb_", pent->d_name);
		saveImgAsppm(outputFileName, optimizedShiftRgbImage, imgProp);
		free(outputFileName);
		free(optimizedShiftRgbImage);
		*/

		//printSomething("rgbImage", image, imgHeight, imgWidth);

		/* Free alocated space */	
		free(originalImage);
		free(yuvImage);
	}

	clock_t end = clock();
	printf("Standard conversion: %f\n", standardTime / CLOCKS_PER_SEC);
	printf("Shift conversion: %f\n", shiftTime / CLOCKS_PER_SEC);
	printf("Optmized shift conversion: %f\n", optimizedShiftTime / CLOCKS_PER_SEC);
	printf("Total time: %f\n", (float)(end - start) / CLOCKS_PER_SEC);

	return 0;
}