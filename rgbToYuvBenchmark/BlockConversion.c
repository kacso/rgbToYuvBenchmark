#include "main.h"

block_struct* convert_pixel_array_to_blocks(PixelYUV *yuvImg, ImageProperties *imgProp)
{
	int block_rows = imgProp->Height / DCT_BLOCK_DIM;
	int block_columns = imgProp->Width / DCT_BLOCK_DIM;
	int num_of_blocks = block_rows * block_columns;
	block_struct *blocks = malloc(sizeof(block_struct));
	blocks->numberOfBlocks = num_of_blocks;
	blocks->U_blocks = malloc(num_of_blocks*sizeof(Image_block));
	blocks->V_blocks = malloc(num_of_blocks*sizeof(Image_block));
	blocks->Y_blocks = malloc(num_of_blocks*sizeof(Image_block));
	blocks->numberOfColumns = block_columns;
	blocks->numberOfRows = block_rows;
	for (int b_row = 0; b_row < block_rows; b_row++) {
		for (int b_col = 0; b_col < block_columns; b_col++) {
			int current_block_num = b_row * block_columns + b_col;
			int blockIndex = 0;
			int col_offset = b_col * DCT_BLOCK_DIM;
			int row_offset = b_row * DCT_BLOCK_DIM;
			for (int i = 0; i < DCT_BLOCK_DIM; i++)
			{
				int br = row_offset + i;
				for (int j = 0; j < DCT_BLOCK_DIM; j++)
				{
					int col = col_offset + j;
					int index = br*imgProp->Width + col;
					blocks->U_blocks[current_block_num][blockIndex] = yuvImg[index].U;
					blocks->V_blocks[current_block_num][blockIndex] = yuvImg[index].V;
					blocks->Y_blocks[current_block_num][blockIndex] = yuvImg[index].Y;
					blockIndex++;
				}
			}
		}
	}
	return blocks;
}

PixelYUV* convert_block_to_pixel_array(block_struct *blocks)
{
	PixelYUV *yuvImage = malloc(blocks->numberOfBlocks * DCT_BLOCK_SIZE * sizeof(PixelYUV));
	for (int bl_num = 0; bl_num < blocks->numberOfBlocks; bl_num++)
	{
		int col_offset = (bl_num % blocks->numberOfColumns)*DCT_BLOCK_DIM;
		int row_offset = (bl_num / blocks->numberOfColumns)*DCT_BLOCK_DIM;
		for (int i = 0; i < DCT_BLOCK_SIZE; i++)
		{
			int row = i / DCT_BLOCK_DIM;
			int col = i % DCT_BLOCK_DIM;
			int loc = (row_offset + row)* blocks->numberOfColumns*DCT_BLOCK_DIM + col_offset + col;
			yuvImage[loc].U = blocks->U_blocks[bl_num][i];
			yuvImage[loc].V = blocks->V_blocks[bl_num][i];
			yuvImage[loc].Y = blocks->Y_blocks[bl_num][i];
		}
	}
	return yuvImage;
}


