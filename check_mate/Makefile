CC = gcc

CFLAGS = -Wall -lSDL_image -lGL -lGLU -lm `sdl-config --cflags --libs` 

check_mate: check_mate.c
	$(CC) check_mate.c -g -o check_mate \
	    $(CFLAGS) 


image_mate: image_mate.c
	$(CC) image_mate.c -g -o image_mate \
	    $(CFLAGS) 
drawing_test: drawing_test.c
	$(CC) drawing_test.c -g -o drawing_test \
	    $(CFLAGS) 

gl_sprite: gl_sprite.c
	$(CC) gl_sprite.c -g -o gl_sprite\
	    $(CFLAGS) 



clean:
	rm -f *.o check_mate 
