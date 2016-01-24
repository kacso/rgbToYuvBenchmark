#include "Main.h"

//1D DCT over every row and then every column
void dct_1d(double *in, double *out, const int count)
{
	for (int u = 0; u < count; u++)
	{
		double z = 0;
		for (int x = 0; x < count; x++)
		{
			z += in[x] * cos(PI * (double)u * (double)(2 * x + 1)/ (double)(2 * count));
		}
		if (u == 0) z *= 1.0 / sqrt(2.0);
		out[u] = z / 2.0;
	}
}

void dct2_block(Image_block oldBlock, Image_block newblock)
{
	int i, j;
	double in[8], out[8], rows[8][8];	
	/* transform rows */
	for (int j = 0; j<8; j++)
	{
		for (int i = 0; i < 8; i++) //load row
			in[i] = (double) oldBlock[j*8+i];  

		dct_1d(in, out, 8); //transform row
		for (int i = 0; i<8; i++) rows[j][i] = out[i];// save row to intermediate block
	}
	/* transform columns */
	for (j = 0; j<8; j++)
	{
		for (i = 0; i<8; i++)
			in[i] = rows[i][j];
		dct_1d(in, out, 8);
		for (i = 0; i < 8; i++) newblock[j * 8 + i]; 
	}
}

block_struct* dct_2(block_struct* blocks)
{
	return dct_generic(blocks, &dct2_block);
}