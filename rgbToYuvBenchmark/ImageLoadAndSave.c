#include "Main.h"

ImageProperties readImageProperties(FILE *imgFile) {
	char readedLine[10];
	ImageProperties imgProp;
	fseek(imgFile, 0L, SEEK_SET);

	/**Read type of file*/
	if (fscanf_s(imgFile, "%s", readedLine, 10) == EOF) {
		perror("EOF");
		exit(-2);
	}
	/**Check if type is P6*/
	if (!(readedLine[0] == 'P' && readedLine[1] == '6' && strlen(readedLine) == 2)) {
		perror("Wrong file type.");
		exit(-4);
	}

	/**Read image width, height and maximum color value*/
	do {
		if (fscanf_s(imgFile, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	imgProp.Width = (unsigned)atoi(readedLine);

	do {
		if (fscanf_s(imgFile, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	imgProp.Height = (unsigned)atoi(readedLine);

	do {
		if (fscanf_s(imgFile, "%s", readedLine, 10) == EOF) {
			perror("EOF");
			exit(-3);
		}
	} while (readedLine[0] == '#');
	imgProp.maxColorValue = (unsigned)atoi(readedLine);

	/**Read \n*/
	fscanf_s(imgFile, "%c", readedLine, 1);

	//fscanf_s(img, "%c", readedLine, 1);
	imgProp.propertiesLength = ftell(imgFile);
	return imgProp;
}

PixelRGB *loadRGBImage(FILE *imgFile, ImageProperties imgProp)
{
	PixelRGB *image = malloc(imgProp.Height * imgProp.Width * sizeof(PixelRGB));
	if (image == NULL) perror("Malloc error");
	fseek(imgFile, imgProp.propertiesLength, SEEK_SET);
	fread(image, sizeof(PixelRGB), imgProp.Height * imgProp.Width, imgFile);
	return image;
}

void saveHeaderOfppm(char* fileName, ImageProperties imgProp)
{
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "wb") != 0) {
		printf("Error while opening output file.");
		exit(-2);
	}

	fprintf(output, "P6\n%d %d\n%d\n", imgProp.Width, imgProp.Height, imgProp.maxColorValue);

	fclose(output);
}

void saveImgAsppm(char* fileName, PixelRGB *blocks, ImageProperties imgProp)
{
	//struct rgbPixel *readedLine = malloc(rowWidth * sizeof(struct rgbPixel));
	unsigned i, j;
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "wb") != 0) {
		printf("Error while opening output file.");
		exit(-2);
	}

	fprintf(output, "P6\n%d %d\n%d\n", imgProp.Width, imgProp.Height, imgProp.maxColorValue);


	fwrite(blocks, sizeof(PixelRGB), imgProp.Height * imgProp.Width, output);

	/*for (i = 0; i < imgProp.Height; ++i) {
	for (j = 0; j < imgProp.Width; ++j) {
	fwrite(&blocks[i * imgProp.Width + j].R, sizeof(uint8_t), 1, output);
	fwrite(&blocks[i * imgProp.Width + j].G, sizeof(uint8_t), 1, output);
	fwrite(&blocks[i * imgProp.Width + j].B, sizeof(uint8_t), 1, output);
	}
	}*/
	fclose(output);
}

void saveBlocksToppm(char* fileName, PixelRGB **blocks, unsigned imageWidth)
{
	//struct rgbPixel *readedLine = malloc(rowWidth * sizeof(struct rgbPixel));
	unsigned i, j, k;
	FILE *output;

	/**Open an output file*/
	if (fopen_s(&output, fileName, "a") != 0 || output == NULL) {
		perror("Error while opening output file.");
		exit(-2);
	}

	fseek(output, 0, SEEK_END);

	for (i = 0; i < DCT_BLOCK_DIM; ++i) {
		for (j = 0; j < imageWidth / DCT_BLOCK_DIM; ++j) {//do broja blokova u retku
			for (k = 0; k < DCT_BLOCK_DIM; ++k) {
				fwrite(&blocks[j][i * DCT_BLOCK_DIM + k].R, sizeof(uint8_t), 1, output);
				fwrite(&blocks[j][i * DCT_BLOCK_DIM + k].G, sizeof(uint8_t), 1, output);
				fwrite(&blocks[j][i * DCT_BLOCK_DIM + k].B, sizeof(uint8_t), 1, output);
			}
		}
		//fprintf(output, "\n");
		/*fread(readedLine, sizeof(struct rgbPixel), rowWidth, img);
		for (j = 0; j < BLOCK_WIDTH; j++) {
		readedBlock[i * BLOCK_WIDTH + j] = readedLine[columnOffset + j];
		}*/
	}
	fclose(output);
}