CC = gcc

CFLAGS = -pg -Wall -lSDL_image -lm -lGL `pkg-config --cflags --libs pangocairo` `sdl-config --cflags --libs`				

bouncer: bouncer.c ../jaunty/jmap.c ../jaunty/jmap.h ../jaunty/jactor.c \
	 ../jaunty/jutils.c ../jaunty/jutils.h \
	../jaunty/jen.h ../jaunty/jactor.h ../jaunty/jen.c ../jaunty/jdirty.h\
   	../jaunty/jdirty.c ../jaunty/jplayer.h ../jaunty/jplayer.c \
	text_jactor.c text_jactor.h
	$(CC) bouncer.c ../jaunty/jen.c ../jaunty/jmap.c ../jaunty/jactor.c \
		  ../jaunty/jutils.c ../jaunty/jdirty.c ../jaunty/jplayer.c \
		  text_jactor.c \
	   	-g -o bouncer \
	    $(CFLAGS)

clean:
	rm -f *.o bouncer 
