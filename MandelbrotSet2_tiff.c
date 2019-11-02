#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <tiffio.h>
#if defined linux
#	include <bsd/string.h>
#else
#	include <string.h>
#endif

/*
 * M : Vertical coordinate
 * N : Horizontal coordinate
 */
#define DEFAULT_M 800
#define DEFAULT_N 1200
#define DEFAULT_TIMEMAX 16000
#define MAX_VAL 4 // 2^2
#define FILENAME_LENGTH 512

typedef struct {double r; double i;} Complex;


int writeimage(char *filename, uint16 *Img_s, int M, int N, int BPS);
int Mandelbrot(uint16_t *img, int M, int N, int BPS, int BaseM, int BaseN, double Scale, unsigned int TimeMax);
Complex c_plus(Complex z1, Complex z2);
Complex c_mult(Complex z1, Complex z2);
double c_abs(Complex z);
double c_abs_p2(Complex z);



int
main(int argc, char *argv[])
{
	char help[] =
" MandelbrotSet - Make Mandelbrot Set Picture -\n"
"\n"
" -h, --help        Show command list\n"
" -o FILENAME       Specify output filename\n"
" -8b               Set 8-bit tiff output\n"
"                   Default tiff output is 16 bits\n"
" VAR=NUM           Set some variable (M, N, TIME) as NUM\n"
"                        M    : Picture height\n"
"                        N    : Picture width\n"
"                        TIME : Maximum time count for checking diverge or not\n"
"                               (default: %d)\n"
"                        SCALE: Scale of image to Gaussian plane(default: 1.0)\n"
"\n"
" *** Examples ***\n"
"\n"
" * To make picture as \"outtif.tif\" :\n"
"      MandelbrotSet -o outtif\n"
"\n"
" * To make picture of 8-bit 4096 pel width :\n"
"      MandelbrotSet -8b N=4096\n\n";
	char filename[256] = "MandelbrotSet2";
	char *ptr;
	uint16_t *img;
	unsigned int TimeMax = DEFAULT_TIMEMAX;
	int M = 0, N = 0, BPS = 16;
	double Scale = 1.0;
	int i;

	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)) {
				printf(help, DEFAULT_TIMEMAX);
				exit(EXIT_SUCCESS);
			} else if (strcmp(argv[i], "-o") == 0) {
				i++;
				strlcpy(filename, argv[i], FILENAME_LENGTH/2);
			} else if (strcmp(argv[i], "-8b") == 0) {
				BPS = 8;
				printf("Set bit depth %d-bit\n", BPS);
			} else if ((ptr = strchr(argv[i], '=')) != NULL) {
				if (argv[i][0] == 'M') {
					sscanf(ptr + 1, "%7d", &M);
					printf("picture height = %d\n", M);
				} else if (argv[i][0] == 'N') {
					sscanf(ptr + 1, "%7d", &N);
					printf("picture width = %d\n", N);
				} else if (strncmp(argv[i], "TIME", 4) == 0) {
					sscanf(ptr + 1, "%10u", &TimeMax);
					printf("time count max = %d\n", TimeMax);
				} else if (strncmp(argv[i], "SCALE", 4) == 0) {
					sscanf(ptr + 1, "%lf", &Scale);
					printf("Scale = %f\n", Scale);
				}
			}
		}
	}
	if ((M <= 0) && (N > 0)) {
		M = ceil(N * 2.0 / 3.0);
	} else if ((M > 0) && (N <= 0)) {
		N = ceil(M * 3.0 / 2.0);
	} else if ((M <= 0) && (N <= 0)) {
		printf("%d <= 0 && %d <= 0\n", M, N);
		M = DEFAULT_M;
		N = DEFAULT_N;
	}
	printf("W%d x H%d x %dbit\n", N, M, BPS);
	if (TimeMax == 0) {
		TimeMax = DEFAULT_TIMEMAX;
	}
	printf("Allocating Memory space for Image data...   ");
	if ((img = (uint16 *)calloc(M * N, sizeof(uint16))) == NULL) {
		fprintf(stderr, "calloc error in main\n");
	}
	printf("Complete!\nCompute Mandelbrot Set...\n");
	Mandelbrot(img, M, N, BPS, ceil(M / 2.0), ceil(N * 2.0 / 3.0), Scale, TimeMax);
	printf("Complete!\n");
	writeimage(filename, img, M, N, BPS);
	printf("Saved to \"MandelbrotSet_%s.tif\"\n", filename);
	return EXIT_SUCCESS;
}


