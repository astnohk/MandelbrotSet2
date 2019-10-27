CC=gcc
WARNING=-Wall -Wextra
LIBES=-lm -lbsd -lX11 -ltiff
OPTION=-O2 -fopenmp
MACROS=


MAIN_CFILES = MandelbrotSet2_pnm.c MandelbrotSet2_tiff.c pnm.c

CFILES = $(MAIN_CFILES)


OFILES_PNM = MandelbrotSet2_pnm.o pnm.o
OFILES_TIFF = MandelbrotSet2_tiff.o pnm.o
OFILES = MandelbrotSet2_pnm.o MandelbrotSet2_tiff.o pnm.o

OUTNAME_PNM = MandelbrotSet2_pnm
OUTNAME_TIFF = MandelbrotSet2_tiff




MandelbrotSet2: $(OFILES_PNM)
	$(CC) $(WARNING) $(LIBES) $(OPTION) -o $(OUTNAME_PNM) $(OFILES_PNM)
	$(CC) $(WARNING) $(LIBES) $(OPTION) -o $(OUTNAME_TIFF) $(OFILES_TIFF)


MandelbrotSet2_pnm.o: MandelbrotSet2_pnm.c
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

MandelbrotSet2_tiff.o: MandelbrotSet2_tiff.c
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^

pnm.o: pnm.c
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^



debug: $(CFILES)
	$(CC) $(WARNING) $(LIBES) -g -O2 $(MACROS) -o $(OUTNAME) $^

debugmp: $(CFILES)
	$(CC) $(WARNING) $(LIBES) $(OPTION) $(MACROS) -g -O2 -o $(OUTNAME) $^

clean:
	rm -f $(OFILES)
	find -name "*.gch" -exec rm {} +

