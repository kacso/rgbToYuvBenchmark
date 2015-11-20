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

unsigned propertiesLength = 0;

struct rgbPixel
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
};

struct yuvPixel
{
	float Y;
	float U;
	float V;
};


void printSomething(char* title, struct rgbPixel* out, int x, int y) {
	int j, k;
	printf("\n%s\n", title);
	for (j = 0; j < x; ++j) {
		for (k = 0; k < y; ++k) {
			printf("%0x %0x %0x ", out[j*x+k].R, out[j*x + k].G, out[j*x + k].B);
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
void readImageProperties(FILE *img, unsigned *imgHeight, unsigned *imgWidth,
	unsigned *maxColorValue) {
	char readedLine[10];

	fseek(img, 0L, SEEK_SET);

	/**Read type of file*/
	if (fscanf_s(img, "%s", readedLine, 10) == EOF) {
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
		if (fscanf_s(img, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	*imgWidth = (unsigned)atoi(readedLine);

	do {
		if (fscanf_s(img, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	*imgHeight = (unsigned)atoi(readedLine);

	do {
		if (fscanf_s(img, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	*maxColorValue = (unsigned)atoi(readedLine);

	/**Read \n*/
	fscanf_s(img, "%c", readedLine, 1);

	//fscanf_s(img, "%c", readedLine, 1);
	propertiesLength = ftell(img);

}

struct rgbPixel *readImage(FILE *img, unsigned *imgHeight, unsigned *imgWidth,
	unsigned *maxColorValue)
{
	unsigned i, j;
	readImageProperties(img, imgHeight, imgWidth, maxColorValue);


	struct rgbPixel *image = malloc(*imgHeight * *imgWidth * sizeof(struct rgbPixel));
	if (image == NULL) {
		perror("Malloc error");
	}

	fseek(img, propertiesLength, SEEK_SET);

	fread(image, sizeof(struct rgbPixel), *imgHeight * *imgWidth, img);
	return image;
}


/**Converts from rgb to yuv
Values are returned through variables Y, Cb and Cr */
void standardRgbToYuv(struct rgbPixel *rgbImage, unsigned imgHeight, unsigned imgWidth, unsigned BLOCK_NUM, struct yuvPixel *yuvImage) {
	int i, j;
	unsigned rowOffset = BLOCK_NUM / (imgWidth / BLOCK_WIDTH) * BLOCK_HEIGHT;
	unsigned columnOffset = (BLOCK_NUM % (imgWidth / BLOCK_WIDTH))  * BLOCK_WIDTH;
	unsigned index;

	for (i = 0; i < BLOCK_HEIGHT; i++) {
		for (j = 0; j < BLOCK_WIDTH; j++) {
			index = (i + rowOffset) * imgWidth + j + columnOffset;

			if (index < 0 || index > imgHeight*imgWidth)
				continue;

			yuvImage[index].Y = 0.257 * rgbImage[index].R +
				0.504 * rgbImage[index].G + 0.098 * rgbImage[index].B + 16;
			yuvImage[index].U = -0.148 * rgbImage[index].R -
				0.291 * rgbImage[index].G + 0.439 * rgbImage[index].B + 128;
			yuvImage[index].V = 0.439 * rgbImage[index].R -
				0.368 * rgbImage[index].G - 0.071 * rgbImage[index].B + 128;
		}
	}
}

void shiftRgbToYuv(struct rgbPixel *rgbImage, unsigned imgHeight, unsigned imgWidth, unsigned BLOCK_NUM, struct yuvPixel *yuvImage) {
	int i, j;
	unsigned rowOffset = BLOCK_NUM / (imgWidth / BLOCK_WIDTH) * BLOCK_HEIGHT;
	unsigned columnOffset = (BLOCK_NUM % (imgWidth / BLOCK_WIDTH))  * BLOCK_WIDTH;
	unsigned index;

	for (i = 0; i < BLOCK_HEIGHT; i++) {
		for (j = 0; j < BLOCK_WIDTH; j++) {
			index = (i + rowOffset) * imgWidth + j + columnOffset;

			if (index < 0 || index > imgHeight*imgWidth)
				continue;

			yuvImage[index].Y = ((66 * rgbImage[index].R +
				129 * rgbImage[index].G + 25 * rgbImage[index].B  + 128) >> 8) + 16;
			yuvImage[index].U = ((-38 * rgbImage[index].R -
				74 * rgbImage[index].G + 112 * rgbImage[index].B + 128) >> 8) + 128;
			yuvImage[index].V = ((112 * rgbImage[index].R -
				94 * rgbImage[index].G - 18 * rgbImage[index].B + 128) >> 8) + 128;
		}
	}
}


void optimizedShiftRgbToYuv(struct rgbPixel *rgbImage, unsigned imgHeight, unsigned imgWidth, unsigned BLOCK_NUM, struct yuvPixel *yuvImage) {
	int i, j;
	unsigned rowOffset = BLOCK_NUM / (imgWidth / BLOCK_WIDTH) * BLOCK_HEIGHT;
	unsigned columnOffset = (BLOCK_NUM % (imgWidth / BLOCK_WIDTH))  * BLOCK_WIDTH;
	unsigned index;

	for (i = 0; i < BLOCK_HEIGHT; i++) {
		for (j = 0; j < BLOCK_WIDTH; j++) {
			index = (i + rowOffset) * imgWidth + j + columnOffset;

			if (index < 0 || index > imgHeight*imgWidth)
				continue;

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
void yuvToRgb(struct yuvPixel *yuvImage, unsigned imgHeight, unsigned imgWidth, unsigned BLOCK_NUM, struct rgbPixel *rgbImage) {
	int i, j;
	unsigned index;

	unsigned rowOffset = BLOCK_NUM / (imgWidth / BLOCK_WIDTH) * BLOCK_HEIGHT;
	unsigned columnOffset = (BLOCK_NUM % (imgWidth / BLOCK_WIDTH))  * BLOCK_WIDTH;

	for (i = 0; i < BLOCK_HEIGHT; i++) {
		for (j = 0; j < BLOCK_WIDTH; j++) {
			index = (i + rowOffset) * imgWidth + j + columnOffset;

			if (index < 0 || index > imgHeight*imgWidth)
				continue;
			rgbImage[index].R = 1.164 * (yuvImage[index].Y - 16) +
				1.596 * (yuvImage[index].V - 128);

			//rgbImage[index].R = 1;
			rgbImage[index].G = 1.164 * (yuvImage[index].Y - 16) -
				0.813 * (yuvImage[index].V - 128)
				- 0.391 * (yuvImage[index].U - 128);

			rgbImage[index].B = 1.164 * (yuvImage[index].Y - 16) +
				2.018 * (yuvImage[index].U - 128);
		}
	}
}


/**Reads one block of 8x8
Prerequest: *img is in front of right row
*/
void readBlock(FILE *img, unsigned rowOffset, unsigned columnOffset, unsigned rowWidth, struct rgbPixel *readedBlock) {
	struct rgbPixel *readedLine = malloc(rowWidth * sizeof(struct rgbPixel));
	unsigned i, j;
	if (readedLine == NULL) {
		perror("Malloc error");
	}

	fseek(img, propertiesLength + rowOffset * rowWidth, SEEK_SET);

	for (i = 0; i < BLOCK_HEIGHT; i++) {
		fread(readedLine, sizeof(struct rgbPixel), rowWidth, img);
		for (j = 0; j < BLOCK_WIDTH; j++) { 
			readedBlock[i * BLOCK_WIDTH + j] = readedLine[columnOffset + j];
		}
	}
	printSomething("Readed block", readedBlock, 8, 8);
	free(readedLine);
}

void saveHeaderOfppm(char* fileName, unsigned imgHeight, unsigned imgWidth,
	unsigned maxColorValue)
{
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "wb") != 0) {
		printf("Error while opening output file.");
		exit(-2);
	}

	fprintf(output, "P6\n%d %d\n%d\n", imgWidth, imgHeight, maxColorValue);
	//fprintf(output, "P6");
	//fwrite(0x0a, sizeof(char), 1, output);
	//fprintf(output, "%d %d", imgWidth, imgHeight);
	//fwrite(0x0a, sizeof(char), 1, output);
	//fprintf(output, "%d", maxColorValue);
	//fwrite(0x0a, sizeof(char), 1, output);
	fclose(output);
}

void saveImgAsppm(char* fileName, struct rgbPixel *blocks, unsigned imgWidth, unsigned imgHeight, unsigned maxColorValue)
{
	//struct rgbPixel *readedLine = malloc(rowWidth * sizeof(struct rgbPixel));
	unsigned i, j, k;
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "wb") != 0) {
		printf("Error while opening output file.");
		exit(-2);
	}

	fprintf(output, "P6\n%d %d\n%d\n", imgWidth, imgHeight, maxColorValue);

	for (i = 0; i < imgHeight; ++i) {
		for (j = 0; j < imgWidth; ++j) {
			fwrite(&blocks[i * imgWidth + j].R, sizeof(uint8_t), 1, output);
			fwrite(&blocks[i * imgWidth + j].G, sizeof(uint8_t), 1, output);
			fwrite(&blocks[i * imgWidth + j].B, sizeof(uint8_t), 1, output);
		}
	}

	fclose(output);
}

void saveBlocksToppm(char* fileName, struct rgbPixel **blocks, unsigned imageWidth)
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




/**Input arguments
** 1. -> image name
*/
int main(int argc, char *argv[]) {
	char *imgDir;
	unsigned int blockNum = 0, imgHeight, imgWidth,
		maxColorValue, rowOffset, columnOffset, BLOCK_NUM, i, j, k;
	FILE *img = NULL;
	struct rgbPixel *standardRgbImage, *shiftRgbImage, *optimizedShiftRgbImage;
	struct yuvPixel *yuvImage;
	struct rgbPixel **original;
	struct rgbPixel *image;
	float standardTime = 0, shiftTime = 0, optimizedShiftTime = 0;

	/**Check if 3 arguments are entered*/
	if (argc != 2) {
		perror("Wrong number of arguments.");
		return -1;
	}

	clock_t start = clock();

	/**Read input arguments*/
	imgDir = argv[1];

	char* path = NULL;

	clock_t sRgb2Yuv, eRgb2Yuv;

	/* Get files from dir */
	DIR *pdir = opendir(imgDir);
	struct dirent *pent = NULL;
	if (pdir == NULL) // if pdir wasn't initialised correctly
	{ // print an error message and exit the program
		printf("\nERROR! pdir could not be initialised correctly\n");
		return; // exit the function
	} // end if

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
		if (fopen_s(&img, path, "rb") != 0) {
			perror("Error while opening file.\n");
			continue;
		}
		free(path);

		image = readImage(img, &imgHeight, &imgWidth, &maxColorValue);
		///**Read properties*/
		//readImageProperties(img, &imgHeight, &imgWidth, &maxColorValue);

		BLOCK_NUM = (imgWidth / BLOCK_WIDTH) * (imgHeight / BLOCK_HEIGHT);

		standardRgbImage = malloc(imgWidth * imgHeight * sizeof(struct rgbPixel));
		shiftRgbImage = malloc(imgWidth * imgHeight * sizeof(struct rgbPixel));
		optimizedShiftRgbImage = malloc(imgWidth * imgHeight * sizeof(struct rgbPixel));
		yuvImage = malloc(imgWidth * imgHeight * sizeof(struct yuvPixel));

		for (i = 0; i < BLOCK_NUM; ++i) {
			/**Convert from RGB to yuv */
			/* Standard */
			sRgb2Yuv = clock();
			standardRgbToYuv(image, imgHeight, imgWidth, i, yuvImage);
			eRgb2Yuv = clock();
			standardTime += eRgb2Yuv - sRgb2Yuv;
			yuvToRgb(yuvImage, imgHeight, imgWidth, i, standardRgbImage);

			/* Shift */
			sRgb2Yuv = clock();
			shiftRgbToYuv(image, imgHeight, imgWidth, i, yuvImage);
			eRgb2Yuv = clock();
			shiftTime += eRgb2Yuv - sRgb2Yuv;
			yuvToRgb(yuvImage, imgHeight, imgWidth, i, shiftRgbImage);

			/* Optimized Shift */
			sRgb2Yuv = clock();
			optimizedShiftRgbToYuv(image, imgHeight, imgWidth, i, yuvImage);
			eRgb2Yuv = clock();
			optimizedShiftTime += eRgb2Yuv - sRgb2Yuv;
			yuvToRgb(yuvImage, imgHeight, imgWidth, i, optimizedShiftRgbImage);
		}

		//printSomething("rgbImage", image, imgHeight, imgWidth);


		char *out = concat("out\\original_", pent->d_name);
		saveImgAsppm(out, image, imgWidth, imgHeight, maxColorValue);

		free(out);
		out = concat("out\\standardRgb_", pent->d_name);
		saveImgAsppm(out, standardRgbImage, imgWidth, imgHeight, maxColorValue);

		free(out);
		out = concat("out\\shiftRgb_", pent->d_name);
		saveImgAsppm(out, shiftRgbImage, imgWidth, imgHeight, maxColorValue);

		free(out);
		out = concat("out\\optimizedShiftRgb_", pent->d_name);
		saveImgAsppm(out, optimizedShiftRgbImage, imgWidth, imgHeight, maxColorValue);

		/* Free alocated space */
		free(out);
		free(image);
		free(standardRgbImage);
		free(shiftRgbImage);
		free(yuvImage);
	}


	clock_t end = clock();
	printf("Standard conversion: %f\n", standardTime / CLOCKS_PER_SEC);
	printf("Shift conversion: %f\n", shiftTime / CLOCKS_PER_SEC);
	printf("Optmized shift conversion: %f\n", optimizedShiftTime / CLOCKS_PER_SEC);
	printf("Total time: %f\n", (float)(end - start) / CLOCKS_PER_SEC);


	/**Close image file it's not needed*/
	fclose(img);
	return 0;
}