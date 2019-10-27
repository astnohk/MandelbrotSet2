#include "pnm.h"


int
fcommentf(FILE *fp, unsigned int *ret)
{
	char ctmp[NUM_READ_STRING + 1];
	char c;
	int c_int;
	int flag;
	unsigned int read;
	long ftold;

	ctmp[NUM_READ_STRING] = '\0'; /* To prevent Buffer Overflow */
	flag = 1;
	while (flag != 0) {
		flag = 0;
		ftold = ftell(fp);
		if (fscanf(fp, STRING_NUM_READ_STRING, ctmp) != 1) {
			fprintf(stderr, "*** fcommentf error - Failed to read by fscanf() ***\n");
			return PNM_FUNCTION_ERROR;
		}
		if (ctmp[0] == '#') {
			if (fseek(fp, ftold, SEEK_SET) != 0) {
				fprintf(stderr, "*** fcommentf error - Failed to do fseek() ***\n");
				return PNM_FUNCTION_ERROR;
			}
			printf("\"");
			while ((flag < 2) && (feof(fp) == 0)) {
				if ((c_int = fgetc(fp)) == EOF) {
					fprintf(stderr, "*** pnmread error - fgetc returns EOF ***\n");
					return PNM_FUNCTION_ERROR;
				}
				c = (char)c_int;
				if (c != '\n') {
					printf("%c", c);
				}
				if ((flag == 0) && (c == '#')) {
					flag = 1;
				} else if ((flag != 0) && (c == '\n')) {
					flag = 2;
				}
			}
			printf("\"\n");
		}
	}
	ctmp[NUM_READ_STRING] = '\0'; /* To prevent Buffer Overflow */
	if (sscanf(ctmp, "%7u", &read) != 1) {
		fprintf(stderr, "*** fcommentf error - Failed to read from ctmp by sscanf() ***\n");
		return PNM_FUNCTION_ERROR;
	}
	*ret = read;
	return PNM_FUNCTION_SUCCESS;
}


int
pnmread(PNM *pnm, char *filename)
{
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";
	char *ErrorFileName = "";
	char *ErrorDescription = "";

	uint8_t *img_uint8 = NULL;
	pnm_img *img = NULL;
	size_t size_t_max = (size_t)(~0u);
	char FileDescriptor[PNM_DESCRIPTOR_LENGTH];
	unsigned int Width = 0u, Height = 0u, MaxInt = 0u;
	unsigned int Width_tmp = 0u;
	unsigned int m, n, byte;
	int test;
	FILE *fp = NULL;

	if (pnm == NULL) {
		ErrorValueName = "pnm";
		goto ErrorPointerNULL;
	} else if (pnm->img != NULL) {
		ErrorValueName = "(pnm->img)";
		goto ErrorPointerNotInitialized;
	}

	if ((fp = fopen(filename, "rb")) == NULL) {
		ErrorFunctionName = "fopen";
		ErrorValueName = "fp";
		ErrorFileName = filename;
		goto ErrorFileOpen;
	}
	if ((fgets(FileDescriptor, PNM_DESCRIPTOR_LENGTH, fp)) == NULL) {
		ErrorFunctionName = "fgets";
		ErrorValueName = "FileDescriptor";
		goto ErrorRead;
	}
	if (FileDescriptor[0] != 'P') {
		ErrorDescription = "The file is not PNM format";
		goto ErrorOthers;
	}

	printf("\n--- Reading from \"%s\" ---\n  FileDescriptor : %s\n", filename, FileDescriptor);
	switch (FileDescriptor[1] - '0') {
		case PORTABLE_BITMAP_ASCII: /* Portable Bitmap ASCII */
			if (fcommentf(fp, &Width) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &Height) != PNM_FUNCTION_SUCCESS) {
				ErrorFunctionName = "fcommentf";
				ErrorValueName = "Width and Height";
				goto ErrorRead;
			}
			if ((double)size_t_max < (double)Width * (double)Height) {
				goto ErrorImageSize;
			}
			MaxInt = 1;
			if (fgetc(fp) == EOF) { /* Pass through '\n' */
				ErrorFunctionName = "fgetc";
				ErrorValueName = "\\n";
				goto ErrorRead;
			}
			if ((img = (pnm_img *)calloc((size_t)(Width * Height), sizeof(int))) == 0) {
				ErrorFunctionName = "calloc";
				ErrorValueName = "img";
				goto ErrorMalloc;
			}
			test = 0;
			for (m = 0; m < Width * Height; m++) {
				test |= fscanf(fp, "%1d", &(img[m]));
				img[m] = 1 - img[m];
			}
			if (test != 1) {
				ErrorFunctionName = "fscanf";
				ErrorValueName = "img";
				goto ErrorRead;
			}
			break;
		case PORTABLE_GRAYMAP_ASCII: /* Portable Graymap ASCII */
			if (fcommentf(fp, &Width) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &Height) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &MaxInt) != PNM_FUNCTION_SUCCESS) {
				ErrorFunctionName = "fcommentf";
				ErrorValueName = "Width, Height and MaxInt";
				goto ErrorRead;
			}
			if ((double)size_t_max < (double)Width * (double)Height) {
				goto ErrorImageSize;
			}
			if (fgetc(fp) == EOF) { /* Pass through '\n' */
				ErrorFunctionName = "fgetc";
				ErrorValueName = "\\n";
				goto ErrorRead;
			}
			if ((img = (pnm_img *)calloc((size_t)(Width * Height), sizeof(int))) == 0) {
				ErrorFunctionName = "calloc";
				ErrorValueName = "img";
				goto ErrorMalloc;
			}
			test = 0;
			for (m = 0; m < Width * Height; m++) {
				test |= fscanf(fp, "%7d", &(img[m]));
			}
			if (test != 1) {
				ErrorFunctionName = "fscanf";
				ErrorValueName = "img";
				goto ErrorRead;
			}
			break;
		case PORTABLE_PIXMAP_ASCII: /* Portable Pixmap ASCII */
			if (fcommentf(fp, &Width) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &Height) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &MaxInt) != PNM_FUNCTION_SUCCESS) {
				ErrorFunctionName = "fcommentf";
				ErrorValueName = "Width, Height and MaxInt";
				goto ErrorRead;
			}
			if ((double)size_t_max < (double)Width * (double)Height) {
				goto ErrorImageSize;
			}
			if (fgetc(fp) == EOF) { /* Pass through '\n' */
				ErrorFunctionName = "fgetc";
				ErrorValueName = "\\n";
				goto ErrorRead;
			}
			if ((img = (pnm_img *)calloc((size_t)(3 * Width * Height), sizeof(int))) == NULL) {
				ErrorFunctionName = "calloc";
				ErrorValueName = "img";
				goto ErrorMalloc;
			}
			test = 0;
			for (m = 0; m < Width * Height; m++) {
				test |= fscanf(fp, "%7d", &(img[m]));
				test |= fscanf(fp, "%7d", &(img[Width * Height + m]));
				test |= fscanf(fp, "%7d", &(img[2 * Width * Height + m]));
			}
			if (test != 1) {
				ErrorFunctionName = "fscanf";
				ErrorValueName = "img";
				goto ErrorRead;
			}
			break;
		case PORTABLE_BITMAP_BINARY: /* Portable Bitmap Binary */
			if (fcommentf(fp, &Width) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &Height) != PNM_FUNCTION_SUCCESS) {
				ErrorFunctionName = "fcommentf";
				ErrorValueName = "Width and Height";
				goto ErrorRead;
			}
			if ((double)size_t_max < (double)Width * (double)Height) {
				goto ErrorImageSize;
			}
			MaxInt = 1;
			if (fgetc(fp) == EOF) { /* Pass through '\n' */
				ErrorFunctionName = "fgetc";
				ErrorValueName = "\\n";
				goto ErrorRead;
			}
			Width_tmp = (unsigned int)ceil((double)Width / BITS_OF_BYTE);
			if ((img = (pnm_img *)calloc((size_t)(Width * Height), sizeof(int))) == 0) {
				ErrorFunctionName = "calloc";
				ErrorValueName = "img";
				goto ErrorMalloc;
			}
			if ((img_uint8 = (uint8_t *)calloc((size_t)(Width_tmp * Height), sizeof(uint8_t))) == 0) {
				ErrorFunctionName = "calloc";
				ErrorValueName = "img_uint8";
				goto ErrorMalloc;
			}
			if ((unsigned int)fread(img_uint8, sizeof(uint8_t), (size_t)(Width_tmp * Height), fp) != Width_tmp * Height) {
				ErrorFunctionName = "fread";
				ErrorValueName = "img_uint8";
				goto ErrorRead;
			}
			for (m = 0; m < Height; m++) {
				for (n = 0; n < Width_tmp; n++) {
					for (byte = 0; byte < BITS_OF_BYTE && BITS_OF_BYTE * n + byte < Width; byte++) {
						img[Width * m + BITS_OF_BYTE * n + byte]
						    = ((img_uint8[Width_tmp * m + n] & (1 << (BITS_OF_BYTE - byte - 1))) == 0) ? 1 : 0;
					}
				}
			}
			break;
		case PORTABLE_GRAYMAP_BINARY: /* Portable Graymap Binary */
			if (fcommentf(fp, &Width) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &Height) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &MaxInt) != PNM_FUNCTION_SUCCESS) {
				ErrorFunctionName = "fcommentf";
				ErrorValueName = "Width, Height and MaxInt";
				goto ErrorRead;
			}
			if ((double)size_t_max < (double)Width * (double)Height) {
				goto ErrorImageSize;
			}
			if (fgetc(fp) == EOF) { /* Pass through '\n' */
				ErrorFunctionName = "fgetc";
				ErrorValueName = "\\n";
				goto ErrorRead;
			}
			if ((img = (pnm_img *)calloc((size_t)(Width * Height), sizeof(int))) == 0) {
				ErrorFunctionName = "calloc";
				ErrorValueName = "img";
				goto ErrorMalloc;
			}
			if (MaxInt > 0xFF) {
				/* 16-bit data */
				if ((img_uint8 = (uint8_t *)calloc((size_t)(Width * Height), sizeof(uint16_t))) == 0) {
					ErrorFunctionName = "calloc";
					ErrorValueName = "img_uint8";
					goto ErrorMalloc;
				}
				if ((unsigned int)fread(img_uint8, sizeof(uint8_t), (size_t)(2 * Width * Height), fp) != 2 * Width * Height) {
					ErrorFunctionName = "fread";
					ErrorValueName = "img_uint8";
					goto ErrorRead;
				}
				for (m = 0; m < Width * Height; m++) {
					img[m] = (int)(img_uint8[2 * m] << 8) + (int)img_uint8[2 * m + 1];
				}
			} else {
				/* 8-bit data */
				if ((img_uint8 = (uint8_t *)calloc((size_t)(Width * Height), sizeof(uint8_t))) == 0) {
					ErrorFunctionName = "calloc";
					ErrorValueName = "img_uint8";
					goto ErrorMalloc;
				}
				if ((unsigned int)fread(img_uint8, sizeof(uint8_t), (size_t)(Width * Height), fp) != Width * Height) {
					ErrorFunctionName = "fread";
					ErrorValueName = "img_uint8";
					goto ErrorRead;
				}
				for (m = 0; m < Width * Height; m++) {
					img[m] = (int)img_uint8[m];
				}
			}
			break;
		case PORTABLE_PIXMAP_BINARY: /* Portable Pixmap Binary */
			if (fcommentf(fp, &Width) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &Height) != PNM_FUNCTION_SUCCESS
			    || fcommentf(fp, &MaxInt) != PNM_FUNCTION_SUCCESS) {
				ErrorFunctionName = "fcommentf";
				ErrorValueName = "Width, Height and MaxInt";
				goto ErrorRead;
			}
			if ((double)size_t_max < (double)Width * (double)Height) {
				goto ErrorImageSize;
			}
			if (fgetc(fp) == EOF) { /* Pass through '\n' */
				ErrorFunctionName = "fgetc";
				ErrorValueName = "\\n";
				goto ErrorRead;
			}
			if ((img = (pnm_img *)calloc((size_t)(3 * Width * Height), sizeof(int))) == 0) {
				ErrorFunctionName = "calloc";
				ErrorValueName = "img";
				goto ErrorMalloc;
			}
			if (MaxInt > 0xFF) {
				/* 16-bit data */
				if ((img_uint8 = (uint8_t *)calloc((size_t)(3 * Width), sizeof(uint16_t))) == 0) {
					ErrorFunctionName = "calloc";
					ErrorValueName = "img_uint8";
					goto ErrorMalloc;
				}
				for (m = 0; m < Height; m++) {
					if ((unsigned int)fread(img_uint8, sizeof(uint8_t), (size_t)(2 * 3 * Width), fp) != 2 * 3 * Width) {
						ErrorFunctionName = "fread";
						ErrorValueName = "img_uint8";
						goto ErrorRead;
					}
					for (n = 0; n < Width; n++) {
						img[Width * m + n] = (int)(img_uint8[2 * 3 * n] << 8) + (int)img_uint8[2 * 3 * n + 1];
						img[Width * Height + Width * m + n] = (int)(img_uint8[2 * 3 * n + 2] << 8) + (int)img_uint8[2 * 3 * n + 3];
						img[2 * Width * Height + Width * m + n] = (int)(img_uint8[2 * 3 * n + 4] << 8) + (int)img_uint8[2 * 3 * n + 5];
					}
				}
			} else {
				/* 8-bit data */
				if ((img_uint8 = (uint8_t *)calloc((size_t)(3 * Width), sizeof(uint8_t))) == 0) {
					ErrorFunctionName = "calloc";
					ErrorValueName = "img_uint8";
					goto ErrorMalloc;
				}
				for (m = 0; m < Height; m++) {
					if ((unsigned int)fread(img_uint8, sizeof(uint8_t), (size_t)(3 * Width), fp) != 3 * Width) {
						ErrorFunctionName = "fread";
						ErrorValueName = "img_uint8";
						goto ErrorRead;
					}
					for (n = 0; n < Width; n++) {
						img[Width * m + n] = (int)img_uint8[3 * n];
						img[Width * Height + Width * m + n] = (int)img_uint8[3 * n + 1];
						img[2 * Width * Height + Width * m + n] = (int)img_uint8[3 * n + 2];
					}
				}
			}
			break;
		default: /* ERROR */
			goto ErrorIncorrectDescriptor;
	}
	free(img_uint8);
	img_uint8 = NULL;
	if (fclose(fp) == EOF) {
		fprintf(stderr, "*** pnmread error - Failed to close the file by fclose() ***\n");
		free(img);
		img = NULL;
		return PNM_FUNCTION_ERROR;
	}
	pnm->desc = (int)FileDescriptor[1] - (int)'0';
	pnm->width = Width;
	pnm->height = Height;
	pnm->maxint = MaxInt;
	pnm->img = img;
	img = NULL;