int
Mandelbrot(uint16_t *img, int M, int N, int BPS, int BaseM, int BaseN, double Scale, unsigned int TimeMax)
{
	unsigned int timeStep = 32;
	Complex c, zn, znp1;
	int m, n, val;
	unsigned int t, dt;
	double Mmax = M - BaseM;
	double Nmax = N - BaseN;
	double MNmax = (Mmax > Nmax ? Mmax : Nmax) * Scale;
	double max = 0;
	Complex *arr;

	if (BPS == 8) {
		val = 0xff;
	} else {
		val = 0xffff;
	}
	if ((arr = calloc(M * N, sizeof(Complex))) == NULL) {
		fprintf(stderr,"calloc error on *arr\n");
		exit(EXIT_FAILURE);
	}
	for (m = 0; m < M * N; m++) {
		arr[m] = (Complex){0, 0};
	}

	for (t = 0; t < TimeMax; t += timeStep) {
#pragma omp parallel for private(n, dt, c, zn, znp1)
		for (m = 0; m < M; m++) {
			for (n = 0; n < N; n++) {
				for (dt = 0; dt < timeStep && img[N * m + n] == 0; dt++) {
					zn = arr[N * m + n];
					c = (Complex){(n - BaseN) / MNmax, (BaseM - m) / MNmax};
					znp1 = c_plus(c_mult(zn, zn), c);
					arr[N * m + n] = znp1;
					if (c_abs_p2(znp1) > MAX_VAL) {
						arr[N * m + n] = (Complex){t + dt, 0};
						img[N * m + n] = (uint16_t)val; // End flag
					}
				}
			}
		}
		printf("%6d / %6d (timeStep: %d)\x1b[0E", t, TimeMax, timeStep);
	}
	printf("\n");
	for (m = 0; m < M * N; m++) {
		if (img[m] == 0) {
			arr[m] = (Complex){TimeMax, 0};
		}
		if (arr[m].r > max) {
			max = arr[m].r;
		}
	}
	#pragma omp parallel for
	for (m = 0; m < M * N; m++) {
		img[m] = (uint16_t)(val * pow(arr[m].r / max, 0.18));
	}
	free(arr);
	return 0;
}


Complex
c_plus(Complex z1, Complex z2)
{
	return (Complex){z1.r + z2.r, z1.i + z2.i};
}


Complex
c_mult(Complex z1, Complex z2)
{
	return (Complex){z1.r * z2.r - z1.i * z2.i, z1.r * z2.i + z1.i * z2.r};
}


double
c_abs(Complex z)
{
	return sqrt(z.r * z.r + z.i * z.i);
}


double
c_abs_p2(Complex z)
{
	return z.r * z.r + z.i * z.i;
}


int
writeimage(char *filename, uint16 *Img_s, int M, int N, int BPS)
{
	uint8 *img8 = 0;
	char filename_out[300];
	TIFF *img = 0;

	if (BPS != 8 && BPS != 16) {
		BPS = 16;
	}
	if (BPS == 8) {
		int i;
		if ((img8 = (uint8 *)calloc(M * N, sizeof(uint8))) == NULL) {
			fprintf(stderr, "calloc Error in writeimage\n");
			exit(EXIT_FAILURE);
		}
		for (i = 0; i < M * N; i++) {
			img8[i] = Img_s[i];
		}
	}
	strcpy(filename_out, filename);
	strcat(filename_out, ".tif");
	if ((img = TIFFOpen(filename_out, "w")) == NULL) {
		fprintf(stderr,"TIFFOpen Error in writeimage\n");
		exit(EXIT_FAILURE);
	}
	TIFFSetField(img, TIFFTAG_IMAGEWIDTH, N);
	TIFFSetField(img, TIFFTAG_IMAGELENGTH, M);
	TIFFSetField(img, TIFFTAG_BITSPERSAMPLE, BPS);
	TIFFSetField(img, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(img, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(img, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	TIFFSetField(img, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
	TIFFSetField(img, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(img, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	printf("%d x %d x %d\n", N, M, BPS);
	if (BPS == 8) {
		TIFFWriteEncodedStrip(img, 0, img8, M * N);
	} else {
		TIFFWriteEncodedStrip(img, 0, Img_s, M * N * 2);
	}
	TIFFClose(img);
	printf("\n ***** Write to the File \"%s\" *****\n\n", filename_out);
	return 0;
}

