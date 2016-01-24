#include "Main.h"

void TestDCTFunctions(PixelRGB *orgImage, ImageProperties imgProp, FILE *analysisResultsFile) {
	clock_t startTime, endTime, timeDiff;
	PixelYUV_FP *yuvImage = shiftRgbToYuv(orgImage, imgProp);
	PixelRGB* YUV_to_RGB = yuvToRgb(yuvImage, imgProp);
	fprintf(analysisResultsFile, "Comparison: Original RGB -> YUV ->RGB2 \n");
	ComparePictures(orgImage, imgProp, YUV_to_RGB, imgProp, analysisResultsFile);
	block_struct* blocks = convert_pixel_array_to_blocks(yuvImage, imgProp);

	//DCT
	startTime = clock();
	block_struct* dctImg = dct_1(blocks);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(analysisResultsFile, "DCT conversion duration: %i (ms) \n", (int)timeDiff);

	//IDCT
	startTime = clock();
	block_struct* idctImg = idct_1(dctImg);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(analysisResultsFile, "IDCT conversion duration: %i (ms) \n", (int)timeDiff);

	PixelYUV_FP* backFromBlocks = convert_block_to_pixel_array(idctImg);
	PixelRGB* rgbFromIdct = yuvToRgb(yuvImage, imgProp);

	fprintf(analysisResultsFile, "Comparison: YUV converted with IDCT \n");
	ComparePictures(YUV_to_RGB, imgProp, rgbFromIdct, imgProp, analysisResultsFile);

	fprintf(analysisResultsFile, "Comparison: Original -> DCT -> IDCT -> RGB \n");
	ComparePictures(orgImage, imgProp, rgbFromIdct, imgProp, analysisResultsFile);


	//DCT2
	startTime = clock();
	block_struct* dct2Img = dct_2(blocks);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(analysisResultsFile, "DCT2 conversion duration: %i (ms) \n", (int)timeDiff);

	//IDCT (DCT2)
	block_struct* idct2Img = idct_1(dctImg);
	PixelYUV_FP* idct2_yuv = convert_block_to_pixel_array(idct2Img); //
	PixelRGB* idct2_rgb = yuvToRgb(idct2_yuv, imgProp);

	fprintf(analysisResultsFile, "Comparison: YUV converted with IDCT (DCT2) \n");
	ComparePictures(YUV_to_RGB, imgProp, idct2_rgb, imgProp, analysisResultsFile);

	fprintf(analysisResultsFile, "Comparison: Original -> DCT2 -> IDCT  -> RGB \n");
	ComparePictures(orgImage, imgProp, idct2_rgb, imgProp, analysisResultsFile);

	//DCT3
	startTime = clock();
	block_struct* dct3Img = dct_3(blocks);
	endTime = clock();
	timeDiff = endTime - startTime;
	fprintf(analysisResultsFile, "DCT3 conversion duration: %i (ms) \n", (int)timeDiff);
	// IDCT(DCT3)
	block_struct* idct3Img = idct_1(dctImg);
	PixelYUV_FP* idct3_yuv = convert_block_to_pixel_array(idct2Img);
	PixelRGB* idct3_rgb = yuvToRgb(idct2_yuv, imgProp);

	fprintf(analysisResultsFile, "Comparison: YUV converted with IDCT (DCT3) \n");
	ComparePictures(YUV_to_RGB, imgProp, idct3_rgb, imgProp, analysisResultsFile);

	fprintf(analysisResultsFile, "Comparison: Original -> DCT3 -> IDCT  -> RGB \n");
	ComparePictures(orgImage, imgProp, idct3_rgb, imgProp, analysisResultsFile);

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
