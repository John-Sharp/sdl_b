CC = gcc

CFLAGS = -pg -g -Wall -lSDL_image -lm -lGL `sdl-config --cflags --libs`				
random_sheeps: random_sheeps.c ../isometric/isoeng.c ../isometric/isoeng.h \
	../isometric/isoactor.c ../isometric/isoactor.h\
	../isometric/isomap.c ../isometric/isomap.h
	$(CC) random_sheeps.c ../isometric/isoeng.c ../isometric/isoactor.c\
	   ../isometric/isomap.c -o random_sheeps $(CFLAGS) 


clean:
	rm -f *.o random_sheeps