#ifndef PNM_NO_PRINT
	printf("  Width : %u\n  Height %u\n  Bit Depth : %d\n", Width, Height, pnm_bitdepth(pnm));
	printf("--- Successfully read the image ---\n\n");
#endif
	return PNM_FUNCTION_SUCCESS;

/* Errors */
ErrorMalloc:
	fprintf(stderr, "*** pnmread error - Cannot allocate memory for (*%s) by %s() ***\n", ErrorValueName, ErrorFunctionName);
	goto ErrorReturn;
ErrorPointerNULL:
	fprintf(stderr, "*** pnmread error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	goto ErrorReturn;
ErrorPointerNotInitialized:
	fprintf(stderr, "*** pnmread error - The pointer (*%s) may already be allocated or not initialized ***\n", ErrorValueName);
	goto ErrorReturn;
ErrorRead:
	fprintf(stderr, "*** pnmread error - Cannot read %s correctly by %s() ***\n", ErrorValueName, ErrorFunctionName);
	goto ErrorReturn;
ErrorFileOpen:
	fprintf(stderr, "*** pnmread error - Cannot find \"%s\" by %s() for (%s) ***\n", ErrorFileName, ErrorFunctionName, ErrorValueName);
	goto ErrorReturn;
ErrorImageSize:
	fprintf(stderr, "*** pnmread error - The size of image is larger than size_t ***\n");
	goto ErrorReturn;
ErrorIncorrectDescriptor:
	fprintf(stderr, "*** pnmread error - Descriptor is incorrect (\"%s\") ***\n", FileDescriptor);
	goto ErrorReturn;
ErrorOthers:
	fprintf(stderr, "*** pnmread error - %s ***\n", ErrorDescription);

ErrorReturn:
	if (fp != NULL && fclose(fp) == EOF) {
		fprintf(stderr, "*** pnmread error - Failed to close the file by fclose() ***\n");
	}
	free(img_uint8);
	img_uint8 = NULL;
	free(img);
	img = NULL;
	return PNM_FUNCTION_ERROR;
}


char *
pnm_FixExtension(char *filename, int desc)
{
	char *FunctionName = "pnm_FixExtension";
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";

	char *extension = NULL;
	char *fixed = NULL;

	if (filename == NULL) {
		fprintf(stderr, "*** %s() error - The pointer (*filename) is NULL ***\n", FunctionName);
		return NULL;
	}
	extension = filename + strlen(filename) - 4;
	if (extension > filename
	    && *extension == '.' && *(extension + 1) == 'p' && *(extension + 3) == 'm') {
		if ((fixed = (char *)calloc(strlen(filename) + 1, sizeof(char))) == NULL) {
			ErrorFunctionName = "calloc";
			ErrorValueName = "fixed";
			goto ErrorMalloc;
		}
		if (strcpy(fixed, filename) == NULL) {
			ErrorFunctionName = "strcpy";
			ErrorValueName = "(filename -> fixed)";
			goto ErrorFunctionFailed;
		}
		extension = fixed + strlen(fixed) - 4;
		switch (desc) {
			case PORTABLE_BITMAP_ASCII:
			case PORTABLE_BITMAP_BINARY:
				if (*(extension + 2) != 'b') {
					fprintf(stderr, "*** %s() warning - File extension is incorrect. Automatically fixed it to '%s'\n", FunctionName, ".pbm");
					*(extension + 2) = 'b';
				}
				break;
			case PORTABLE_GRAYMAP_ASCII:
			case PORTABLE_GRAYMAP_BINARY:
				if (*(extension + 2) != 'g') {
					fprintf(stderr, "*** %s() warning - File extension is incorrect. Automatically fixed it to '%s'\n", FunctionName, ".pgm");
					*(extension + 2) = 'g';
				}
				break;
			case PORTABLE_PIXMAP_ASCII:
			case PORTABLE_PIXMAP_BINARY:
				if (*(extension + 2) != 'p') {
					fprintf(stderr, "*** %s() warning - File extension is incorrect. Automatically fixed it to '%s'\n", FunctionName, ".ppm");
					*(extension + 2) = 'p';
				}
		}
	} else {
		if ((fixed = (char *)calloc(strlen(filename) + 5, sizeof(char))) == NULL) {
			ErrorFunctionName = "calloc";
			ErrorValueName = "fixed";
			goto ErrorMalloc;
		}
		if (strcpy(fixed, filename) == NULL) {
			ErrorFunctionName = "strcpy";
			ErrorValueName = "(filename -> fixed)";
			goto ErrorFunctionFailed;
		}
		switch (desc) {
			case PORTABLE_BITMAP_ASCII:
			case PORTABLE_BITMAP_BINARY:
				if (strcpy(fixed + strlen(filename), ".pbm") == NULL) {
					ErrorFunctionName = "strcpy";
					ErrorValueName = "('.pbm' +> fixed)";
					goto ErrorFunctionFailed;
				}
				break;
			case PORTABLE_GRAYMAP_ASCII:
			case PORTABLE_GRAYMAP_BINARY:
				if (strcpy(fixed + strlen(filename), ".pgm") == NULL) {
					ErrorFunctionName = "strcpy";
					ErrorValueName = "('.pgm' +> fixed)";
					goto ErrorFunctionFailed;
				}
				break;
			case PORTABLE_PIXMAP_ASCII:
			case PORTABLE_PIXMAP_BINARY:
				if (strcpy(fixed + strlen(filename), ".ppm") == NULL) {
					ErrorFunctionName = "strcpy";
					ErrorValueName = "('.ppm' +> fixed)";
					goto ErrorFunctionFailed;
				}
		}
	}
	return fixed;
/* Error */
ErrorMalloc:
	fprintf(stderr, "*** %s error - Cannot allocate memory for (*%s) by %s() ***\n", FunctionName, ErrorValueName, ErrorFunctionName);
	goto ErrorReturn;
ErrorFunctionFailed:
	fprintf(stderr, "*** %s error - %s() failed to compute (%s) ***\n", FunctionName, ErrorFunctionName, ErrorValueName);
ErrorReturn:
	free(fixed);
	return NULL;
}


