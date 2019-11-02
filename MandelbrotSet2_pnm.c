#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#if defined linux
#	include <bsd/string.h>
#else
#	include <string.h>
#endif
#include "pnm.h"

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


int Mandelbrot(uint16_t *img, int M, int N, int BPS, int BaseM, int BaseN, unsigned int TimeMax);
Complex c_plus(Complex z1, Complex z2);
Complex c_mult(Complex z1, Complex z2);
double c_abs(Complex z);
inline double c_abs_p2(Complex z);
void writeimage(char *filename, uint16_t *Img_s, int M, int N, int BPS);



int
main(int argc, char *argv[])
{
	char help[] =
	    " MandelbrotSet - Make Mandelbrot Set Picture -\n"
	    "\n"
	    " -h, --help        Show command list\n"
	    " -o FILENAME       Specify output filename\n"
	    " -8b               Set 8-bit depth output\n"
	    "                   Default output bit depth is 16 bits\n"
	    " VAR=NUM           Set some variable (M, N, TIME) as NUM\n"
	    "                        M    : picture height\n"
	    "                        N    : picture width\n"
	    "                        TIME : Maximum time count for checking diverge or not\n"
	    "                               (default: %d)\n"
	    "\n"
	    " *** Examples ***\n"
	    "\n"
	    " * To make picture as \"out.pgm\" :\n"
	    "      MandelbrotSet -o out\n"
	    "\n"
	    " * To make picture of 8-bit 4096 pel width :\n"
	    "      MandelbrotSet -8b N=4096\n\n";
	char filename[FILENAME_LENGTH/2] = "MandelbrotSet2";
	char *ptr;
	uint16_t *img;
	unsigned int TimeMax = DEFAULT_TIMEMAX;
	int M = 0;
	int N = 0;
	int BPS = 16;
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
				}
			}
		}
	}
	if ((M <= 0) && (N > 0)) {
		M = ceil(N * 2.0 / 3.0);
	} else if ((M > 0) && (N <= 0)) {
		N = ceil(M * 3.0 / 2.0);
	} else if ((M <= 0) && (N <= 0)) {
		M = DEFAULT_M;
		N = DEFAULT_N;
	}
	if (TimeMax == 0) {
		TimeMax = DEFAULT_TIMEMAX;
	}
	printf("Allocating Memory space for Image data...   ");
	if ((img = (uint16_t *)calloc(M*N, sizeof(uint16_t))) == 0) {
		fprintf(stderr, "calloc error in main\n");
	}
	printf("Complete!\nCompute Mandelbrot Set...   ");
	Mandelbrot(img, M, N, BPS, ceil(M / 2.0), ceil(N * 2.0 / 3.0), TimeMax);
	printf("Complete!\n");
	writeimage(filename, img, M, N, BPS);
	printf("Saved to \"MandelbrotSet_%s.pgm\"\n", filename);
	return EXIT_SUCCESS;
}


int
Mandelbrot(uint16_t *img, int M, int N, int BPS, int BaseM, int BaseN, unsigned int TimeMax)
{
	Complex c, zn, znp1;
	unsigned int i;
	int m, n, val;
	double Mmax = M - BaseM;
	double Nmax = N - BaseN;
	double max = 0;
	double *arr;

	if (BPS == 8) {
		val = 0xff;
	} else {
		val = 0xffff;
	}
	if ((arr = calloc(M*N, sizeof(double))) == 0) {
		fprintf(stderr,"calloc error on *arr\n");
		exit(EXIT_FAILURE);
	}

	#pragma omp parallel for private(n, c, zn, znp1, i)
	for (m=0; m<M; m++) {
		for (n=0; n<N; n++) {
			zn = (Complex){0, 0};
			c = (Complex){(n - BaseN)/Nmax, (BaseM - m)/Mmax};
			img[N*m + n] = val;
			for (i=0; i < TimeMax; i++) {
				znp1 = c_plus(c_mult(zn, zn), c);
				if (c_abs_p2(znp1) > MAX_VAL) {
					arr[N*m + n] = i + 1.0/(c_abs(znp1) - 1.0);
					//arr[N*m + n] = i;
					break;
				}
				zn = znp1;
			}
		}
	}
	for (m=0; m < M*N; m++) {
		if (max < arr[m]) {
			max = arr[m];
		}
	}
	max = 1.0 / max;
	#pragma omp parallel for
	for (m=0; m < M*N; m++) {
		img[m] += val * pow(max * arr[m], 0.18);
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
	return (Complex){
	    z1.r * z2.r - (z1.i * z2.i),
	    z1.r * z2.i + (z1.i * z2.r)
	    };
}


double
c_abs(Complex z)
{
	return sqrt((z.r * z.r) + (z.i * z.i));
}


inline double
c_abs_p2(Complex z)
{
	return (z.r * z.r) + (z.i * z.i);
}


void
writeimage(char *filename, uint16_t *img, int M, int N, int BPS)
{
	char filename_out[FILENAME_LENGTH];
	PNM pnm = PNM_NULL;
	int i;

	if (BPS != 8) {
		BPS = 16;
	}
	if (pnmnew(&pnm, PORTABLE_GRAYMAP_BINARY, N, M, (0xFFFF >> (16 - BPS))) == PNM_FUNCTION_ERROR) {
		fprintf(stderr, "writeimage error - Cannot Allocate Memory for output image -\n");
		exit(EXIT_FAILURE);
	}
	for (i=0; i < M*N; i++) {
		pnm.img[i] = (int)img[i];
	}
	printf("ok\n");
	strlcpy(filename_out, filename, FILENAME_LENGTH);
	strlcat(filename_out, ".pgm", FILENAME_LENGTH);
	pnmwrite(&pnm, filename_out);
	printf("\n ***** Write to the File \"%s\" *****\n\n", filename_out);
	pnmfree(&pnm);
}

