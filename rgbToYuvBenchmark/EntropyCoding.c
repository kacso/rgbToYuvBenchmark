#include "Main.h"

static const int zigzag_block_order[8][8] = {
	{ 0,  1,  5,  6, 14, 15, 27, 28 },
	{ 2,  4,  7, 13, 16, 26, 29, 42 },
	{ 3,  8, 12, 17, 25, 30, 41, 43 },
	{ 9, 11, 18, 24, 31, 40, 44, 53 },
	{ 10, 19, 23, 32, 39, 45, 52, 54 },
	{ 20, 22, 33, 38, 46, 51, 55, 60 },
	{ 21, 34, 37, 47, 50, 56, 59, 61 },
	{ 35, 36, 48, 49, 57, 58, 62, 63 }
};
#define MAX_COEF_BITS 10 /* The legal range of a DCT coefficient is -1024 .. +1023  for 8-bit data; */
#define OUTPUT_BUF_SIZE 32768  /* 4096	*/
typedef struct {
	int32_t code_buffer;					/* current bit-accumulation buffer */
	int32_t bits_in_code_buffer;			/* # of bits now in it */

	int8_t * output_buffer;
	int8_t * buffer_next_byte_position;	/*  next byte to write in buffer */
	uint16_t free_space_in_output_buffer;	/* # of byte spaces remaining in buffer */

	FILE * output_file;
} working_state;

typedef struct {
	uint32_t dc_freq[256];
	uint32_t ac_freq[256];
} coeficient_frequencies;

typedef struct {
	uint8_t bits[17];		// bits[k] = # of symbols with codes of length k bits; bits[0] is unused 				
	uint8_t huffval[256];	// input values associated with each code word in order of increasing code length
} huffman_specification;

typedef struct {
	uint32_t ehufco[256];  // code word for each symbol 
	int8_t ehufsi[256];		   // length of code for each symbol. If no code has been allocated for a symbol S, ehufsi[S] contains 0
} huffman_code;

coeficient_frequencies* gather_dct_symbol_statiscits(BlockRow imageBlocks, int32_t numberOfBlocks)
{
	coeficient_frequencies *coef = malloc(sizeof(coeficient_frequencies));
	MEMZERO(coef->ac_freq);
	MEMZERO(coef->dc_freq);

	long last_dc_val = 0;
	for (int blkn = 0; blkn < numberOfBlocks; blkn++) {
		int8_t * block = imageBlocks[blkn];
		int coeff_value;
		int zz_position;
		int num_of_bits;
		int runlength;

		//Encode the DC coefficient - explained in section F.1.2.1 
		coeff_value = block[0] - last_dc_val;
		if (coeff_value < 0) coeff_value = -coeff_value;

		num_of_bits = 0;
		while (coeff_value) {
			num_of_bits++;
			coeff_value >>= 1;
		}
		if (num_of_bits > MAX_COEF_BITS + 1) ERREXIT("DCT coefficient out of range");
		coef->dc_freq[num_of_bits]++; 	//Count the Huffman symbol for the number of bits 

										// Encode the AC coefficients - explained in section F.1.2.2 
		runlength = 0;
		for (int k = 1; k <= DCT_BLOCK_SIZE; k++) {
			zz_position = zigzag_block_order[k];
			coeff_value = block[zz_position];
			if (coeff_value == 0) {
				runlength++;
			}
			else {
				while (runlength > 15) { //if run length > 15 add special run-length-16 code (0xF0) 
					coef->ac_freq[0xF0]++;
					runlength -= 16;
				}

				//Find the number of bits needed for the magnitude of the coefficient 
				if (coeff_value < 0)coeff_value = -coeff_value;

				num_of_bits = 1; //there must be at least one 1 bit 
				while ((coeff_value >>= 1))
					num_of_bits++;

				if (num_of_bits > MAX_COEF_BITS) ERREXIT("DCT coefficient out of range");

				coef->ac_freq[(runlength << 4) + num_of_bits]++; // RUNLENGTH 4B + SIZE 4B
				runlength = 0;
			}
		}
		// it there are zero coefficients at the end -> just add end-of-block code
		if (runlength > 0) coef->ac_freq[0]++;
	}
	return coef;
}

