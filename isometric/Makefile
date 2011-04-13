CC = gcc

CFLAGS = -pg -g -Wall -lSDL_image -lm -lGL `sdl-config --cflags --libs`				

isomap: isomap_harness.c isomap.c isomap.h
	$(CC) isomap_harness.c isomap.c $(CFLAGS) -o isomap


isoeng: isoeng_harness.c isoeng.c isoeng.h
	$(CC) isoeng_harness.c isoeng.c $(CFLAGS) -o isoeng


clean:
	rm -f *.o isomap isoeng
