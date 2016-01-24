#include "Main.h"

uint8_t k1_table[DCT_BLOCK_SIZE] = {
	16, 11, 10, 16, 24, 40, 51, 61 ,
	12, 12, 14, 19, 26, 58, 60, 55 ,
	14, 13, 16, 24, 40, 57, 69, 56 ,
	14, 17, 22, 29, 51, 87, 80, 62 ,
	18, 22, 37, 56, 68, 109, 103, 77 ,
	24, 35, 55, 64, 81, 104, 113, 92 ,
	49, 64, 78, 87, 103, 121, 120, 101 ,
	72, 92, 95, 98, 112, 100, 103, 99  };

uint8_t k2_table[DCT_BLOCK_SIZE] = {
	17, 18, 24, 47, 99, 99, 99, 99 ,
	18, 21, 26, 66, 99, 99, 99, 99 ,
	24, 26, 56, 99, 99, 99, 99, 99 ,
	47, 66, 99, 99, 99, 99, 99, 99 ,
	99, 99, 99, 99, 99, 99, 99, 99 ,
	99, 99, 99, 99, 99, 99, 99, 99 ,
	99, 99, 99, 99, 99, 99, 99, 99 ,
	99, 99, 99, 99, 99, 99, 99, 99  };

block_struct* quantization(block_struct* blocks)
{
	block_struct *newBlocks = malloc(sizeof(block_struct));
	newBlocks->numberOfBlocks = blocks->numberOfBlocks;
	newBlocks->U_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->V_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->Y_blocks = malloc(newBlocks->numberOfBlocks *sizeof(Image_block));
	newBlocks->numberOfColumns = blocks->numberOfColumns;
	newBlocks->numberOfRows = blocks->numberOfRows;
	for (size_t bl_num = 0; bl_num < blocks->numberOfBlocks; bl_num++)
	{
		for (int i = 0; i < DCT_BLOCK_SIZE; i++)
		{
			newBlocks->Y_blocks[bl_num][i] = blocks->Y_blocks[bl_num][i] / k1_table[i];
		}
		for (int i = 0; i < DCT_BLOCK_SIZE; i++)
		{
			newBlocks->V_blocks[bl_num][i] = blocks->V_blocks[bl_num][i] / k2_table[i];
		}
		for (int i = 0; i < DCT_BLOCK_SIZE; i++)
		{
			newBlocks->U_blocks[bl_num][i] = blocks->U_blocks[bl_num][i] / k2_table[i];
		}
		return newBlocks;
	}