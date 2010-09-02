#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL/SDL.h"
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>

/* Create checkerboard texture */
#define checkImageWidth 64
#define checkImageHeight 64
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];
static GLuint texName;

void makeCheckImage(void)
{
    int i, j, c;
    
    for (i = 0; i < checkImageHeight; i++) {
        for (j = 0; j < checkImageWidth; j++) {
            c = ((((i&0x8)==0)^((j&0x8))==0))*255;
            checkImage[i][j][0] = (GLubyte) c;
            checkImage[i][j][1] = (GLubyte) c;
            checkImage[i][j][2] = (GLubyte) c;
            checkImage[i][j][3] = (GLubyte) 255;
        }
    }
} 

int main(int argc, char **argv)
{
    SDL_Surface *screen;
	int flags = 0;
    int width = 250;
	int height = 250;
	int bpp = 0;

    if(SDL_Init(SDL_INIT_VIDEO) == -1){
		fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
        return -1;
    }

    atexit(SDL_Quit);


	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	flags = SDL_OPENGL; 

	if((screen = SDL_SetVideoMode(width, height, bpp, flags)) == 0) {
		fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
        	exit(-1);
	}

    /* OpenGL initialisation */
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glViewport(0, 0, (GLsizei) width, (GLsizei) height); 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat) width/(GLfloat) height, 1.0, 30.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -5.6);

    /* need to research more carefully what this does */
    glShadeModel(GL_FLAT);

    /*this may need to be enabled, or may not be necessary */
	glDisable(GL_DEPTH_TEST); /* This is a 2d program, no need for depth test */

    makeCheckImage();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* generates a texture name */
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight,
            0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, texName);
    glBegin(GL_QUADS);
    
        glTexCoord2f(0.0, 0.0);
        glVertex3f(-2.0, -1.0, 0.0);

        glTexCoord2f(0.0, 1.0);
        glVertex3f(-2.0, 1.0, 0.0);

        glTexCoord2f(1.0, 1.0);
        glVertex3f(0.0, 1.0, 0.0);

        glTexCoord2f(1.0, 0.0);
        glVertex3f(0.0, -1.0, 0.0);
        


        glTexCoord2f(0.0, 0.0);
        glVertex3f(1.0, -1.0, 0.0);

        glTexCoord2f(0.0, 1.0);
        glVertex3f(1.0, 1.0, 0.0);

        glTexCoord2f(1.0, 1.0);
        glVertex3f(2.41421, 1.0, -1.41421);

        glTexCoord2f(1.0, 0.0);
        glVertex3f(2.41421, -1.0, -1.41421);
    glEnd();
    glFlush();
    glDisable(GL_TEXTURE_2D);


    SDL_GL_SwapBuffers();

    SDL_Delay(5000);


    return 0;
}

