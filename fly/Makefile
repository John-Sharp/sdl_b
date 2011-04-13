CC = gcc

CFLAGS = -lSDL_image -lGL -lm `sdl-config --cflags --libs`				

fly: fly.c
	$(CC) fly.c -g -o fly \
	    $(CFLAGS)

clean:
	rm -f *.o bouncer 
