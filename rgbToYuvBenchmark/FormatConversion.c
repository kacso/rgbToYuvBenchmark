#include "Main.h"

/**Converts from rgb to yuv
Values are returned through variables Y, Cb and Cr */
PixelYUV_FP * standardRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp) {
	PixelYUV_FP *yuvImage = malloc(imgProp.Height * imgProp.Width * sizeof(PixelYUV_FP));
	for (uint32_t index = 0; index < imgProp.Height * imgProp.Width; index++) {
			yuvImage[index].Y = 
				0.257f * rgbImage[index].R +
				0.504f * rgbImage[index].G + 
				0.098f * rgbImage[index].B + 16;
			yuvImage[index].U = 
				-0.148f * rgbImage[index].R -
				0.291f * rgbImage[index].G +
				0.439f * rgbImage[index].B + 128;
			yuvImage[index].V = 
				0.439f * rgbImage[index].R -
				0.368f * rgbImage[index].G -
				0.071f * rgbImage[index].B + 128;		
	}
	return yuvImage;
}

PixelYUV_FP * shiftRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp) {
	PixelYUV_FP *yuvImage = malloc(imgProp.Height * imgProp.Width * sizeof(PixelYUV_FP));

	for (uint32_t index = 0; index < imgProp.Height * imgProp.Width; index++) {
			yuvImage[index].Y = ((66 * rgbImage[index].R +
				129 * rgbImage[index].G + 25 * rgbImage[index].B + 128) >> 8) + 16;
			yuvImage[index].U = ((-38 * rgbImage[index].R -
				74 * rgbImage[index].G + 112 * rgbImage[index].B + 128) >> 8) + 128;
			yuvImage[index].V = ((112 * rgbImage[index].R -
				94 * rgbImage[index].G - 18 * rgbImage[index].B + 128) >> 8) + 128;		
	}
	return yuvImage;
}

PixelYUV_FP * optimizedShiftRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp) {
	PixelYUV_FP *yuvImage = malloc(imgProp.Height * imgProp.Width * sizeof(PixelYUV_FP));
	for (uint32_t index = 0; index < imgProp.Height * imgProp.Width; index++) {
			yuvImage[index].Y = ((66 * rgbImage[index].R +
				4 * 32 * rgbImage[index].G + 25 * rgbImage[index].B + 128) >> 8) + 16;
			yuvImage[index].U = ((-38 * rgbImage[index].R -
				74 * rgbImage[index].G + 112 * rgbImage[index].B + 128) >> 8) + 128;
			yuvImage[index].V = ((3 * 38 * rgbImage[index].R -
				3 * 32 * rgbImage[index].G - 18 * rgbImage[index].B + 128) >> 8) + 128;		
	}
	return yuvImage;
}

//PixelYUV *ippRgbToYuv(PixelRGB *rgbImage, ImageProperties imgProp) {
//PixelYUV *yuvImage = malloc(imgProp.Height * imgProp.Width * sizeof(PixelYUV));
//	IppiSize size;
//	size.height = imgProp.Height;
//	size.width = imgProp.Width;
//	IppStatus status = ippiRGBToYUV_8u_C3R((Ipp8u*)rgbImage, 1, (Ipp8u*) yuvImage, 1, size);	
//return yuvImage;
//}

/** Converts from yuv to rgb */
PixelRGB* yuvToRgb(PixelYUV_FP *yuvImage, ImageProperties imgProp) {
	PixelRGB *rgbImage = malloc(imgProp.Height * imgProp.Width * sizeof(PixelRGB));

	for (uint32_t index = 0; index < imgProp.Height * imgProp.Width; index++) {
			rgbImage[index].R = 
				1.164f * (yuvImage[index].Y - 16) +
				1.596f * (yuvImage[index].V - 128);

			rgbImage[index].G =
				1.164f * (yuvImage[index].Y - 16) -
				0.813f * (yuvImage[index].V - 128)
				- 0.391f * (yuvImage[index].U - 128);

			rgbImage[index].B = 
				1.164f * (yuvImage[index].Y - 16) +
				2.018f * (yuvImage[index].U - 128);		
	}
	return rgbImage;
}
