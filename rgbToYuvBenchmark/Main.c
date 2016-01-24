#include "Main.h"

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

long TestRGBtoYuvConversionFunction(PixelRGB *rgbImage, ImageProperties imgProp, char* imageFilName, char* conversionFunctionName, conversionFunction conversionFunc, FILE *outputFile)
{
	fprintf(outputFile, "File: %s \t Conversion Function: %s \n", imageFilName, conversionFunctionName);
	clock_t startTime, endTime, timeDiff;
	startTime = clock();
	PixelYUV_FP *yuvImage = conversionFunc(rgbImage, imgProp);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(outputFile, "Conversion Duration: %i (ms) \n", (int)timeDiff);
	PixelRGB* convertedBack = yuvToRgb(yuvImage, imgProp);
	ComparePictures(rgbImage, imgProp, convertedBack, imgProp, outputFile);
	free(yuvImage);
	free(convertedBack);
	return timeDiff;
}

#define ANALYSIS_FILE "Results.txt"
//Returns conversion Time
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
	fopen_s(&resultOutFile, ANALYSIS_FILE, "w");

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
			

		//TestDCTFunctions(originalImage, imgProp, resultOutFile);

		/**Convert from RGB to yuv */
		/* Standard */
		standardTime += TestRGBtoYuvConversionFunction(originalImage, imgProp, pent->d_name, "Standard RGB->YUV", &standardRgbToYuv, resultOutFile);
		/* Shift */
		shiftTime += TestRGBtoYuvConversionFunction(originalImage, imgProp, pent->d_name, "Shift RGB->YUV", &shiftRgbToYuv,  resultOutFile);
		/* Optimized Shift */
		optimizedShiftTime += TestRGBtoYuvConversionFunction(originalImage, imgProp, pent->d_name, "Optimized shift RGB->YUV", &optimizedShiftRgbToYuv,  resultOutFile);

		// ippTime += TestRGBtoYuvConversionFunction(originalImage, imgProp, concat("out\\ippRgb_", pent->d_name), &ippRgbToYuv);

		/*DCT*/

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