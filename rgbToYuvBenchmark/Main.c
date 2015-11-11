#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define BLOCK_WIDTH 8
#define BLOCK_HEIGHT 8
#define PI M_PI

unsigned propertiesLength = 0;

struct rgbPixel
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
};

struct YCbCrPixel
{
	float Y;
	float Cb;
	float Cr;
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

	fscanf_s(img, "%c", readedLine, 1);
	propertiesLength = ftell(img);

}

/**Converts from rgb to YCbCr
Values are returned through variables Y, Cb and Cr */
struct YCbCrPixel *rgbToYCbCr(struct rgbPixel *rgbBlock) {
	int i, j;
	struct YCbCrPixel *yCbCrBlock =
		malloc(BLOCK_HEIGHT * BLOCK_WIDTH * sizeof(struct YCbCrPixel));
	for (i = 0; i < BLOCK_HEIGHT; i++) {
		for (j = 0; j < BLOCK_WIDTH; j++) {
			yCbCrBlock[i * BLOCK_WIDTH + j].Y = 0.299 * rgbBlock[i * BLOCK_WIDTH + j].R +
				0.587 * rgbBlock[i * BLOCK_WIDTH + j].G + 0.114 * rgbBlock[i * BLOCK_WIDTH + j].B;
			yCbCrBlock[i * BLOCK_WIDTH + j].Cb = -0.1687 * rgbBlock[i * BLOCK_WIDTH + j].R -
				0.3313 * rgbBlock[i * BLOCK_WIDTH + j].G + 0.5 * rgbBlock[i * BLOCK_WIDTH + j].B + 128;
			yCbCrBlock[i * BLOCK_WIDTH + j].Cr = 0.5 * rgbBlock[i * BLOCK_WIDTH + j].R -
				0.4187 * rgbBlock[i * BLOCK_WIDTH + j].G - 0.0813 * rgbBlock[i * BLOCK_WIDTH + j].B + 128;
		}
	}
	return yCbCrBlock;
}

/**Converts from rgb to YCbCr
Values are returned through variables Y, Cb and Cr */
struct rgbPixel *YCbCrToRgb(struct YCbCrPixel *yCbCrBlock) {
	int i, j;
	struct rgbPixel *rgbBlock =
		malloc(BLOCK_HEIGHT * BLOCK_WIDTH * sizeof(struct rgbPixel));
	for (i = 0; i < BLOCK_HEIGHT; i++) {
		for (j = 0; j < BLOCK_WIDTH; j++) {
			rgbBlock[i * BLOCK_WIDTH + j].R = 0.299 * yCbCrBlock[i * BLOCK_WIDTH + j].Y +
				1.402 * (yCbCrBlock[i * BLOCK_WIDTH + j].Cr - 128);

			rgbBlock[i * BLOCK_WIDTH + j].G = yCbCrBlock[i * BLOCK_WIDTH + j].Y -
				0.34414 * (yCbCrBlock[i * BLOCK_WIDTH + j].Cb - 128) 
				- 0.71414 * (yCbCrBlock[i * BLOCK_WIDTH + j].Cr - 128);

			rgbBlock[i * BLOCK_WIDTH + j].B = yCbCrBlock[i * BLOCK_WIDTH + j].Y -
				1.772 * (yCbCrBlock[i * BLOCK_WIDTH + j].Cb - 128);
		}
	}
	return rgbBlock;
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
	if (fopen_s(&output, fileName, "w") != 0) {
		printf("Error while opening output file.");
		exit(-2);
	}

	fprintf(output, "P6\n%d %d\n%d\n", imgWidth, imgHeight, maxColorValue);
	fclose(output);
}

