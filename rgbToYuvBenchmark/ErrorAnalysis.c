#include "Main.h"

void ComparePictures(PixelRGB* img1, ImageProperties* prop_img1, PixelRGB* img2, ImageProperties* prop_img2, FILE* outFile)
{
	unsigned int numberOfPixels = prop_img1->Height * prop_img1->Width;
	unsigned diffR, diffG, diffB, diffTotal, maxDiff = 0;
	unsigned int squaredError = 0;
	int differenceCount[256 * 2 * 3]; // [pixel difference]
	MEMZERO(differenceCount);
			
	//Start comparison
	for (unsigned i = 0; i < numberOfPixels; i++) { // i = current pixel
		diffR = abs(img1[i].R - img2[i].R);
		diffG = abs(img1[i].G - img2[i].G);
		diffB = abs(img1[i].B - img2[i].B);
		diffTotal = diffR + diffG + diffB;		
		(differenceCount[diffTotal])++;
		if (diffTotal > 0) squaredError += (diffTotal * diffTotal);
		if (diffTotal > maxDiff) maxDiff = diffTotal;
	}
	double meanSquaredError = (squaredError / numberOfPixels);
	fprintf(outFile, "Mean squared error: %.2f \n", meanSquaredError);

	/*fprintf(outFile, "PixelDifference\t  \tNumberOfPixels \t(percent) \n");
	for (unsigned i = 0; i <= maxDiff; i++)
	{
		fprintf(outFile, "%d \t %6u \t%f \n",
			i,
			differenceCount[i],
			((float)(100 * differenceCount[i]) / numberOfPixels));
	}*/
}