// algorithm from section K.2 of the JPEG standard 
huffman_specification* generate_huffman_specification_table(long freq[])
{
#define MAX_CODE_LEN 32		
	uint8_t bits[MAX_CODE_LEN + 1];	// bits[k] = # of symbols with code length k 
	int codesize[257];		// codesize[k] = code length of symbol k 
	int others[257];		// next symbol in current branch of tree 
	int v1, v2;
	int k, i, j;
	long smallest_freq;

	MEMZERO(bits);
	MEMZERO(codesize);

	for (i = 0; i < 257; i++) others[i] = -1; // initialize links to empty 

	freq[256] = 1;	// set the pseudo-symbol 256 					

					// Figure K.1 – Procedure to find Huffman code sizes
	while (true)
	{
		// select the largest value of V with the least value of FREQ(V) greater than zero
		v1 = -1;
		smallest_freq = 1000000000L;
		for (i = 0; i <= 256; i++) {
			if (freq[i] && freq[i] <= smallest_freq) {
				smallest_freq = freq[i];
				v1 = i;
			}
		}

		// Find the next smallest nonzero frequency
		v2 = -1;
		smallest_freq = 1000000000L;
		for (i = 0; i <= 256; i++) {
			if (freq[i] && freq[i] <= smallest_freq && i != v1) {
				smallest_freq = freq[i];
				v2 = i;
			}
		}

		// only one frequency left -> Done
		if (v2 < 0)
			break;

		// merge the two counts
		freq[v1] += freq[v2];
		freq[v2] = 0;

		codesize[v1]++;
		while (others[v1] >= 0) {
			v1 = others[v1];
			codesize[v1]++;
		}
		others[v1] = v2; // chain v2 onto v1's tree  

						 // Increment the codesize of everything in v2's tree 
		codesize[v2]++;
		while (others[v2] >= 0) {
			v2 = others[v2];
			codesize[v2]++;
		}
	}

	// Figure K.2 – Procedure to find the number of codes of each size	
	for (i = 0; i <= 256; i++) {
		if (codesize[i]) {
			if (codesize[i] > MAX_CODE_LEN) ERREXIT("Huffman code size table overflow");
			bits[codesize[i]]++;
		}
	}

	// Figure K.3 – Procedure for limiting code lengths to 16 bits
	for (i = MAX_CODE_LEN; i > 16; i--) {
		while (bits[i] > 0) {
			j = i - 2;
			while (bits[j] == 0)
				j--;

			bits[i] -= 2;
			bits[i - 1]++;
			bits[j + 1] += 2;
			bits[j]--;
		}
	}

	while (bits[i] == 0) //Remove the count for the pseudo-symbol 256 from the largest codelength 
		i--;
	bits[i]--;

	huffman_specification* spec = malloc(sizeof(huffman_specification));
	MEMZERO(spec->huffval);

	memcpy((void *)(spec->bits), (const void *)(bits), (size_t)(sizeof(spec->bits)));

	//Figure K.4 – Sorting of input values according to code size
	k = 0;
	for (i = 1; i <= MAX_CODE_LEN; i++) {
		for (j = 0; j <= 255; j++) {
			if (codesize[j] == i) {
				spec->huffval[k] = (uint8_t)j;
				k++;
			}
		}
	}
	return spec;
}