void saveImgAsppm(char* fileName, struct rgbPixel *blocks, unsigned imgWidth, unsigned imgHeight, unsigned maxColorValue)
{
	//struct rgbPixel *readedLine = malloc(rowWidth * sizeof(struct rgbPixel));
	unsigned i, j, k;
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "w") != 0) {
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
	char *imgName;
	unsigned int blockNum = 0, imgHeight, imgWidth,
		maxColorValue, rowOffset, columnOffset, BLOCK_NUM, i, j, k;
	FILE *img = NULL;
	struct rgbPixel *rgbBlock, *rgbImage;
	struct YCbCrPixel *yCbCrBlock;
	struct rgbPixel *convertedRgbBlock;
	struct rgbPixel **rgbBlocks, **original;

	/**Check if 3 arguments are entered*/
	if (argc != 2) {
		perror("Wrong number of arguments.");
		return -1;
	}

	clock_t start = clock();

	/**Read input arguments*/
	imgName = argv[1];
	/*blockNum = atoi(argv[2]);
	outputName = argv[3];*/

	/**Open an image file*/
	if (fopen_s(&img, imgName, "rb") != 0) {
		perror("Error while opening file.");
		return -2;
	}

	/**Read properties*/
	readImageProperties(img, &imgHeight, &imgWidth, &maxColorValue);

	BLOCK_NUM = (imgWidth / BLOCK_WIDTH) * (imgHeight / BLOCK_HEIGHT);

	rgbImage = malloc(imgWidth * imgHeight * sizeof(struct rgbPixel));
	//saveHeaderOfppm("out.ppm", imgHeight, imgWidth, maxColorValue);
	//saveHeaderOfppm("out_original.ppm", imgHeight, imgWidth, maxColorValue);
	//rgbBlocks = malloc(imgWidth / BLOCK_WIDTH * sizeof(struct rgbPixel *));
	original = malloc(imgWidth / BLOCK_WIDTH * sizeof(struct rgbPixel *));
	for (i = 0; i < BLOCK_NUM; ++i) {
		/**Find block to read*/
		rowOffset = i / (imgWidth / BLOCK_WIDTH) * BLOCK_HEIGHT;
		columnOffset = (i % (imgWidth / BLOCK_WIDTH))  * BLOCK_WIDTH;
		//printf("Row: %d, column: %d, imgWidth: %d.\n", rowOffset, columnOffset, imgWidth);

		/**Read defined block*/
		rgbBlock = malloc(BLOCK_HEIGHT * BLOCK_WIDTH * sizeof(struct rgbPixel));
		readBlock(img, rowOffset, columnOffset, imgWidth, rgbBlock);


		/**Convert from RGB to YCbCr*/
		yCbCrBlock = rgbToYCbCr(rgbBlock);
		convertedRgbBlock = YCbCrToRgb(yCbCrBlock);
		//rgbBlocks[i % (imgWidth / BLOCK_WIDTH)] = convertedRgbBlock;
		//original[i % (imgWidth / BLOCK_WIDTH)] = rgbBlock;
		//printf("rgbBlock\n");
		for (j = 0; j < BLOCK_HEIGHT; ++j) {
			for (k = 0; k < BLOCK_WIDTH; ++k) {
				unsigned index = (rowOffset + j)* imgWidth + columnOffset + k;
				rgbImage[index].R = rgbBlock[j * BLOCK_WIDTH + k].R;
				rgbImage[index].G = rgbBlock[j * BLOCK_WIDTH + k].G;
				rgbImage[index].B = rgbBlock[j * BLOCK_WIDTH + k].B;
				//printf("%0x %0x %0x", rgbBlock[j * BLOCK_WIDTH + k].R, rgbBlock[j * BLOCK_WIDTH + k].G, rgbBlock[j * BLOCK_WIDTH + k].B);
			}
			//printf("\n");
		}
		printSomething("rgbBlock", rgbBlock, 8,8);
		//free(rgbBlock);

		if (i != 0 && i % (imgWidth / BLOCK_WIDTH) == 0) {
			/**Save*/
			//saveBlocksToppm("out.ppm", rgbBlocks, imgWidth);
			//SPREMAJ CIJELU SLIKU U POLJE, A ZATIM POLJE U DATOTEKU

			//for (i = 0; i < BLOCK_HEIGHT; ++i) {
			//	for (j = 0; j < imgWidth / BLOCK_WIDTH; ++j) {//do broja blokova u retku
			//		free(rgbBlocks[j][0]);
			//		free(rgbBlocks[j]);
			//	}
			//}
			//free(rgbBlocks);
		}
	}

	printSomething("rgbImage", rgbImage, imgHeight, imgWidth);

	saveImgAsppm("out_original.ppm", rgbImage, imgWidth, imgHeight, maxColorValue);
	clock_t end = clock();

	printf("%f\n", (float)(end - start) / CLOCKS_PER_SEC);
	/**Close image file it's not needed*/
	fclose(img);
	return 0;
}