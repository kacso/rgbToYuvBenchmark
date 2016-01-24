#include "Main.h"

void translateValuesOfBlock(BlockRow rowOfBlocks, int numOfBlocks, int correction) {
	for (int bl_num = 0; bl_num < numOfBlocks; bl_num++)
	{
		for (uint8_t i = 0; i < DCT_BLOCK_SIZE; i++)
		{
			rowOfBlocks[bl_num][i] += correction;
		}		
	}
}

void dct1_block(Image_block oldBlock, Image_block newBlock)
{
	float Cu, Cv;
	for (uint8_t u = 0; u < DCT_BLOCK_DIM; u++)
	{
		for (uint8_t v = 0; v < DCT_BLOCK_DIM; v++)
		{
			if (u == 0) Cu = 1.0f / sqrt(2.0); else Cu = 1.0f;
			if (v == 0) Cv = 1.0f / sqrt(2.0); else Cv = 1.0f;

			float z = 0;

			for (int y = 0; y < DCT_BLOCK_DIM; y++)
			{
				for (int x = 0; x < DCT_BLOCK_DIM; x++)
				{
					z += oldBlock[x*DCT_BLOCK_DIM + y] *
						cos((float)(2 * x + 1) * (float)u * PI / 16.0) *
						cos((float)(2 * y + 1) * (float)v * PI / 16.0);
				}
			}
			newBlock[v * DCT_BLOCK_DIM + u] = 0.25f * Cu * Cv * z;
		}
	}
}

block_struct* dct_generic(block_struct* blocks, transform_block block_transform_func)
{
	block_struct *newBlocks = malloc(sizeof(block_struct));
	newBlocks->numberOfBlocks = blocks->numberOfBlocks;
	newBlocks->U_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->V_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->Y_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->numberOfColumns = blocks->numberOfColumns;
	newBlocks->numberOfRows = blocks->numberOfRows;

	translateValuesOfBlock(blocks->U_blocks, blocks->numberOfBlocks, -128);
	for (size_t bl_num = 0; bl_num < blocks->numberOfBlocks; bl_num++)	
		block_transform_func(blocks->U_blocks[bl_num], newBlocks->U_blocks[bl_num]);

	translateValuesOfBlock(blocks->V_blocks, blocks->numberOfBlocks, -128);
	for (size_t bl_num = 0; bl_num < blocks->numberOfBlocks; bl_num++)
		block_transform_func(blocks->V_blocks[bl_num], newBlocks->V_blocks[bl_num]);

	for (size_t bl_num = 0; bl_num < blocks->numberOfBlocks; bl_num++)
		block_transform_func(blocks->Y_blocks[bl_num], newBlocks->Y_blocks[bl_num]);
	translateValuesOfBlock(blocks->Y_blocks, blocks->numberOfBlocks, -128);

	return newBlocks;
}

block_struct* dct_1(block_struct* blocks)
{
	return dct_generic(blocks, &dct1_block);
}

void idct1_block(Image_block oldBlock, Image_block newBlock)
{
	float Cu, Cv;

	for (int y = 0; y < DCT_BLOCK_DIM; y++)
	{
		for (int x = 0; x < DCT_BLOCK_DIM; x++)
		{
			float z = 0;
			for (uint8_t u = 0; u < DCT_BLOCK_DIM; u++)
			{
				for (uint8_t v = 0; v < DCT_BLOCK_DIM; v++)
				{
					if (u == 0) Cu = 1.0f / sqrt(2.0); else Cu = 1.0f;
					if (v == 0) Cv = 1.0f / sqrt(2.0); else Cv = 1.0f;										

					z += Cu * Cv * oldBlock[v*DCT_BLOCK_DIM + u] *
						cos((float)(2 * x + 1) * (float)u * PI / 16.0) *
						cos((float)(2 * y + 1) * (float)v * PI / 16.0);
				}
			}
			z /= 4.0f;
			if (z > 255.0) z = 255.0f;
			if (z < 0) z = 0.0;
			newBlock[x * DCT_BLOCK_DIM + y] = z;
		}
	}		
}

block_struct* idct_1(block_struct* blocks)
{
	block_struct *newBlocks = malloc(sizeof(block_struct));
	newBlocks->numberOfBlocks = blocks->numberOfBlocks;
	newBlocks->U_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->V_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->Y_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->numberOfColumns = blocks->numberOfColumns;
	newBlocks->numberOfRows = blocks->numberOfRows;

	translateValuesOfBlock(blocks->U_blocks, blocks->numberOfBlocks, 128);
	translateValuesOfBlock(blocks->V_blocks, blocks->numberOfBlocks, 128);
	translateValuesOfBlock(blocks->Y_blocks, blocks->numberOfBlocks, 128);

	for (size_t bl_num = 0; bl_num < blocks->numberOfBlocks; bl_num++)
	{
		idct1_block(blocks->U_blocks[bl_num], newBlocks->U_blocks[bl_num]);
		idct1_block(blocks->V_blocks[bl_num], newBlocks->V_blocks[bl_num]);
		idct1_block(blocks->Y_blocks[bl_num], newBlocks->Y_blocks[bl_num]);
	}
	return newBlocks;
}






//void convertBlock_dct_1(PixelYUV *yuvImage, PixelYUV *dctImage, unsigned rowOffset, unsigned columnOffset, ImageProperties imgProp) {
//	int i, j, u, v;
//	double sumY, sumCb, sumCr, coscos, Cu = 1, Cv = 1;
//	double coef = 2 / sqrt(DCT_BLOCK_SIZE);
//
//	for (u = rowOffset; u < (rowOffset + DCT_BLOCK_DIM); u++) {
//		for (v = columnOffset; v < (columnOffset + DCT_BLOCK_DIM); v++) {
//			sumY = sumCb = sumCr = 0;
//
//
//			for (i = rowOffset; i < (rowOffset + DCT_BLOCK_DIM); i++) {
//				for (j = columnOffset; j < (columnOffset + DCT_BLOCK_DIM); j++) {
//					coscos = cos((2.0 * i + 1) * u * PI / (2.0 * DCT_BLOCK_DIM)) *
//						cos((2.0 * j + 1) * v * PI / (2.0 * DCT_BLOCK_DIM));
//					sumY += yuvImage[i * imgProp.Width + j].Y * coscos;
//					sumCb += yuvImage[i * imgProp.Width + j].U * coscos;
//					sumCr += yuvImage[i * imgProp.Width + j].V * coscos;
//				}
//			}
//
//			if (u == 0 && v == 0) {
//				Cu = sqrt(0.5);
//				Cv = sqrt(0.5);
//			}
//
//			dctImage[u * imgProp.Width + v].Y = coef * Cu * Cv * sumY;
//			dctImage[u * imgProp.Width + v].U = coef * Cu * Cv * sumCb;
//			dctImage[u * imgProp.Width + v].V = coef * Cu * Cv * sumCr;
//		}
//	}
//}
