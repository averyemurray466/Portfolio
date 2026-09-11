CC = gcc
CFLAGS = -Wall -std=c99 -g -D_GNU_SOURCE

snark: snark.o archive.o compress.o error.o IO.o buffer.o

snark.o: snark.c archive.h error.h 
archive.o: archive.c archive.h compress.h IO.h error.h
IO.o: IO.c IO.h buffer.h
compress.o: compress.c compress.h buffer.h
buffer.o: buffer.c buffer.h
error.o: error.c error.h

clean:
	rm -f buffer.o IO.o compress.o error.o archive.o snark.o stdout.txt stderr.txt
	rm -f snark