int
pnmwrite(PNM *pnm, char *filename)
{
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";

	char *fixed_filename = NULL;
	uint8_t *img_uint8 = NULL;
	unsigned int Width = 0u;
	unsigned int Height = 0u;
	unsigned int MaxInt = 0u;
	unsigned int Width_tmp = 0u;
	unsigned int m, n;
	unsigned int byte;
	FILE *fp = NULL;

	if (pnm == NULL) {
		ErrorValueName = "pnm";
		goto ErrorPointerNull;
	} else if (pnm->img == NULL) {
		ErrorValueName = "(pnm->img)";
		goto ErrorPointerNull;
	} else if (filename == NULL) {
		ErrorValueName = "filename";
		goto ErrorPointerNull;
	}
	if ((fixed_filename = pnm_FixExtension(filename, pnm->desc)) == NULL) {
		ErrorFunctionName = "pnm_FixExtension";
		ErrorValueName = "(filename -> fixed)";
		goto ErrorFunctionFailed;
	}
	Width = pnm->width;
	Height = pnm->height;
	MaxInt = pnm->maxint;
	if ((fp = fopen(fixed_filename, "wb")) == NULL) {
		fprintf(stderr, "*** pnmwrite() error - Cannot open the file \"%s\" ***\n", fixed_filename);
		return PNM_FUNCTION_ERROR;
	}
	printf("\n--- Writing to \"%s\" ---\n  Output PNM descriptor : %d\n", fixed_filename, pnm->desc);
	printf("  Width     = %u\n  Height    = %u\n  Intensity = %u\n", Width, Height, MaxInt);
	free(fixed_filename);
	fixed_filename = NULL;

	switch (pnm->desc) {
		case PORTABLE_BITMAP_ASCII:
			fprintf(fp, "P1\n%u %u\n", Width, Height);
			for (m = 0; m < Height; m++) {
				for (n = 0; n < Width; n++) {
					fprintf(fp, "%d ", pnm->img[Width * m + n] > 0 ? 0 : 1);
				}
				fprintf(fp, "\n");
			}
			break;
		case PORTABLE_GRAYMAP_ASCII:
			fprintf(fp, "P2\n%u %u\n%u\n", Width, Height, MaxInt);
			for (m = 0; m < Height; m++) {
				for (n = 0; n < Width; n++) {
					fprintf(fp, "%d ", pnm->img[Width * m + n]);
				}
				fprintf(fp, "\n");
			}
			break;
		case PORTABLE_PIXMAP_ASCII:
			fprintf(fp, "P3\n%u %u\n", Width, Height);
			for (m = 0; m < Height; m++) {
				for (n = 0; n < Width; n++) {
					fprintf(fp, "%d %d %d", pnm->img[Width * m + n], pnm->img[Height * Width + Width * m + n], pnm->img[2 * Height * Width + Width * m + n]);
				}
				fprintf(fp,"\n");
			}
			break;
		case PORTABLE_BITMAP_BINARY: /* Bitmap */
			/* 16-bit and 8-bit data */
			Width_tmp = (unsigned int)ceil((double)Width / BITS_OF_BYTE);
			if ((img_uint8 = (uint8_t *)calloc((size_t)(Width_tmp * Height), sizeof(uint8_t))) == NULL) {
				ErrorFunctionName = "calloc";
				ErrorValueName = "img_uint8";
				goto ErrorMalloc;
			}
			for (m = 0; m < Height; m++) {
				for (n = 0; n < Width_tmp; n++) {
					for (byte = 0; byte < BITS_OF_BYTE; byte++) {
						img_uint8[Width_tmp * m + n] <<= 1;
						if (BITS_OF_BYTE * n + byte < Width
						    && pnm->img[Width * m + BITS_OF_BYTE * n + byte] < (int)(MaxInt + 1) / 2) {
							img_uint8[Width_tmp * m + n] |= 1u;
						}
					}
				}
			}
			fprintf(fp, "P4\n%u %u\n", Width, Height);
			if ((unsigned int)fwrite(img_uint8, sizeof(uint8_t), (size_t)(Width_tmp * Height), fp) != Width_tmp * Height) {
				ErrorFunctionName = "fwrite";
				ErrorValueName = "*img_uint8";
				goto ErrorFunctionFailed;
			}
			break;
		case PORTABLE_GRAYMAP_BINARY: /* Portable Graymap Binary */
			fprintf(fp, "P5\n%u %u\n%u\n", Width, Height, MaxInt);
			if (MaxInt > 0xFF) {
				/* 16-bit data align in Big Endian order in 8-bit array */
				if ((img_uint8 = (uint8_t *)calloc((size_t)(Width * Height), sizeof(uint16_t))) == NULL) {
					ErrorFunctionName = "calloc";
					ErrorValueName = "img_uint8";
					goto ErrorMalloc;
				}
				for (m = 0; m < Width * Height; m++) {
					img_uint8[2 * m] = (uint8_t)(((unsigned int)pnm->img[m] >> 8) & 0xFF);
					img_uint8[2 * m + 1] = (uint8_t)((unsigned int)pnm->img[m] & 0xFF);
				}
				if ((unsigned int)fwrite(img_uint8, sizeof(uint8_t), (size_t)(2 * Width * Height), fp) != 2 * Width * Height) {
					ErrorFunctionName = "fwrite";
					ErrorValueName = "img_uint8";
					goto ErrorFunctionFailed;
				}
			} else {
				/* 8-bit data */
				if ((img_uint8 = (uint8_t *)calloc((size_t)(Width * Height), sizeof(uint8_t))) == NULL) {
					ErrorFunctionName = "calloc";
					ErrorValueName = "img_uint8";
					goto ErrorMalloc;
				}
				for (m = 0; m < Width * Height; m++) {
					img_uint8[m] = (uint8_t)((unsigned int)pnm->img[m] & 0xFF);
				}
				if ((unsigned int)fwrite(img_uint8, sizeof(uint8_t), (size_t)(Width * Height), fp) != Width * Height) {
					ErrorFunctionName = "fwrite";
					ErrorValueName = "img_uint8";
					goto ErrorFunctionFailed;
				}
			}
			break;
		case PORTABLE_PIXMAP_BINARY: /* Portable Pixmap Binary */
			fprintf(fp, "P6\n%u %u\n%u\n", Width, Height, MaxInt);
			if (MaxInt > 0xFF) {
				/* 16-bit data align in Big Endian order in 8-bit array */
				if ((img_uint8 = (uint8_t *)calloc((size_t)(3 * Width * Height), sizeof(uint16_t))) == NULL) {
					ErrorFunctionName = "calloc";
					ErrorValueName = "img_uint8";
					goto ErrorMalloc;
				}
				for (m = 0; m < Width * Height; m++) {
					img_uint8[6 * m + 1] = (uint8_t)(((unsigned int)pnm->img[m] >> 8) & 0xFF); /* Higher 8-bit */
					img_uint8[6 * m + 2] = (uint8_t)((unsigned int)pnm->img[m] & 0xFF); /* Lower 8-bit */
					img_uint8[6 * m + 3] = (uint8_t)(((unsigned int)pnm->img[Width * Height + m] >> 8) & 0xFF); /* Higher 8-bit */
					img_uint8[6 * m + 4] = (uint8_t)((unsigned int)pnm->img[Width * Height + m] & 0xFF); /* Lower 8-bit */
					img_uint8[6 * m + 5] = (uint8_t)(((unsigned int)pnm->img[2 * Width * Height + m] >> 8) & 0xFF); /* Higher 8-bit */
					img_uint8[6 * m + 6] = (uint8_t)((unsigned int)pnm->img[2 * Width * Height + m] & 0xFF); /* Lower 8-bit */
				}
				if ((unsigned int)fwrite(img_uint8, sizeof(uint8_t), (size_t)(2 * 3 * Width * Height), fp) != 2 * 3 * Width * Height) {
					ErrorFunctionName = "fwrite";
					ErrorValueName = "img_uint8";
					goto ErrorFunctionFailed;
				}
			} else {
				/* 8-bit data */
				if ((img_uint8 = (uint8_t *)calloc((size_t)(3 * Width * Height), sizeof(uint8_t))) == NULL) {
					ErrorFunctionName = "calloc";
					ErrorValueName = "img_uint8";
					goto ErrorMalloc;
				}
				for (m = 0; m < Width * Height; m++) {
					img_uint8[3 * m] = (uint8_t)((unsigned int)pnm->img[m] & 0xFF);
					img_uint8[3 * m + 1] = (uint8_t)((unsigned int)pnm->img[Width * Height + m] & 0xFF);
					img_uint8[3 * m + 2] = (uint8_t)((unsigned int)pnm->img[2 * Width * Height + m] & 0xFF);
				}
				if ((unsigned int)fwrite(img_uint8, sizeof(uint8_t), (size_t)(3 * Width * Height), fp) != 3 * Width * Height) {
					ErrorFunctionName = "fwrite";
					ErrorValueName = "img_uint8";
					goto ErrorFunctionFailed;
				}
			}
			break;
		default: /* ERROR */
			fprintf(stderr, "*** pnmwrite() error - Descriptor is incorrect (%d) ***\n", pnm->desc);
			if (fclose(fp) == EOF) {
				fprintf(stderr, "*** pnmwrite() error - Failed to close the File by fclose() ***\n");
			}
			fp = NULL;
			return PNM_FUNCTION_ERROR;
	}
	free(img_uint8);
	img_uint8 = NULL;
	if (fclose(fp) == EOF) {
		fprintf(stderr, "*** pnmwrite() error - Failed to close the File by fclose() ***\n");
		return PNM_FUNCTION_ERROR;
	}
	printf("--- Successfully wrote the image ---\n\n");
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorMalloc:
	fprintf(stderr, "*** pnmwrite() error - Cannot allocate memory for (*%s) by %s() ***\n", ErrorValueName, ErrorFunctionName);
	goto ErrorReturn;
ErrorFunctionFailed:
	fprintf(stderr, "*** pnmwrite() error - %s() failed to compute (%s) ***\n", ErrorFunctionName, ErrorValueName);
	goto ErrorReturn;
ErrorPointerNull:
	fprintf(stderr, "*** pnmwrite() error - The pointer to (*%s) is NULL ***\n", ErrorValueName);
ErrorReturn:
	free(img_uint8);
	img_uint8 = NULL;
	if (fclose(fp) == EOF) {
		fprintf(stderr, "*** pnmwrite() error - Failed to close the File by fclose() ***\n");
	}
	return PNM_FUNCTION_ERROR;
}