// Algorithm from section C of the JPEG standard 
// Compute Huffman codes 
huffman_code * generate_huffman_table(huffman_specification *htbl, bool isDC)
{

	int8_t huffsize[257];	 // contains the length of Huffman codes 
	uint32_t huffcode[257];  // contains the Huffman codes

							 // Figure C.1 – Generation of table of Huffman code sizes 	
	int k = 0;
	for (int i = 1; i <= 16; i++) {
		int num_bits = (int)htbl->bits[i];
		for (int j = 1; j <= num_bits; j++)
		{
			huffsize[k] = (char)i;
			k++;
		}
	}
	huffsize[k] = 0;
	int last_k = k;

	// Figure C.2 – Generation of table of Huffman codes
	uint32_t code = 0;
	int si = huffsize[0];
	k = 0;
	while (huffsize[k]) {
		while (((int)huffsize[k]) == si) {
			huffcode[k] = code;
			k++;
			code++;
		}
		code <<= 1;
		si++;
	}

	huffman_code *code_tables;
	code_tables = (huffman_code *)malloc(sizeof(huffman_code));
	MEMZERO(code_tables->ehufsi);
	MEMZERO(code_tables->ehufco);

	// Figure C.3 – Ordering procedure for encoding procedure code tables
	// additional check for duplicates or out of range codes
	int maxsymbol = isDC ? 15 : 255;
	for (k = 0; k < last_k; k++) {
		int j = htbl->huffval[k];
		if (j < 0 || j > maxsymbol || code_tables->ehufsi[j]) ERREXIT("Bad Huffman table definition");
		code_tables->ehufco[j] = huffcode[k];
		code_tables->ehufsi[j] = huffsize[k];
	}
	return code_tables;
}

void write_buffer_to_file(working_state * state)
{
	fwrite((const void *)(state->output_buffer), (size_t)1, (size_t)(OUTPUT_BUF_SIZE), (state->output_file));
	state->buffer_next_byte_position = state->output_buffer; // set position at the start of buffer
	state->free_space_in_output_buffer = OUTPUT_BUF_SIZE;
}

void write_symbol_to_buffer(working_state * state, uint32_t code, int32_t size)
{
	int32_t working_buffer;
	int32_t bits_in_working_buffer;
	if (size == 0) 	ERREXIT("Code size is 0. Possible incorrect Huffman table.");

	working_buffer = ((int32_t)code) & ((((int32_t)1) << size) - 1);

	bits_in_working_buffer = size + state->bits_in_code_buffer; // add existing bits from coding buffer 

	working_buffer <<= 24 - bits_in_working_buffer; // shift buffer bits to correct position
	working_buffer |= state->code_buffer;  // add bits from coding buffer

	while (bits_in_working_buffer >= 8) {
		int c = (int)((working_buffer >> 16) & 0xFF);

		*state->buffer_next_byte_position = (int8_t)(c);
		state->buffer_next_byte_position++;
		state->free_space_in_output_buffer--;
		if (state->free_space_in_output_buffer == 0)
		{
			write_buffer_to_file(state);
		}
		if (c == 0xFF) {						// check if byte stuffing is needed: section F.1.2.3 of JPEG standard
			*state->buffer_next_byte_position = (int8_t)(0);
			state->buffer_next_byte_position++;
			state->free_space_in_output_buffer--;
			if (state->free_space_in_output_buffer == 0)
				write_buffer_to_file(state);
		}
		working_buffer <<= 8;
		bits_in_working_buffer -= 8;
	}
	state->code_buffer = working_buffer;
	state->bits_in_code_buffer = bits_in_working_buffer;
}


