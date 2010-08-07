CC = gcc

CFLAGS = -Wall -lSDL_image -lm				

revolver: revolver.c 
	$(CC) revolver.c -g -o revolver \
	    $(CFLAGS) `sdl-config --cflags --libs` 

clean:
	rm -f *.o revolver 
