#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL/SDL.h"
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>

void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
}

int main(int argc, char **argv)
{
    SDL_Surface *screen, *image, *temp;
	int flags = 0;
    int width = 550;
	int height = 250;
	int bpp = 0;
    int carry_on = 1;
    double theta = 0;
    static GLuint texName;
    char *filename = "image.png";
    Uint32 rmask, bmask, gmask, amask;
    int max_size;
   // float t[16] = { 




    if(SDL_Init(SDL_INIT_VIDEO) == -1){
		fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
        return -1;
    }

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
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
    init();



#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000; gmask = 0x00ff0000; bmask = 0x0000ff00;	amask = 0x000000ff;
#else
	rmask = 0x000000ff;	gmask = 0x0000ff00;	bmask = 0x00ff0000;	amask = 0xff000000;
#endif
	

	if (!(temp = (SDL_Surface *) IMG_Load(filename))) {
        fprintf(stderr, "Error loading image\n");
        exit(-1);
    }

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
    if ((temp->w > max_size) || (temp->h > max_size)) {
        fprintf(stderr, "Image size exceeds max texture size,"
                " which is %d pixels for each side\n", max_size);
        SDL_FreeSurface(temp);
        exit(-1);
    }

    //would need to check for NPOT 
    //image = SDL_CreateRGBSurface(SDL_SWSURFACE, temp->w, temp->h, 32, rmask,
     //       gmask, bmask, amask);
    image = temp;

    //image = SDL_DisplayFormat(temp);
    //SDL_FreeSurface(temp);




    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* generates a texture name */
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight,
     //       0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, image->pixels);


    glEnable(GL_TEXTURE_2D);

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, texName);


    while(carry_on){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT)
                carry_on = 0;
            else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_a){
                    theta += 1;

                    //glViewport(0, 0, (GLsizei) width, (GLsizei) width); 
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    //glScalef(height, width, 1);
                    glOrtho(-width / 200, width / 200,
                            -height /200, height /200, 1.0, -1.0);
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();
                    //glLoadIdentity();
                    //glScalef(1.5, 1.5, 1.0);
                    glRotatef(theta, 0, 0, 1.0);
             //       glTranslatef(1.3, 0.0, 0);


                    /* need to research more carefully what this does */

                    /*this may need to be enabled, or may not be necessary */
                    //	glDisable(GL_DEPTH_TEST);
                    /* This is a 2d program, no need for depth test */



                  //  glColor3f(0, 0, 1.0);

                    glClear(GL_COLOR_BUFFER_BIT);

                    glBegin(GL_QUADS);

                    glTexCoord2f(0.0, 0.0);
                    glVertex3f(-1.0, -0.5, 0);

                    glTexCoord2f(0.0, 1.0);
                    glVertex3f(-1.0, 0.5, 0);


                    glTexCoord2f(1.0, 1.0);
                    glVertex3f(1.0, 0.5, 0);

                    glTexCoord2f(1.0, 0.0);
                    glVertex3f(1.0, -0.5, 0);
                    glEnd();


                    glFlush();


                    SDL_GL_SwapBuffers();


                }
            }

        }
    }



    glDisable(GL_TEXTURE_2D);


    return 0;
}

