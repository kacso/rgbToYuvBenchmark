#include "Main.h"

//Fast DCT algorithm due to Arai, Agui, Nakajima 

void dct3_block(Image_block oldBlock, Image_block newBlock)
{
	int i;
	int rows[8][8];
	static const int	c1 = 1004 /* cos(pi/16) << 10 */,
						s1 = 200 /* sin(pi/16) */,
						c3 = 851 /* cos(3pi/16) << 10 */,
						s3 = 569 /* sin(3pi/16) << 10 */,
						r2c6 = 554 /* sqrt(2)*cos(6pi/16) << 10 */,
						r2s6 = 1337 /* sqrt(2)*sin(6pi/16) << 10 */,
						r2 = 181; /* sqrt(2) << 7*/
	uint8_t x0, x1, x2, x3, x4, x5, x6, x7, x8;
	/* transform rows */
	for (i = 0; i<8; i++)
	{
		x0 = oldBlock[i * 8]; 
		x1 = oldBlock[i * 8 + 1];
		x2 = oldBlock[i * 8 + 2];
		x3 = oldBlock[i * 8 + 3];
		x4 = oldBlock[i * 8 + 4];
		x5 = oldBlock[i * 8 + 5];
		x6 = oldBlock[i * 8 + 6];
		x7 = oldBlock[i * 8 + 7];

		/* Stage 1 */
		x8 = x7 + x0;
		x0 -= x7;
		x7 = x1 + x6;
		x1 -= x6;
		x6 = x2 + x5;
		x2 -= x5;
		x5 = x3 + x4;
		x3 -= x4;

		/* Stage 2 */
		x4 = x8 + x5;
		x8 -= x5;
		x5 = x7 + x6;
		x7 -= x6;
		x6 = c1*(x1 + x2);
		x2 = (-s1 - c1)*x2 + x6;
		x1 = (s1 - c1)*x1 + x6;
		x6 = c3*(x0 + x3);
		x3 = (-s3 - c3)*x3 + x6;
		x0 = (s3 - c3)*x0 + x6;

		/* Stage 3 */
		x6 = x4 + x5;
		x4 -= x5;
		x5 = r2c6*(x7 + x8);
		x7 = (-r2s6 - r2c6)*x7 + x5;
		x8 = (r2s6 - r2c6)*x8 + x5;
		x5 = x0 + x2;
		x0 -= x2;
		x2 = x3 + x1;
		x3 -= x1;

		/* Stage 4 and output */
		rows[i][0] = x6;
		rows[i][4] = x4;
		rows[i][2] = x8 >> 10;
		rows[i][6] = x7 >> 10;
		rows[i][7] = (x2 - x5) >> 10;
		rows[i][1] = (x2 + x5) >> 10;
		rows[i][3] = (x3*r2) >> 17;
		rows[i][5] = (x0*r2) >> 17;
	}

	/* transform columns */
	for (i = 0; i<8; i++)
	{
		x0 = rows[0][i];
		x1 = rows[1][i];
		x2 = rows[2][i];
		x3 = rows[3][i];
		x4 = rows[4][i];
		x5 = rows[5][i];
		x6 = rows[6][i];
		x7 = rows[7][i];

		/* Stage 1 */
		x8 = x7 + x0;
		x0 -= x7;
		x7 = x1 + x6;
		x1 -= x6;
		x6 = x2 + x5;
		x2 -= x5;
		x5 = x3 + x4;
		x3 -= x4;

		/* Stage 2 */
		x4 = x8 + x5;
		x8 -= x5;
		x5 = x7 + x6;
		x7 -= x6;
		x6 = c1*(x1 + x2);
		x2 = (-s1 - c1)*x2 + x6;
		x1 = (s1 - c1)*x1 + x6;
		x6 = c3*(x0 + x3);
		x3 = (-s3 - c3)*x3 + x6;
		x0 = (s3 - c3)*x0 + x6;

		/* Stage 3 */
		x6 = x4 + x5;
		x4 -= x5;
		x5 = r2c6*(x7 + x8);
		x7 = (-r2s6 - r2c6)*x7 + x5;
		x8 = (r2s6 - r2c6)*x8 + x5;
		x5 = x0 + x2;
		x0 -= x2;
		x2 = x3 + x1;
		x3 -= x1;

		/* Stage 4 and output */		
		newBlock[i] = (double)((x6 + 16) >> 3);
		newBlock[i + 8 * 1] = (double)((x2 + x5 + 16384) >> 13);
		newBlock[i + 8 * 2] = (double)((x8 + 16384) >> 13);
		newBlock[i + 8 * 3] = (double)(((x3 >> 8)*r2 + 8192) >> 12);
		newBlock[i + 8 * 4] = (double)((x4 + 16) >> 3);
		newBlock[i + 8 * 5] = (double)(((x0 >> 8)*r2 + 8192) >> 12);
		newBlock[i + 8 * 6] = (double)((x7 + 16384) >> 13);
		newBlock[i + 8 * 7] = (double)((x2 - x5 + 16384) >> 13);
	}
}

block_struct* dct_3(block_struct* blocks)
{
	return dct_generic(blocks, &dct3_block);
}