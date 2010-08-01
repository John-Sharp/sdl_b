CC = gcc

CFLAGS = -Wall -lSDL_image -lm				

bouncer: bouncer.c ../jaunty/jmap.c ../jaunty/jmap.h ../jaunty/jactor.c \
	../jaunty/jactor.h ../jaunty/jutils.c ../jaunty/jutils.h \
	../jaunty/jen.h ../jaunty/jen.c ../jaunty/jdirty.h ../jaunty/jdirty.c
	$(CC) bouncer.c ../jaunty/jmap.c ../jaunty/jactor.c \
		../jaunty/jen.c  ../jaunty/jutils.c ../jaunty/jdirty.c \
	   	-g -o bouncer \
	    $(CFLAGS) `sdl-config --cflags --libs` 

clean:
	rm -f *.o bouncer 
