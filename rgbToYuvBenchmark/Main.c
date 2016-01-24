#include "Main.h"

typedef PixelYUV *(*conversionFunction)(PixelRGB *rgbImage, ImageProperties imgProp);

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

//Returns conversion Time
long TestRGBtoYuvConversionFunction(PixelRGB *rgbImage, ImageProperties imgProp, char* imageFilName, char* conversionFunctionName,  conversionFunction conversionFunc, FILE *outputFile)
{
	fprintf(outputFile, "\nFile: %s \t Conversion Function: %s \n", imageFilName, conversionFunctionName);
	clock_t startTime, endTime, timeDiff;
	startTime = clock();
	PixelYUV *yuvImage = conversionFunc(rgbImage, imgProp);	
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(outputFile, "Conversion Duration: %i (ms) \n", (int)timeDiff);
	fprintf(outputFile, "Analysis: \n");
	PixelRGB* convertedBack =  yuvToRgb(yuvImage, imgProp);
	ComparePictures(rgbImage, &imgProp, convertedBack, &imgProp, outputFile);
	free(yuvImage);
	free(convertedBack);
	return timeDiff;
}


void toJPEG(PixelRGB *rgbImage, ImageProperties imgProp, FILE *outputFile) {

	clock_t startTime, endTime, timeDiff;
	PixelYUV *yuvImage = shiftRgbToYuv(rgbImage, imgProp);
	PixelRGB* YUV_to_RGB = yuvToRgb(yuvImage, imgProp);
	fprintf(outputFile, "Comparison: Original RGB -> YUV ->RGB2 \n");
	ComparePictures(rgbImage, &imgProp, YUV_to_RGB, &imgProp, outputFile);

	block_struct* blocks = convert_pixel_array_to_blocks(yuvImage, &imgProp);

	//DCT
	startTime = clock();
	block_struct* dctImg = dct_1(blocks);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(outputFile, "\nDCT conversion duration: %i (ms) \n", (int)timeDiff);
	
	//IDCT
	startTime = clock();
	block_struct* idctImg = idct_1(dctImg);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(outputFile, "IDCT conversion duration: %i (ms) \n", (int)timeDiff);

	PixelYUV* backFromBlocks = convert_block_to_pixel_array(idctImg);
	PixelRGB* rgbFromIdct = yuvToRgb(yuvImage, imgProp);

	fprintf(outputFile, "\nComparison: YUV converted with IDCT \n");
	ComparePictures(yuvImage, &imgProp, rgbFromIdct, &imgProp, outputFile);

	fprintf(outputFile, "\nComparison: Original -> DCT -> IDCT -> RGB \n");
	ComparePictures(rgbImage, &imgProp, rgbFromIdct, &imgProp, outputFile);


	//DCT2
	startTime = clock();
	block_struct* dct2Img = dct_1(blocks);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(outputFile, "\nDCT2 conversion duration: %i (ms) \n", (int)timeDiff);

	//IDCT (DCT2)
	block_struct* idct2Img = idct_1(dctImg);
	PixelYUV* idct2_yuv = convert_block_to_pixel_array(idct2Img);
	PixelRGB* idct2_rgb = yuvToRgb(idct2_yuv, imgProp);

	fprintf(outputFile, "\nComparison: YUV converted with IDCT (DCT2) \n");
	ComparePictures(yuvImage, &imgProp, idct2_rgb, &imgProp, outputFile);

	fprintf(outputFile, "\nComparison: Original -> DCT2 -> IDCT  -> RGB \n");
	ComparePictures(rgbImage, &imgProp, idct2_rgb, &imgProp, outputFile);

	//DCT3
	startTime = clock();
	block_struct* dct3Img = dct_1(blocks);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(outputFile, "\nDCT3 conversion duration: %i (ms) \n", (int)timeDiff);
	// IDCT(DCT2)
	block_struct* idct3Img = idct_1(dctImg);
	PixelYUV* idct3_yuv = convert_block_to_pixel_array(idct2Img);
	PixelRGB* idct3_rgb = yuvToRgb(idct2_yuv, imgProp);

	fprintf(outputFile, "\nComparison: YUV converted with IDCT (DCT3) \n");
	ComparePictures(yuvImage, &imgProp, idct3_rgb, &imgProp, outputFile);

	fprintf(outputFile, "\nComparison: Original -> DCT3 -> IDCT  -> RGB \n");
	ComparePictures(rgbImage, &imgProp, idct3_rgb, &imgProp, outputFile);

	free(yuvImage);
	free(YUV_to_RGB);
	free(blocks);
	free(backFromBlocks);
	free(rgbFromIdct);
	free(dct2Img);
	free(idct2Img);
	free(idct2_yuv);
	free(idct2_rgb);
	free(dct3Img);
	free(idct3Img);
	free(idct3_yuv);
	free(idct3_rgb);
}

/**Input arguments
** 1. -> image name
*/
int main(int argc, char *argv[]) {
	long standardTime = 0, shiftTime = 0, optimizedShiftTime = 0, ippTime = 0;
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

	FILE *resultOutFile;
	fopen_s(&resultOutFile, "Results.txt", "w");

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

		char *outputFileName = concat("out\\original_", pent->d_name);
		saveImgAsppm(outputFileName, originalImage, imgProp);
		free(outputFileName);
		

		/**Convert from RGB to yuv */
		/* Standard */
		standardTime += TestRGBtoYuvConversionFunction(originalImage, imgProp, pent->d_name, "Standard RGB->YUV", &standardRgbToYuv, resultOutFile);
		/* Shift */
		shiftTime += TestRGBtoYuvConversionFunction(originalImage, imgProp, pent->d_name, "Shift RGB->YUV", &shiftRgbToYuv,  resultOutFile);
		/* Optimized Shift */
		optimizedShiftTime += TestRGBtoYuvConversionFunction(originalImage, imgProp, pent->d_name, "Optimized shift RGB->YUV", &optimizedShiftRgbToYuv,  resultOutFile);

		// ippTime += TestRGBtoYuvConversionFunction(originalImage, imgProp, concat("out\\ippRgb_", pent->d_name), &ippRgbToYuv);

		/*DCT*/
		toJPEG(originalImage, imgProp, resultOutFile);

		/* Free allocated space */
		free(originalImage);
	}

	clock_t end = clock();
	fprintf(resultOutFile, "Standard Conversion Duration: %i (ms) \n", standardTime);
	fprintf(resultOutFile, "Shift Conversion Duration: %i (ms) \n", shiftTime);
	fprintf(resultOutFile, "Optimized shift Conversion Duration: %i (ms) \n", optimizedShiftTime);

	printf("Standard conversion: %i (ms) \n ", standardTime );
	printf("Shift conversion: %i (ms) \n ", shiftTime );
	printf("Optimized shift conversion: %i (ms) \n ", optimizedShiftTime );
	// printf("IPP conversion: %f\n", ippTime / CLOCKS_PER_SEC);
	printf("Total time: %i (ms) \n", end - start );
	fclose(resultOutFile);

	return 0;
}