void encode_one_block(working_state * state, Image_block block, int last_dc_val, huffman_code *dctbl, huffman_code *actbl)
{
	int coeff_value, coeff_coded;
	int num_of_bits;
	int runlength;
	const int * block_order = zigzag_block_order;

	// Encode the AC coefficients - explained in section F.1.2.2 
	coeff_value = coeff_coded = block[0] - last_dc_val;  // coeff_value

	if (coeff_value < 0) {		//for negative coefficient - save bitwise complement of abs(input)
		coeff_value = -coeff_value;
		coeff_coded--;
	}

	num_of_bits = 0;
	while (coeff_value) {
		num_of_bits++;
		coeff_value >>= 1;
	}

	if (num_of_bits > MAX_COEF_BITS + 1)	 ERREXIT("DCT coefficient out of range");

	write_symbol_to_buffer(state, dctbl->ehufco[num_of_bits], dctbl->ehufsi[num_of_bits]);

	if (num_of_bits)			//size != 0 
		write_symbol_to_buffer(state, (unsigned int)coeff_coded, num_of_bits);

	// Encode the AC coefficients - explained in section F.1.2.2 
	runlength = 0;
	for (int k = 1; k <= DCT_BLOCK_SIZE; k++) {
		if ((coeff_coded = block[block_order[k]]) == 0) {
			runlength++;
		}
		else {
			while (runlength > 15) { //if run length > 15 add special run-length-16 code (0xF0) 
				write_symbol_to_buffer(state, actbl->ehufco[0xF0], actbl->ehufsi[0xF0]);
				runlength -= 16;
			}

			coeff_value = coeff_coded;
			if (coeff_value < 0) {
				coeff_value = -coeff_value;
				coeff_coded--;
			}

			num_of_bits = 1;
			while ((coeff_value >>= 1))
				num_of_bits++;

			if (num_of_bits > MAX_COEF_BITS)	ERREXIT("DCT coefficient out of range");

			coeff_value = (runlength << 4) + num_of_bits;
			write_symbol_to_buffer(state, actbl->ehufco[coeff_value], actbl->ehufsi[coeff_value]);

			write_symbol_to_buffer(state, (unsigned int)coeff_coded, num_of_bits);
			runlength = 0;
		}
	}
	// it there are zero coefficients at the end -> just add end-of-block code
	if (runlength > 0) write_symbol_to_buffer(state, actbl->ehufco[0], actbl->ehufsi[0]);
}

void print_huffman_tables(coeficient_frequencies *coef, huffman_specification* ac_spec, huffman_specification* dc_spec, huffman_code *ac_code, huffman_code *dc_code)
{
	FILE *outFile;
	fopen_s(&outFile, "HuffmanCode.csv", "w");
	// DC: Symbol: i, Frequency:  coef->dc_freq, ac_spec->bits, ac_spec->hufval , ac_code->ehufsi ac_code->ehufco
	fprintf(outFile, "Symbol\t Freq\t Bits\t hufval\t ehufco\t ehufsi\n");
	for (uint8_t i = 0; i < 256; i++)
	{
		fprintf(outFile, "%X\t %u\t %u\t %u\t %u\t %i\n", i, coef->dc_freq[i], dc_spec->bits[i], dc_spec->huffval[i], dc_code->ehufsi[i], dc_code->ehufco[i]);
	}
	fclose(outFile);
}

void initialaze_state(working_state *state, FILE * outFile)
{
	state->code_buffer = 0;
	state->bits_in_code_buffer = 0;

	state->output_buffer = malloc(OUTPUT_BUF_SIZE*sizeof(int8_t));
	state->buffer_next_byte_position = state->output_buffer;
	state->free_space_in_output_buffer = OUTPUT_BUF_SIZE;

	state->output_file = outFile;
}

void encode_picture(block_struct *imageBlocks, ImageProperties *imgProp, FILE *outFile)
{	
	//repeat for Y,U,V :
	BlockRow current_blocks = imageBlocks->U_blocks;

	coeficient_frequencies *coef = gather_dct_symbol_statiscits(current_blocks, imageBlocks->numberOfBlocks);
	huffman_specification *ac_spec = generate_huffman_specification_table(coef->ac_freq);
	huffman_specification *dc_spec = generate_huffman_specification_table(coef->ac_freq);
	huffman_code *ac_huff_code = generate_huffman_table(ac_spec, false);
	huffman_code *dc_huff_code = generate_huffman_table(dc_spec, false);

	print_huffman_tables(coef, ac_spec, dc_spec, ac_huff_code, dc_huff_code);

	working_state state;
	initialaze_state(&state, outFile);
	//ENCODE
	int last_dc_value = 0;
	for (int blkn = 0; blkn < imageBlocks->numberOfBlocks; blkn++) {
		encode_one_block(&state, current_blocks[blkn], last_dc_value, dc_huff_code, ac_huff_code);
		last_dc_value = current_blocks[blkn][0];
	}
}
