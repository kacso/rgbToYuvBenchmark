#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

typedef struct
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
} PixelRGB;

typedef struct
{
	unsigned Height;
	unsigned Width;
	unsigned maxColorValue;
	unsigned propertiesLength;
} ImageProperties;

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

char *appendString(char *s1, char *s2)
{
	size_t stringLen = strlen(s1) + strlen(s2) + 1;
	char *result = malloc(strlen(s1) + strlen(s2) + 1);
	strcpy_s(result, stringLen, s1);
	strcat_s(result, stringLen, s2);
	return result;
}

//Arguments:
//1 - original image
//2 - image to compare with
int main(int argc, char *argv[]) {
	printf("Starting analysis. \n");
	if (argc != 3) {
		perror("Wrong number of arguments.");
		return -1;
	}
	printf("comparing %s and %s \n", argv[1], argv[2]);
	//Loading first image
	FILE *firstImageFile = NULL;
	if (fopen_s(&firstImageFile, argv[1], "rb") != 0) {
		perror("Error while opening file.\n");
	}
	ImageProperties firstImgProp = readImageProperties(firstImageFile);
	PixelRGB *firstImage = loadRGBImage(firstImageFile, firstImgProp);
	fclose(firstImageFile);

	//Loading second image
	FILE *secondImageFile = NULL;
	if (fopen_s(&secondImageFile, argv[2], "rb") != 0) {
		perror("Error while opening file.\n");
	}
	ImageProperties secondImgProp = readImageProperties(secondImageFile);
	PixelRGB *secondImage = loadRGBImage(secondImageFile, secondImgProp);
	fclose(firstImageFile);
	if (firstImgProp.Height != secondImgProp.Height || firstImgProp.Width != secondImgProp.Width || firstImgProp.maxColorValue != secondImgProp.maxColorValue)
		perror("Error - Different file sizes.\n");

	unsigned int numberOfPixels = firstImgProp.Height * firstImgProp.Width;
	unsigned diffR, diffG, diffB, diffTotal, maxDiff = 0;
	unsigned int squaredError = 0;

	//Initialize difference array to 0
	int differenceCount[4][256 * 3]; // [R,G,B,Total] X [pixel difference]
	for (unsigned i = 0; i < 4; i++)	
		for (unsigned j = 0; j < 256 * 3; j++)		
			differenceCount[i][j] = 0;
	
	//Start comparison
	for (unsigned i = 0; i < numberOfPixels; i++) { // i = current pixel
		diffR = abs(firstImage[i].R - secondImage[i].R);
		diffG = abs(firstImage[i].G - secondImage[i].G);
		diffB = abs(firstImage[i].B - secondImage[i].B);
		diffTotal = diffR + diffG + diffB;
		(differenceCount[0][diffR]) ++;
		(differenceCount[1][diffG]) ++;
		(differenceCount[2][diffB]) ++;
		(differenceCount[3][diffTotal]) ++;
		if (diffTotal > 0) squaredError += (diffTotal * diffTotal);
		if (diffTotal > maxDiff) maxDiff = diffTotal;	
	}

	//Difference count check
	unsigned diffSum = 0;
	for (unsigned i = 0; i < 256 * 3; i++) diffSum += differenceCount[0][i];
	if(diffSum != numberOfPixels) printf("Wrong count of red different pixels! \n");
	diffSum = 0;
	for (unsigned i = 0; i < 256 * 3; i++) diffSum += differenceCount[1][i];
	if (diffSum != numberOfPixels) printf("Wrong count of green different pixels! \n");
	diffSum = 0;
	for (unsigned i = 0; i < 256 * 3; i++) diffSum += differenceCount[2][i];
	if (diffSum != numberOfPixels) printf("Wrong count of blue different pixels! \n");

	//Format and save comparison results
	double meanSquaredError = (squaredError / numberOfPixels);
	printf("Mean squared error: %f \n", meanSquaredError);
	char *resultsFileName = "comparison_";
	resultsFileName = appendString(resultsFileName, argv[1]);
	resultsFileName = appendString(resultsFileName, "_");
	resultsFileName = appendString(resultsFileName, argv[2]);
	resultsFileName = appendString(resultsFileName, ".csv");
	FILE *outFile;
	printf("\n%s\n", resultsFileName);
	fopen_s(&outFile, resultsFileName, "w");
	if (outFile == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(outFile, "Different pixels\t Red \t(percent) \tGreen \t(percent) \tBlue \t(percent) \tTotal \t(percent) \n");
	
	for (unsigned i = 0; i <= maxDiff; i++)
	{
		fprintf(outFile,"%d \t %6u \t%f \t%6u \t%f \t%6u \t%f \t%6u \t%f \n",
			i,
			differenceCount[0][i], 
			((float)(100 * differenceCount[0][i]) / numberOfPixels),
			differenceCount[1][i],
			((float)(100 * differenceCount[1][i]) / numberOfPixels),
			differenceCount[2][i],
			((float)(100 * differenceCount[2][i]) / numberOfPixels),
			differenceCount[3][i],
			((float)(100 * differenceCount[3][i]) / numberOfPixels)
			);
	}

	fclose(outFile);
	free(firstImage);
	free(secondImage);
	return 0;
}