int
pnmnew(PNM *pnm, int desc, unsigned int width, unsigned int height, unsigned int maxint)
{
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";
	int ErrorValue = 0;

	size_t size_t_max = (size_t)(~0u);

	if (pnm == NULL) {
		ErrorValueName = "pnm";
		goto ErrorPointerNull;
	} else if (pnm->img != NULL) {
		ErrorValueName = "(pnm->img)";
		goto ErrorPointerNull;
	}
	if ((double)size_t_max < (double)width * (double)height) {
		ErrorValueName = "(width * height)";
		ErrorValue = (int)width * height;
		goto ErrorIncorrectValue;
	}

	if ((desc < PNM_DESCRIPTOR_MIN) || (PNM_DESCRIPTOR_MAX < desc)) {
		ErrorValueName = "desc";
		ErrorValue = desc;
		goto ErrorIncorrectValue;
	}
	*pnm = (PNM){desc, width, height, maxint, NULL};
	if ((desc % PNM_DESCRIPTOR_PIXMAPS) == 0) {
		if ((pnm->img = (pnm_img *)calloc((size_t)(3 * width * height), sizeof(pnm_img))) == NULL) {
			ErrorFunctionName = "calloc";
			ErrorValueName = "(pnm->img)";
			*pnm = PNM_NULL;
			goto ErrorMalloc;
		}
	} else {
		if ((pnm->img = (pnm_img *)calloc((size_t)(width * height), sizeof(pnm_img))) == NULL) {
			ErrorFunctionName = "calloc";
			ErrorValueName = "(pnm->img)";
			*pnm = PNM_NULL;
			goto ErrorMalloc;
		}
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorMalloc:
	fprintf(stderr, "*** pnmnew() error - Cannot allocate memory for (*%s) by %s() ***\n", ErrorFunctionName, ErrorValueName);
	goto ErrorReturn;
ErrorPointerNull:
	fprintf(stderr, "*** pnmnew() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	goto ErrorReturn;
ErrorIncorrectValue:
	fprintf(stderr, "*** pnmnew() error - The variable (%s) is out of bounds or incorrect value (%d) ***\n", ErrorValueName, ErrorValue);
ErrorReturn:
	return PNM_FUNCTION_ERROR;
}


int
pnmdouble_new(PNM_DOUBLE *pnmd, int desc, unsigned int width, unsigned int height, unsigned int maxint)
{
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";
	int ErrorValue = 0;

	size_t size_t_max = (size_t)(~0u);
	unsigned int n;

	if (pnmd == NULL) {
		ErrorValueName = "pnmd";
		goto ErrorPointerNull;
	} else if (pnmd->imgd != NULL) {
		ErrorValueName = "*(pnmd->imgd)";
		goto ErrorNotInitialized;
	}
	if ((double)size_t_max < (double)width * (double)height) {
		ErrorValueName = "(width * height)";
		ErrorValue = (int)width * height;
		goto ErrorIncorrectValue;
	}

	if ((desc < PNM_DESCRIPTOR_MIN) || (PNM_DESCRIPTOR_MAX < desc)) {
		ErrorValueName = "desc";
		ErrorValue = desc;
		goto ErrorIncorrectValue;
	}
	*pnmd = (PNM_DOUBLE){desc, width, height, maxint, NULL};
	if ((desc % PNM_DESCRIPTOR_PIXMAPS) == 0) {
		if ((pnmd->imgd = (pnm_img_double *)calloc((size_t)(3 * width * height), sizeof(pnm_img_double))) == NULL) {
			ErrorFunctionName = "calloc";
			ErrorValueName = "(pnmd->imgd)";
			*pnmd = PNM_DOUBLE_NULL;
			goto ErrorMalloc;
		}
		for (n = 0; n < 3 * width * height; n++) {
			pnmd->imgd[n] = 0;
		}
	} else {
		if ((pnmd->imgd = (pnm_img_double *)calloc((size_t)(width * height), sizeof(pnm_img_double))) == NULL) {
			ErrorFunctionName = "calloc";
			ErrorValueName = "(pnmd->imgd)";
			*pnmd = PNM_DOUBLE_NULL;
			goto ErrorMalloc;
		}
		for (n = 0; n < width * height; n++) {
			pnmd->imgd[n] = 0;
		}
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorMalloc:
	fprintf(stderr, "*** pnmnew() error - Cannot allocate memory for (*%s) by %s() ***\n", ErrorValueName, ErrorFunctionName);
	goto ErrorReturn;
ErrorPointerNull:
	fprintf(stderr, "*** pnmnew() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	goto ErrorReturn;
ErrorNotInitialized:
	fprintf(stderr, "*** pnmnew() error - The pointer (*%s) may already be allocated or not initialized ***\n", ErrorValueName);
	goto ErrorReturn;
ErrorIncorrectValue:
	fprintf(stderr, "*** pnmnew() error - The variable (%s) is out of bound or incorrect value (%d) ***\n", ErrorValueName, ErrorValue);
ErrorReturn:
	return PNM_FUNCTION_ERROR;
}


PNM *
pnmnew_NULL(void)
{
	PNM *pnm_temp = NULL;

	if ((pnm_temp = (PNM *)malloc(sizeof(PNM))) == NULL) {
		fprintf(stderr, "*** pnmnew_NULL error - Cannot allocate memory for *pnm ***\n");
		return NULL;
	}
	*pnm_temp = PNM_NULL;
	return pnm_temp;
}


PNM_DOUBLE *
pnmdouble_new_NULL(void)
{
	PNM_DOUBLE *pnmd_temp = NULL;

	if ((pnmd_temp = (PNM_DOUBLE *)malloc(sizeof(PNM_DOUBLE))) == NULL) {
		fprintf(stderr, "*** pnmdouble_new_NULL error - Cannot allocate memory for *pnmd_temp ***\n");
		return NULL;
	}
	*pnmd_temp = PNM_DOUBLE_NULL;
	return pnmd_temp;
}


void
pnmfree(PNM *pnm)
{
	if (pnm == NULL) {
		return;
	}
	free(pnm->img);
	*pnm = PNM_NULL;
}


void
pnmdouble_free(PNM_DOUBLE *pnmd)
{
	if (pnmd == NULL) {
		return;
	}
	free(pnmd->imgd);
	*pnmd = PNM_DOUBLE_NULL;
}


void
pnmdestroy(PNM *pnm)
{
	if (pnm == NULL) {
		fprintf(stderr, "*** pnmdestroy warning - The pointer to *pnm is NULL (**pnm == 0) ***\n");
		return;
	}
	free(pnm->img);
	pnm->img = NULL;
	free(pnm);
}


void
pnmdouble_destroy(PNM_DOUBLE *pnmd)
{
	if (pnmd == NULL) {
		fprintf(stderr, "*** pnm_doubledestroy warning - The pointer to *pnmd is NULL (**pnmd) ***\n");
		return;
	}
	free(pnmd->imgd);
	pnmd->imgd = NULL;
	free(pnmd);
}


int
pnmcp(PNM *pnm_to, PNM *pnm_from)
{
	char *ErrorValueName = "";
	unsigned int i;

	if (pnm_to == NULL) {
		ErrorValueName = "pnm_to";
		goto ErrorPointerNull;
	} else if (pnm_isNULL(pnm_to) != PNM_TRUE) {
		ErrorValueName = "*(pnm_to->img)";
		goto ErrorNotInitialized;
	}
	if (pnm_from == NULL) {
		ErrorValueName = "pnm_from";
		goto ErrorPointerNull;
	} else if (pnm_isNULL(pnm_from) == PNM_TRUE) {
		ErrorValueName = "(pnm_from->img)";
		goto ErrorPointerNull;
	}

	if (pnmnew(pnm_to, pnm_from->desc, pnm_from->width, pnm_from->height, pnm_from->maxint) != PNM_FUNCTION_SUCCESS) {
		fprintf(stderr, "*** pnmcp() error - Cannot allocate memory for (*pnm_to) by pnmnew() ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if (pnm_isRGB(pnm_from) == PNM_TRUE) {
		for (i = 0; i < 3u * pnm_from->width * pnm_from->height; i++) {
			pnm_to->img[i] = pnm_from->img[i];
		}
	} else {
		for (i = 0; i < pnm_from->width * pnm_from->height; i++) {
			pnm_to->img[i] = pnm_from->img[i];
		}
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnmcp() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
ErrorNotInitialized:
	fprintf(stderr, "*** pnmcp() error - The variable (%s) is NOT initialized ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
}


int
pnmdouble_cp(PNM_DOUBLE *pnm_to, PNM_DOUBLE *pnm_from)
{
	char *ErrorValueName = "";
	unsigned int i;

	if (pnm_to == NULL) {
		ErrorValueName = "pnm_to";
		goto ErrorPointerNull;
	} else if (pnmdouble_isNULL(pnm_to) != PNM_TRUE) {
		ErrorValueName = "*(pnm_to->imgd)";
		goto ErrorNotInitialized;
	}
	if (pnm_from == NULL) {
		ErrorValueName = "pnm_from";
		goto ErrorPointerNull;
	} else if (pnmdouble_isNULL(pnm_from) == PNM_TRUE) {
		ErrorValueName = "pnm_from";
		goto ErrorPointerNull;
	}

	if (pnmdouble_new(pnm_to, pnm_from->desc, pnm_from->width, pnm_from->height, pnm_from->maxint) != PNM_FUNCTION_SUCCESS) {
		fprintf(stderr, "*** pnmdouble_cp() error - Cannot allocate memory for (*pnm_to) by pnmdouble_new() ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if (pnmdouble_isRGB(pnm_from) == PNM_TRUE) {
		for (i = 0; i < 3u * pnm_from->width * pnm_from->height; i++) {
			pnm_to->imgd[i] = pnm_from->imgd[i];
		}
	} else {
		for (i = 0; i < pnm_from->width * pnm_from->height; i++) {
			pnm_to->imgd[i] = pnm_from->imgd[i];
		}
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnmdouble_cp() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
ErrorNotInitialized:
	fprintf(stderr, "*** pnmdouble_cp() error - The variable (%s) is NOT initialized ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
}


int
pnm_offset(PNM_OFFSET *offset, double r, double g, double b)
{
	if (offset == NULL) {
		fprintf(stderr, "*** pnm_offset error - The pointer to offset is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	}
	offset->r = r;
	offset->g = g;
	offset->b = b;
	return PNM_FUNCTION_SUCCESS;
}


int
pnm_double2int(PNM *pnm_int, PNM_DOUBLE *pnm_double, double coeff, const char *process, PNM_OFFSET *add_offset)
{
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";

	double FRC = 0.5;
	unsigned int M;
	unsigned int N;
	unsigned int i;

	if (pnm_double == NULL) {
		ErrorValueName = "pnm_double";
		goto ErrorPointerNull;
	} else if (pnm_double->imgd == NULL) {
		ErrorValueName = "(pnm_double->imgd)";
		goto ErrorPointerNull;
	}
	if (pnm_isNULL(pnm_int) == PNM_FALSE) {
		ErrorValueName = "pnm_int";
		goto ErrorNotInitialized;
	}
	if (fabs(coeff) <= 1.0 / pnm_double->maxint) {
		fprintf(stderr, "*** pnm_double2int() warning - The coefficient is ZERO ***\n");
	}

	if (pnmnew(pnm_int, pnm_double->desc, pnm_double->width, pnm_double->height, pnm_double->maxint) != PNM_FUNCTION_SUCCESS) {
		ErrorFunctionName = "pnmnew";
		ErrorValueName = "pnm_int";
		goto ErrorFunctionFailed;
	}
	M = pnm_double->height;
	N = pnm_double->width;
	if (process != NULL) {
		if (strcmp(process, "floor") == 0) {
			FRC = .0;
		} else if (strcmp(process, "round") == 0) {
			FRC = 0.5;
		} else if (strcmp(process, "ceil") == 0) {
			FRC = 1.0;
		}
	}
	switch (pnm_double->desc) {
		case PORTABLE_BITMAP_ASCII:
		case PORTABLE_GRAYMAP_ASCII:
		case PORTABLE_BITMAP_BINARY:
		case PORTABLE_GRAYMAP_BINARY:
			for (i = 0; i < M * N; i++) {
				if (add_offset == NULL) {
					pnm_int->img[i] = (pnm_img)floor(coeff * pnm_double->imgd[i] + FRC); /* adding FRC is for same as round() on C89 */
				} else {
					pnm_int->img[i] = (pnm_img)floor(coeff * pnm_double->imgd[i] + add_offset->r + FRC); /* adding FRC is for same as round() on C89 */
				}
			}
			break;
		case PORTABLE_PIXMAP_ASCII:
		case PORTABLE_PIXMAP_BINARY:
			for (i = 0; i < M * N; i++) {
				if (add_offset == NULL) { /* adding FRC is for same as round() on C89 */
					pnm_int->img[i] = (pnm_img)floor(coeff * pnm_double->imgd[i] + FRC);
					pnm_int->img[M * N + i] = (pnm_img)floor(coeff * pnm_double->imgd[M * N + i] + FRC);
					pnm_int->img[2 * M * N + i] = (pnm_img)floor(coeff * pnm_double->imgd[2 * M * N + i] + FRC);
				} else {
					pnm_int->img[i] = (pnm_img)floor(coeff * pnm_double->imgd[i] + add_offset->r + FRC);
					pnm_int->img[M * N + i] = (pnm_img)floor(coeff * pnm_double->imgd[M * N + i] + add_offset->g + FRC);
					pnm_int->img[2 * M * N + i] = (pnm_img)floor(coeff * pnm_double->imgd[2 * M * N + i] + add_offset->b + FRC);
				}
			}
			break;
		default: /* ERROR */
			fprintf(stderr, "*** pnm_double2int() error - Descriptor is incorrect (P%d) ***\n", pnm_double->desc);
			return PNM_FUNCTION_ERROR;
	}
	pnm_int->desc = pnm_double->desc;
	pnm_int->width = pnm_double->width;
	pnm_int->height = pnm_double->height;
	pnm_int->maxint = pnm_double->maxint;
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnm_double2int() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
ErrorNotInitialized:
	fprintf(stderr, "*** pnm_double2int() error - The variable (%s) is NOT initialized ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
ErrorFunctionFailed:
	fprintf(stderr, "*** pnm_double2int() error - %s() failed to compute (%s) ***\n", ErrorFunctionName, ErrorValueName);
	return PNM_FUNCTION_ERROR;
}


int
pnm_int2double(PNM_DOUBLE *pnm_double, PNM *pnm_int, double coeff, PNM_OFFSET *add_offset)
{
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";
	int ErrorValue = 0;

	unsigned int M;
	unsigned int N;
	unsigned int i;

	if (pnm_int == NULL) {
		ErrorValueName = "pnm_int";
		goto ErrorPointerNull;
	} else if (pnm_int->img == NULL) {
		ErrorValueName = "(pnm_int->img)";
		goto ErrorPointerNull;
	}
	if (pnmdouble_isNULL(pnm_double) == PNM_FALSE) {
		ErrorValueName = "pnm_double";
		goto ErrorNotInitialized;
	}
	if (fabs(coeff) <= 1.0 / pnm_int->maxint) {
		fprintf(stderr, "*** pnm_int2double() warning - The coefficient is ZERO ***\n");
	}

	if (pnmdouble_new(pnm_double, pnm_int->desc, pnm_int->width, pnm_int->height, pnm_int->maxint) != PNM_FUNCTION_SUCCESS) {
		ErrorFunctionName = "pnmdouble_new";
		ErrorValueName = "*pnm_double";
		goto ErrorFunctionFailed;
	}
	M = pnm_int->height;
	N = pnm_int->width;
	switch (pnm_int->desc) {
		case PORTABLE_BITMAP_ASCII:
		case PORTABLE_GRAYMAP_ASCII:
		case PORTABLE_BITMAP_BINARY:
		case PORTABLE_GRAYMAP_BINARY:
			for (i = 0; i < M * N; i++) {
				if (add_offset == NULL) {
					pnm_double->imgd[i] = coeff * (pnm_img_double)pnm_int->img[i];
				} else {
					pnm_double->imgd[i] = coeff * (pnm_img_double)pnm_int->img[i] + add_offset->r;
				}
			}
			break;
		case PORTABLE_PIXMAP_ASCII:
		case PORTABLE_PIXMAP_BINARY:
			for (i = 0; i < M * N; i++) {
				if (add_offset == NULL) {
					pnm_double->imgd[i] = coeff * (pnm_img_double)pnm_int->img[i];
					pnm_double->imgd[M * N + i] = coeff * (pnm_img_double)pnm_int->img[M * N + i];
					pnm_double->imgd[2 * M * N + i] = coeff * (pnm_img_double)pnm_int->img[2 * M * N + i];
				} else {
					pnm_double->imgd[i] = coeff * (pnm_img_double)pnm_int->img[i] + add_offset->r;
					pnm_double->imgd[M * N + i] = coeff * (pnm_img_double)pnm_int->img[M * N + i] + add_offset->g;
					pnm_double->imgd[2 * M * N + i] = coeff * (pnm_img_double)pnm_int->img[2 * M * N + i] + add_offset->b;
				}
			}
			break;
		default: /* ERROR */
			ErrorValueName = "(pnm_int->desc)";
			ErrorValue = pnm_int->desc;
			goto ErrorIncorrectValue;
	}
	pnm_double->desc = pnm_int->desc;
	pnm_double->width = pnm_int->width;
	pnm_double->height = pnm_int->height;
	pnm_double->maxint = pnm_int->maxint;
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnm_int2double() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	goto ErrorReturn;
ErrorIncorrectValue:
	fprintf(stderr, "*** pnm_int2double() error - The variable (%s) has incorrect value (%d) ***\n", ErrorValueName, ErrorValue);
	goto ErrorReturn;
ErrorNotInitialized:
	fprintf(stderr, "*** pnm_int2double() error - The variable (%s) is NOT initialized ***\n", ErrorValueName);
	goto ErrorReturn;
ErrorFunctionFailed:
	fprintf(stderr, "*** pnm_int2double() error - %s() failed to compute (%s) ***\n", ErrorFunctionName, ErrorValueName);
ErrorReturn:
	pnmdouble_free(pnm_double);
	return PNM_FUNCTION_ERROR;
}


int
pnm_bitdepth(PNM *pnm)
{
	if (pnm == NULL) {
		fprintf(stderr, "*** pnm_bitdepth() error - The pointer to PNM is NULL (*pnm) ***\n");
		return PNM_FUNCTION_ERROR;
	}
	return (int)floor(log(pnm->maxint + 1.0) / log(2.0) + 0.5); /* plus 0.5 is for same as round() on C89 */
}


int
pnmdouble_bitdepth(PNM_DOUBLE *pnmd)
{
	if (pnmd == NULL) {
		fprintf(stderr, "*** pnmdouble_bitdepth() error - The pointer to PNM is NULL (*pnmd) ***\n");
		return PNM_FUNCTION_ERROR;
	}
	return (int)floor(log(pnmd->maxint + 1.0) / log(2.0) + 0.5); /* plus 0.5 is for same as round() on C89 */
}


int
pnm_isNULL(PNM *pnm)
{
	if (pnm == NULL) {
		fprintf(stderr, "*** pnm_isNULL() error - The pointer (*pnm) is NULL *** \n");
		return PNM_FUNCTION_ERROR;
	}
	if (pnm->desc == 0 && pnm->width == 0 && pnm->height == 0 && pnm->maxint == 0 && pnm->img == NULL) {
		return PNM_TRUE;
	} else {
		return PNM_FALSE;
	}
}


int
pnmdouble_isNULL(PNM_DOUBLE *pnm)
{
	if (pnm == NULL) {
		fprintf(stderr, "*** pnmdouble_isNULL() error - The pointer (*pnm) is NULL *** \n");
		return PNM_FUNCTION_ERROR;
	}
	if (pnm->desc == 0 && pnm->width == 0 && pnm->height == 0 && pnm->maxint == 0 && pnm->imgd == NULL) {
		return PNM_TRUE;
	} else {
		return PNM_FALSE;
	}
}


int
pnm_isSameFormat(PNM *pnm1, PNM *pnm2)
{
	if (pnm1 == NULL) {
		fprintf(stderr, "*** pnm_isSameFormat() error - The pointer (*pnm1) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	} else if (pnm2 == NULL) {
		fprintf(stderr, "*** pnm_isSameFormat() error - The pointer (*pnm2) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if ((pnm1->desc % PNM_DESCRIPTOR_LENGTH) == (pnm2->desc % PNM_DESCRIPTOR_LENGTH)) {
		return PNM_TRUE;
	} else {
		return PNM_FALSE;
	}
}


int
pnmdouble_isSameFormat(PNM_DOUBLE *pnm1, PNM_DOUBLE *pnm2)
{
	if (pnm1 == NULL) {
		fprintf(stderr, "*** pnmdouble_isSameFormat() error - The pointer (*pnm1) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	} else if (pnm2 == NULL) {
		fprintf(stderr, "*** pnmdouble_isSameFormat() error - The pointer (*pnm2) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if ((pnm1->desc % PNM_DESCRIPTOR_LENGTH) == (pnm2->desc % PNM_DESCRIPTOR_LENGTH)) {
		return PNM_TRUE;
	} else {
		return PNM_FALSE;
	}
}


int
pnm_isSameDescriptor(PNM *pnm1, PNM *pnm2)
{
	if (pnm1 == NULL) {
		fprintf(stderr, "*** pnm_isSameDescriptor() error - The pointer (*pnm1) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	} else if (pnm2 == NULL) {
		fprintf(stderr, "*** pnm_isSameDescriptor() error - The pointer (*pnm2) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if (pnm1->desc == pnm2->desc) {
		return PNM_TRUE;
	} else {
		return PNM_FALSE;
	}
}


int
pnmdouble_isSameDescriptor(PNM_DOUBLE *pnm1, PNM_DOUBLE *pnm2)
{
	if (pnm1 == NULL) {
		fprintf(stderr, "*** pnmdouble_isSameDescriptor() error - The pointer (*pnm1) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	} else if (pnm2 == NULL) {
		fprintf(stderr, "*** pnmdouble_isSameDescriptor() error - The pointer (*pnm2) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if (pnm1->desc == pnm2->desc) {
		return PNM_TRUE;
	} else {
		return PNM_FALSE;
	}
}


int
pnm_isRGB(PNM *pnm)
{
	if (pnm == NULL) {
		fprintf(stderr, "*** pnm_isRGB() error - The pointer (*pnm) is NULL ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if (pnm->desc % PNM_DESCRIPTOR_PIXMAPS == 0) {
		return PNM_TRUE;
	} else {
		return PNM_FALSE;
	}
}


int
pnmdouble_isRGB(PNM_DOUBLE *pnmd)
{
	if (pnmd == NULL) {
		fprintf(stderr, "*** pnmdouble_isRGB() error - The pointer to PNM is NULL (*pnmd) ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if (pnmd->desc % PNM_DESCRIPTOR_PIXMAPS == 0) {
		return PNM_TRUE;
	} else {
		return PNM_FALSE;
	}
}


int
pnm_RGB2Gray(PNM_DOUBLE *out, PNM_DOUBLE *in)
{
	char *ErrorValueName = "";
	int M = 0;
	int N = 0;
	int i;

	if (out == NULL) {
		ErrorValueName = "out";
		goto ErrorPointerNull;
	} else if (out->imgd != NULL) {
		ErrorValueName = "(out->imgd)";
		goto ErrorPointerNull;
	}
	if (in == NULL) {
		ErrorValueName = "in";
		goto ErrorPointerNull;
	} else if (in->imgd == NULL) {
		ErrorValueName = "(in->imgd)";
		goto ErrorPointerNull;
	}
	M = (int)in->height;
	N = (int)in->width;
	if ((M < 0) || (N < 0)) {
		fprintf(stderr, "*** pnm_RGB2Gray error - The size of image is beyond the upper bound that can handle correctly ***\n");
		return PNM_FUNCTION_ERROR;
	}

	if ((in->desc % 3) != 0) {
		fprintf(stderr, "*** pnm_RGB2Gray warning - The input image is binarymap or graymap ***\n");
		if (in->desc <= PNM_DESCRIPTOR_ASCII_MAX) {
			out->desc = PORTABLE_GRAYMAP_ASCII;
		} else {
			out->desc = PORTABLE_GRAYMAP_BINARY;
		}
		out->width = in->width;
		out->height = in->height;
		out->maxint = in->maxint;
		if ((out->imgd = (pnm_img_double *)calloc((size_t)(M * N), sizeof(pnm_img_double))) == NULL) {
			fprintf(stderr, "*** pnm_RGB2Gray error - Cannot allocate memory for (out->img) ***\n");
			*out = PNM_DOUBLE_NULL;
			return PNM_FUNCTION_ERROR;
		}
		for (i = 0; i < M * N; i++) {
			out->imgd[i] = in->imgd[i];
		}
		fprintf(stderr, "*** The Non-RGB image (*in) has been copied to (*out) ***\n");
		return PNM_FUNCTION_SUCCESS;
	}

	if (in->desc <= PNM_DESCRIPTOR_ASCII_MAX) {
		out->desc = PORTABLE_GRAYMAP_ASCII;
	} else {
		out->desc = PORTABLE_GRAYMAP_BINARY;
	}
	out->width = in->width;
	out->height = in->height;
	out->maxint = in->maxint;
	if ((out->imgd = (pnm_img_double *)calloc((size_t)(M * N), sizeof(pnm_img_double))) == NULL) {
		fprintf(stderr, "*** pnm_RGB2Gray error - Cannot allocate memory for (out->img) ***\n");
		*out = PNM_DOUBLE_NULL;
		return PNM_FUNCTION_ERROR;
	}
	for (i = 0; i < M * N; i++) {
		out->imgd[i] =
		    PNM_YUV_Y_RED * in->imgd[i]
		    + PNM_YUV_Y_GREEN * in->imgd[M * N + i]
		    + PNM_YUV_Y_BLUE * in->imgd[2 * M * N + i];
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnm_RGB2Gray() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
}


int
pnm_Gray2RGB(PNM *out, PNM *in)
{
	char *ErrorValueName = "";
	unsigned int i;

	if (out == NULL) {
		ErrorValueName = "out";
		goto ErrorPointerNull;
	} else if (out->img != NULL) {
		ErrorValueName = "(out->img)";
		goto ErrorNotInitialized;
	}
	if (in == NULL) {
		ErrorValueName = "in";
		goto ErrorPointerNull;
	} else if (in->img == NULL) {
		ErrorValueName = "(in->img)";
		goto ErrorPointerNull;
	} else if (pnm_isRGB(in) == PNM_TRUE) {
		fprintf(stderr, "*** pnm_Gray2RGB error - PNM Descriptor of in is NOT right ***\n");
		return PNM_FUNCTION_ERROR;
	}

	if ((out->img = (pnm_img *)calloc((size_t)(3 * in->width * in->height), sizeof(pnm_img))) == NULL) {
		fprintf(stderr, "*** pnm_Gray2RGB() error - Cannot allocate memory for (out->img) ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if (in->desc <= PNM_DESCRIPTOR_ASCII_MAX) {
		out->desc = PORTABLE_PIXMAP_ASCII;
	} else {
		out->desc = PORTABLE_PIXMAP_BINARY;
	}
	out->width = in->width;
	out->height = in->height;
	out->maxint = in->maxint;
	for (i = 0; i < in->width * in->height; i++) {
		out->img[i] = in->img[i];
		out->img[out->width * out->height + i] = in->img[i];
		out->img[2u * out->width * out->height + i] = in->img[i];
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnm_Gray2RGB() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
ErrorNotInitialized:
	fprintf(stderr, "*** pnm_Gray2RGB() error - The pointer (*%s) is NOT initialized ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
}


int
pnmdouble_Gray2RGB(PNM_DOUBLE *out, PNM_DOUBLE *in)
{
	char *ErrorValueName = "";
	unsigned int i;

	if (out == NULL) {
		ErrorValueName = "out";
		goto ErrorPointerNull;
	} else if (out->imgd != NULL) {
		ErrorValueName = "(out->imgd)";
		goto ErrorNotInitialized;
	}
	if (in == NULL) {
		ErrorValueName = "in";
		goto ErrorPointerNull;
	} else if (in->imgd == NULL) {
		ErrorValueName = "(in->imgd)";
		goto ErrorPointerNull;
	} else if (pnmdouble_isRGB(in) != PNM_FALSE) {
		fprintf(stderr, "*** pnmdouble_Gray2RGB error - PNM Descriptor of (*in) is NOT right ***\n");
		return PNM_FUNCTION_ERROR;
	}

	if ((out->imgd = (pnm_img_double *)calloc((size_t)(3 * in->width * in->height), sizeof(pnm_img_double))) == NULL) {
		fprintf(stderr, "*** pnm_Gray2RGB() error - Cannot allocate memory for (out->imgd) ***\n");
		return PNM_FUNCTION_ERROR;
	}
	if (in->desc <= PNM_DESCRIPTOR_ASCII_MAX) {
		out->desc = PORTABLE_PIXMAP_ASCII;
	} else {
		out->desc = PORTABLE_PIXMAP_BINARY;
	}
	out->width = in->width;
	out->height = in->height;
	out->maxint = in->maxint;
	for (i = 0; i < in->width * in->height; i++) {
		out->imgd[i] = in->imgd[i];
		out->imgd[out->width * out->height + i] = in->imgd[i];
		out->imgd[2u * out->width * out->height + i] = in->imgd[i];
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnmdouble_Gray2RGB() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
ErrorNotInitialized:
	fprintf(stderr, "*** pnmdouble_Gray2RGB() error - The pointer (*%s) is NOT initialized ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
}


int
pnm_RGB2YCbCr(PNM_DOUBLE *out, PNM_DOUBLE *in)
{
	char *ErrorValueName = "";
	int M = 0;
	int N = 0;
	int i;

	if (out == NULL) {
		ErrorValueName = "out";
		goto ErrorPointerNull;
	} else if (out->imgd != NULL) {
		ErrorValueName = "(out->imgd)";
		goto ErrorPointerNull;
	}
	if (in == NULL) {
		ErrorValueName = "in";
		goto ErrorPointerNull;
	} else if (in->imgd == NULL) {
		ErrorValueName = "(in->imgd)";
		goto ErrorPointerNull;
	}
	M = (int)in->height;
	N = (int)in->width;
	if ((M < 0) || (N < 0)) {
		fprintf(stderr, "*** pnm_RGB2YCbCr() error - The size of image is beyond the upper bound that can handle correctly ***\n");
		return PNM_FUNCTION_ERROR;
	}

	if ((in->desc % 3) != 0) {
		fprintf(stderr, "*** pnm_RGB2YCbCr() error - The input image is binary or grayscale ***\n");
		*out = PNM_DOUBLE_NULL;
		return PNM_FUNCTION_ERROR;
	}

	out->desc = PORTABLE_PIXMAP_BINARY;
	out->width = in->width;
	out->height = in->height;
	out->maxint = in->maxint;
	if ((out->imgd = (pnm_img_double *)calloc((size_t)(3 * M * N), sizeof(pnm_img_double))) == NULL) {
		fprintf(stderr, "*** pnm_RGB2YCbCr() error - Cannot allocate memory for (out->img) ***\n");
		*out = PNM_DOUBLE_NULL;
		return PNM_FUNCTION_ERROR;
	}
	for (i = 0; i < M * N; i++) {
		out->imgd[i] =
		    PNM_YUV_Y_RED * in->imgd[i]
		    + PNM_YUV_Y_GREEN * in->imgd[M * N + i]
		    + PNM_YUV_Y_BLUE * in->imgd[2 * M * N + i];
		out->imgd[M*N + i] =
		    PNM_YUV_Cb_RED * in->imgd[i]
		    + PNM_YUV_Cb_GREEN * in->imgd[M * N + i]
		    + PNM_YUV_Cb_BLUE * in->imgd[2 * M * N + i];
		out->imgd[2*M*N + i] =
		    PNM_YUV_Cr_RED * in->imgd[i]
		    + PNM_YUV_Cr_GREEN * in->imgd[M * N + i]
		    + PNM_YUV_Cr_BLUE * in->imgd[2 * M * N + i];
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnm_RGB2YCbCr() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
}


int
pnm_YCbCr2RGB(PNM_DOUBLE *out, PNM_DOUBLE *in)
{
	char *ErrorValueName = "";
	int M = 0;
	int N = 0;
	int i;

	if (out == NULL) {
		ErrorValueName = "out";
		goto ErrorPointerNull;
	} else if (out->imgd != NULL) {
		ErrorValueName = "(out->imgd)";
		goto ErrorPointerNull;
	}
	if (in == NULL) {
		ErrorValueName = "in";
		goto ErrorPointerNull;
	} else if (in->imgd == NULL) {
		ErrorValueName = "(in->imgd)";
		goto ErrorPointerNull;
	}
	M = (int)in->height;
	N = (int)in->width;
	if ((M < 0) || (N < 0)) {
		fprintf(stderr, "*** pnm_RGB2YCbCr error - The size of image is beyond the upper bound that can handle correctly ***\n");
		return PNM_FUNCTION_ERROR;
	}

	if ((in->desc % 3) != 0) {
		fprintf(stderr, "*** pnm_RGB2YCbCr error - The input image is binary or grayscale ***\n");
		*out = PNM_DOUBLE_NULL;
		return PNM_FUNCTION_ERROR;
	}

	out->width = in->width;
	out->height = in->height;
	out->maxint = in->maxint;
	if ((out->imgd = (pnm_img_double *)calloc((size_t)(3 * M * N), sizeof(pnm_img_double))) == NULL) {
		fprintf(stderr, "*** pnm_RGB2YCbCr error - Cannot allocate memory for (out->img) ***\n");
		*out = PNM_DOUBLE_NULL;
		return PNM_FUNCTION_ERROR;
	}
	for (i = 0; i < M * N; i++) {
		out->imgd[i] =
		    PNM_RGB_RED_Y * in->imgd[i]
		    + PNM_RGB_RED_Cb * in->imgd[M * N + i]
		    + PNM_RGB_RED_Cr * in->imgd[2 * M * N + i];
		out->imgd[M*N + i] =
		    PNM_RGB_GREEN_Y * in->imgd[i]
		    + PNM_RGB_GREEN_Cb * in->imgd[M * N + i]
		    + PNM_RGB_GREEN_Cr * in->imgd[2 * M * N + i];
		out->imgd[2*M*N + i] =
		    PNM_RGB_BLUE_Y * in->imgd[i]
		    + PNM_RGB_BLUE_Cb * in->imgd[M * N + i]
		    + PNM_RGB_BLUE_Cr * in->imgd[2 * M * N + i];
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorPointerNull:
	fprintf(stderr, "*** pnm_YCbCr2RGB() error - The pointer (*%s) is NULL ***\n", ErrorValueName);
	return PNM_FUNCTION_ERROR;
}


int
pnm_resize(PNM_DOUBLE *pnm_out, PNM_DOUBLE *pnm_in, unsigned int width_o, unsigned int height_o, const char *Method)
{
	char *FunctionName = "pnm_resize";
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";
	int ErrorValue = 0;

	char *MethodList[NUM_RESIZE_METHOD] ={"z-hold", "bicubic"};
	double *Tmp = NULL;
	int method_num = 0;
	double alpha;
	unsigned int M, N;
	double scale_x = .0;
	double scale_y = .0;
	unsigned int area_x;
	unsigned int area_y;
	unsigned int m, n;
	unsigned int x, y;
	double sum;

	if (pnm_in == NULL) {
		ErrorValueName = "pnm_in";
		goto ErrorPointerNull;
	} else if (pnm_out->imgd != NULL) {
		ErrorValueName = "*(pnm_out->imgd)";
		goto ErrorNotInitialized;
	} else if (width_o == 0) {
		ErrorValueName = "width_o";
		ErrorValue = width_o;
		goto ErrorIncorrectValue;
	} else if (height_o == 0) {
		ErrorValueName = "height_o";
		ErrorValue = height_o;
		goto ErrorIncorrectValue;
	} else if (Method == NULL) {
		ErrorValueName = "Method";
		goto ErrorPointerNull;
	}

	for (method_num = 0; method_num < NUM_RESIZE_METHOD; method_num++) {
		if (strcmp(Method, MethodList[method_num]) == 0) {
			break;
		}
	}
	N = pnm_in->width;
	M = pnm_in->height;
	scale_x = (double)width_o / pnm_in->width;
	scale_y = (double)height_o / pnm_in->height;
	switch (method_num) {
		case 0: /* Zero-Order Hold (Shrink with mean filter) */
			if (pnmdouble_new(pnm_out, pnm_in->desc, width_o, height_o, pnm_in->maxint) != PNM_FUNCTION_SUCCESS) {
				ErrorFunctionName = "pnmdouble_new";
				ErrorValueName = "pnm_out";
				goto ErrorMalloc;
			}
			area_x = ceil((double)N / width_o);
			area_y = ceil((double)M / height_o);
			for (y = 0; y < height_o; y++) {
				for (x = 0; x < width_o; x++) {
					sum = .0;
					for (m = 0; m < area_y; m++) {
						for (n = 0; n< area_x; n++) {
							sum += (double)pnm_in->imgd[N * ((int)floor(y / scale_y) + m) + (int)floor(x / scale_x) + n];
						}
					}
					pnm_out->imgd[width_o * y + x] = sum / (area_x * area_y);
					if (pnm_in->desc % PNM_DESCRIPTOR_PIXMAPS == 0) {
						for (m = 0; m < area_y; m++) {
							for (n = 0; n< area_x; n++) {
								sum += pnm_in->imgd[M * N + N * ((int)floor(y / scale_y) + m) + (int)floor(x / scale_x) + n];
							}
						}
						pnm_out->imgd[height_o * width_o + width_o * y + x] = sum / (area_x * area_y);
						for (m = 0; m < area_y; m++) {
							for (n = 0; n< area_x; n++) {
								sum += pnm_in->imgd[2 * M * N + N * ((int)floor(y / scale_y) + m) + (int)floor(x / scale_x) + n];
							}
						}
						pnm_out->imgd[2 * height_o * width_o + width_o * y + x] = sum / (area_x * area_y);
					}
				}
			}
			break;
		default: /* Default method is bicubic */
		case 1: /* Bicubic (alpha = -0.5) */
			alpha = -0.5;
			if (pnm_Bicubic(pnm_out, pnm_in, alpha, width_o, height_o) != PNM_FUNCTION_SUCCESS) {
				ErrorFunctionName = "pnm_Bicubic";
				ErrorValueName = "(pnm_in -> pnm_out)";
				goto ErrorFunctionFailed;
			}
	}
	if (pnm_in->desc % PNM_DESCRIPTOR_PIXMAPS == 0){
		for (x = 0; x < 3 * pnm_out->width * pnm_out->height; x++) {
			if (pnm_out->imgd[x] < .0) {
				pnm_out->imgd[x] = .0;
			} else if (pnm_out->imgd[x] >= pnm_out->maxint) {
				pnm_out->imgd[x] = (double)pnm_out->maxint;
			}
		}
	} else {
		for (x = 0; x < pnm_out->width * pnm_out->height; x++) {
			if (pnm_out->imgd[x] < .0) {
				pnm_out->imgd[x] = .0;
			} else if (pnm_out->imgd[x] >= pnm_out->maxint) {
				pnm_out->imgd[x] = (double)pnm_out->maxint;
			}
		}
	}
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorMalloc:
	fprintf(stderr, "*** %s() error - Cannot allocate memory for (*%s) by %s() ***\n", FunctionName, ErrorValueName, ErrorFunctionName);
	goto ErrorReturn;
ErrorPointerNull:
	fprintf(stderr, "*** %s() error - The pointer (*%s) is NULL ***\n", FunctionName, ErrorValueName);
	goto ErrorReturn;
ErrorNotInitialized:
	fprintf(stderr, "*** %s() error - The variable (%s) is NOT initialized ***\n", FunctionName, ErrorValueName);
	goto ErrorReturn;
ErrorIncorrectValue:
	fprintf(stderr, "*** %s() error - The variable (*%s) is out of bound or incorrect value (%d) ***\n", FunctionName, ErrorValueName, ErrorValue);
	goto ErrorReturn;
ErrorFunctionFailed:
	fprintf(stderr, "*** %s() error - %s() failed to compute (%s) ***\n", FunctionName, ErrorFunctionName, ErrorValueName);
ErrorReturn:
	pnmdouble_free(pnm_out);
	free(Tmp);
	return PNM_FUNCTION_ERROR;
}


int
pnm_Bicubic(PNM_DOUBLE *pnm_out, PNM_DOUBLE *pnm_in, double alpha, unsigned int width_o, unsigned int height_o)
{
	char *FunctionName = "pnm_Bicubic";
	char *ErrorFunctionName = "";
	char *ErrorValueName = "";

	double *Tmp = NULL;
	double *conv = NULL;
	double scale_x, scale_y;
	double scale_conv;
	int L, L_center;
	double dx, dy;
	int x, y;
	int m, n;
	int index;

	if (pnm_in == NULL) {
		ErrorValueName = "pnm_in";
		goto ErrorPointerNull;
	}

	scale_x = (double)width_o / pnm_in->width;
	scale_y = (double)height_o / pnm_in->height;
	if (pnm_in->desc % PNM_DESCRIPTOR_PIXMAPS == 0) {
		if ((Tmp = (double *)calloc((size_t)width_o * pnm_in->height * 3u, sizeof(double))) == NULL) {
			ErrorFunctionName = "calloc";
			ErrorValueName = "Tmp";
			goto ErrorMalloc;
		}
	} else {
		if ((Tmp = (double *)calloc((size_t)width_o * pnm_in->height, sizeof(double))) == NULL) {
			ErrorFunctionName = "calloc";
			ErrorValueName = "Tmp";
			goto ErrorMalloc;
		}
	}
	/* The length of cubic convolution coefficient */
	scale_conv = 1.0;
	if (scale_x < 1.0 || scale_y < 1.0) {
		scale_conv = ceil(1.0 / (scale_x < scale_y ? scale_x : scale_y));
	}
	if ((conv = (double *)calloc((size_t)scale_conv * 4, sizeof(double))) == NULL) {
		ErrorFunctionName = "calloc";
		ErrorValueName = "conv";
		goto ErrorMalloc;
	}
	if (pnmdouble_new(pnm_out, pnm_in->desc, width_o, height_o, pnm_in->maxint) != PNM_FUNCTION_SUCCESS) {
		ErrorFunctionName = "pnmdouble_new";
		ErrorValueName = "pnm_out";
		goto ErrorFunctionFailed;
	}

	/* Horizontal convolution */
	for (x = 0; (unsigned int)x < width_o; x++) {
		if (scale_x >= 1.0) {
			scale_conv = 1;
			dx = (x - (scale_x - 1.0) / 2.0) / scale_x;
		} else {
			scale_conv = 1.0 / scale_x;
			dx = x / scale_x + (1.0 / scale_x - 1.0) / 2.0;
		}
		L = 4 * (int)ceil(scale_conv);
		L_center = floor((L - 1.0) / 2);
		for (n = 0; n < L; n++) {
			conv[n] = pnm_Cubic(((double)(n - L_center) - (dx - floor(dx))) / scale_conv, alpha);
			conv[n] /= scale_conv;
		}
		for (y = 0; (unsigned int)y < pnm_in->height; y++) {
			Tmp[width_o * y + x] = .0;
			if (pnm_in->desc % PNM_DESCRIPTOR_PIXMAPS == 0) {
				Tmp[width_o * pnm_in->height + width_o * y + x] = .0;
				Tmp[2 * width_o * pnm_in->height + width_o * y + x] = .0;
			}
			for (n = 0; n < L; n++) {
				index = (int)floor(dx) + n - L_center;
				if (index < 0) {
					index = abs(index) - 1;
				} else if (index >= (int)pnm_in->width) {
					index = 2 * pnm_in->width - 1 - index;
				}
				Tmp[width_o * y + x] += conv[n] * pnm_in->imgd[pnm_in->width * y + index];
				if (pnm_in->desc % PNM_DESCRIPTOR_PIXMAPS == 0) {
					Tmp[width_o * pnm_in->height + width_o * y + x] += conv[n] * pnm_in->imgd[pnm_in->width * pnm_in->height + pnm_in->width * y + index];
					Tmp[2 * width_o * pnm_in->height + width_o * y + x] += conv[n] * pnm_in->imgd[2 * pnm_in->width * pnm_in->height + pnm_in->width * y + index];
				}
			}
		}
	}
	/* Vertical convolution */
	for (y = 0; (unsigned int)y < height_o; y++) {
		if (scale_y >= 1.0) {
			scale_conv = 1;
			dy = (y - (scale_y - 1.0) / 2.0) / scale_y;
		} else {
			scale_conv = 1.0 / scale_y;
			dy = y / scale_y + (1.0 / scale_y - 1.0) / 2.0;
		}
		L = 4 * (int)ceil(scale_conv);
		L_center = floor((L - 1.0) / 2);
		for (m = 0; m < L; m++) {
			conv[m] = pnm_Cubic(((double)(m - L_center) - (dy - floor(dy))) / scale_conv, alpha);
			conv[m] /= scale_conv;
		}
		for (x = 0; (unsigned int)x < width_o; x++) {
			pnm_out->imgd[width_o * y + x] = .0;
			if (pnm_in->desc % PNM_DESCRIPTOR_PIXMAPS == 0) {
				pnm_out->imgd[width_o * height_o + width_o * y + x] = .0;
				pnm_out->imgd[2 * width_o * height_o + width_o * y + x] = .0;
			}
			for (m = 0; m < L; m++) {
				index = (int)floor(dy) + m - L_center;
				if (index < 0) {
					index = abs(index) - 1;
				} else if (index >= (int)pnm_in->height) {
					index = 2 * pnm_in->height - 1 - index;
				}
				pnm_out->imgd[width_o * y + x] += conv[m] * Tmp[width_o * index + x];
				if (pnm_in->desc % PNM_DESCRIPTOR_PIXMAPS == 0) {
					pnm_out->imgd[width_o * height_o + width_o * y + x] += conv[m] * Tmp[width_o * pnm_in->height + width_o * index + x];
					pnm_out->imgd[2 * width_o * height_o + width_o * y + x] += conv[m] * Tmp[2 * width_o * pnm_in->height + width_o * index + x];
				}
			}
		}
	}
	free(conv);
	free(Tmp);
	return PNM_FUNCTION_SUCCESS;
/* Error */
ErrorMalloc:
	fprintf(stderr, "*** %s() error - Cannot allocate memory for (*%s) by %s() ***\n", FunctionName, ErrorValueName, ErrorFunctionName);
	goto ErrorReturn;
ErrorPointerNull:
	fprintf(stderr, "*** %s() error - The pointer (*%s) is NULL ***\n", FunctionName, ErrorValueName);
	goto ErrorReturn;
ErrorFunctionFailed:
	fprintf(stderr, "*** %s() error - %s() failed to compute (%s) ***\n", FunctionName, ErrorFunctionName, ErrorValueName);
ErrorReturn:
	pnmdouble_free(pnm_out);
	free(conv);
	free(Tmp);
	return PNM_FUNCTION_ERROR;
}


double
pnm_Cubic(double x, double a)
{
	double x_abs = fabs(x);

	if (x_abs <= 1.0) {
		return ((a + 2.0) * x_abs - (a + 3.0)) * x_abs * x_abs + 1.0;
	} else if (x_abs < 2.0) {
		return ((a * x_abs - 5.0 * a) * x_abs + 8.0 * a) * x_abs - 4.0 * a;
	} else {
		return 0;
	}
}


int *
pnm2int(PNM *pnm)
{
	int *Image = NULL;
	unsigned int n;

	if ((Image = (int *)calloc((size_t)pnm->width * pnm->height, sizeof(int))) == NULL) {
		fprintf(stderr, "*** pnm2int() error - Cannot allocate memory for (*Image) by calloc() ***\n");
		return NULL;
	}
	for (n = 0; n < pnm->width * pnm->height; n++) {
		Image[n] = pnm->img[n];
	}
	return Image;
}


double *
pnm2double(PNM *pnm)
{
	double *Image = NULL;
	unsigned int n;

	if ((Image = (double *)calloc((size_t)pnm->width * pnm->height, sizeof(double))) == NULL) {
		fprintf(stderr, "*** pnm2double() error - Cannot allocate memory for (*Image) by calloc() ***\n");
		return NULL;
	}
	for (n = 0; n < pnm->width * pnm->height; n++) {
		Image[n] = (double)pnm->img[n];
	}
	return Image;
}


double *
pnmdouble2double(PNM_DOUBLE *pnmd)
{
	double *Image = NULL;
	unsigned int n;

	if ((Image = (double *)calloc((size_t)pnmd->width * pnmd->height, sizeof(double))) == NULL) {
		fprintf(stderr, "*** pnmdouble2double() error - Cannot allocate memory for (*Image) by calloc() ***\n");
		return NULL;
	}
	for (n = 0; n < pnmd->width * pnmd->height; n++) {
		Image[n] = pnmd->imgd[n];
	}
	return Image;
}

