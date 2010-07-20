CC = gcc

CFLAGS = -Wall -lSDL_image -lm				

bouncer: bouncer.c ../jaunty/jmap.c ../jaunty/jmap.h ../jaunty/jactor.c \
	../jaunty/jactor.h ../jaunty/jutils.c ../jaunty/jutils.h
	$(CC) bouncer.c ../jaunty/jmap.c ../jaunty/jactor.c \
		../jaunty/jutils.c -g -o bouncer $(CFLAGS) \
		`sdl-config --cflags --libs` 

clean:
	rm -f *.o bouncer 
