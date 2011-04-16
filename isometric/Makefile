CC = gcc

CFLAGS = -pg -g -Wall -lSDL_image -lm -lGL `sdl-config --cflags --libs`				

isomap: isomap_harness.c isomap.c isomap.h
	$(CC) isomap_harness.c isomap.c $(CFLAGS) -o isomap


isoeng: isoeng_harness.c isoeng.c isoeng.h \
	isoactor.c isoactor.h\
	isomap.c isomap.h
	$(CC) isoeng_harness.c isoeng.c isoactor.c\
	   isomap.c	$(CFLAGS) -o isoeng


clean:
	rm -f *.o isomap isoeng
