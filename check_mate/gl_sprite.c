#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL/SDL.h"
#include <SDL/SDL_image.h>
#include <GL/gl.h>

/* returns a number that is  a number, 'a', converted into the nearest number
 * that is a whole power of 2 (rounding up) */ 
#define mkp2(a) (int)powf(2.0, ceilf(logf((float)a)/logf(2.0)))


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	enum { RMASK = 0xff000000, GMASK = 0x00ff0000, BMASK = 0x0000ff00,	AMASK = 0x000000ff};
#else
	enum { RMASK = 0x000000ff,	GMASK = 0x0000ff00,	BMASK = 0x00ff0000,	AMASK = 0xff000000};
#endif


typedef struct gl_sprite gl_sprite;

struct gl_sprite {
    /* width and height of sprite */
    int w, h;

    /* width and height of the actual texture, which is required to be 
     * a power of 2 by openGL */
    int p2w, p2h;

    /* x and y coordinates of pivot-point of sprite */
    double x, y;

    /* angle through which the sprite has been rotated about the pivot-point */
    double theta;

    /* distance of the sprite from the pivot-point */
    double r;

    /* sprite image */
    SDL_Surface *sprite;

    /* name of texture associated with this sprite */
    GLuint tex_name;

};

/* release the resources allocated to the sprite */
void gl_sprite_free(gl_sprite *gls);

/* create a sprite */
gl_sprite *gl_sprite_create(int w, int h, const char *filename);

/* draw the sprite */
void gl_sprite_paint(gl_sprite *gls);

void gl_sprite_free(gl_sprite *gls)
{
    SDL_FreeSurface(gls->sprite);
    glDeleteTextures(1, &(gls->tex_name));
    free(gls);
}

gl_sprite *gl_sprite_create(int w, int h, const char *filename)
{
    gl_sprite *gls;
    SDL_Surface *temp;
    int xpad, ypad;
    int max_size;
    SDL_Rect dst;
    Uint32 alpha;

    if((gls = malloc(sizeof(*gls))) == NULL){
        fprintf(stderr, "Could not allocate memory for sprite\n");
        return NULL; 
    }

    gls->w = w;
    gls->h = h;

    /* load image */
    temp = IMG_Load(filename);

    if(!temp){
        fprintf(stderr, "Error! Could not load %s\n", filename);
        gl_sprite_free(gls);
        return NULL;
    } 

    /* check it doesn't exceed the maximum texture size */
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);

    if ((temp->w > max_size) || (temp->h > max_size)) {
        fprintf(stderr, "Image size exceeds max texture size\n");
        SDL_FreeSurface(temp);
        gl_sprite_free(gls);
        return NULL;
    }

    /* copy image onto a surface whose width and height are 
     * both a power of two (as demanded by openGL) */
    gls->p2w = mkp2(w);
    gls->p2h = mkp2(h);

    xpad = (gls->p2w - gls->w)/2;
    ypad = (gls->p2h - gls->h)/2;

    gls->sprite = SDL_CreateRGBSurface(SDL_SWSURFACE, gls->p2w,
            gls->p2h, 32, RMASK, GMASK, BMASK, AMASK);

    alpha = SDL_MapRGBA(temp->format, 0, 0, 0, 255); 
    SDL_FillRect(gls->sprite, NULL, alpha); 
    

    if(!gls->sprite){
        fprintf(stderr, "Error creating a surface for the sprite\n");
        SDL_FreeSurface(temp);
        gl_sprite_free(gls);
        return NULL;
    }

    dst.x = xpad;
    dst.y = ypad;
    dst.w = gls->w;
    dst.h = gls->h;

    SDL_BlitSurface(temp, 0, gls->sprite, &dst); 
    SDL_FreeSurface(temp);

    /* create an openGL texture and bind the sprite's image
     * to it */
    glGenTextures(1, &(gls->tex_name));
    glBindTexture(GL_TEXTURE_2D, gls->tex_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gls->sprite->w, gls->sprite->h,
            0, GL_RGBA, GL_UNSIGNED_BYTE, gls->sprite->pixels);

    SDL_FreeSurface(gls->sprite);

    return gls;
}


void gl_sprite_paint(gl_sprite *gls)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(gls->x, 0, 0);
    glRotatef(gls->theta, 0, 0, 1.0);
    glTranslatef(gls->r, 0.0, 0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gls->tex_name);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-(float)gls->p2w/2.0, -(float)gls->p2h/2.0, 0);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-(float)gls->p2w/2.0, (float)gls->p2h/2.0, 0);

    glTexCoord2f(1.0, 0.0);
    glVertex3f((float)gls->p2w/2.0, (float)gls->p2h/2.0, 0);

    glTexCoord2f(1.0, 1.0);
    glVertex3f((float)gls->p2w/2.0, -(float)gls->p2h/2.0, 0);

    glEnd();

    glDisable(GL_TEXTURE_2D);

    return;
}


int main(int argc, char **argv)
{
	int flags = 0;
    SDL_Surface *screen;
    int width = 550;
	int height = 250;
	int bpp = 0;
    int carry_on = 1;
    char *filename = "cue.png";
    gl_sprite *spr;

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
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-width/2, width/2,
            -height/2, height/2, 1.0, -1.0);


    spr = gl_sprite_create(1000, 25, filename);

    //spr->theta = 90;
    spr->r = 550;
    glClear(GL_COLOR_BUFFER_BIT);
    gl_sprite_paint(spr);

    glFlush();

    SDL_GL_SwapBuffers();

    while(carry_on){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT)
                carry_on = 0;
            else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_a){
                    spr->x += 5;
                    
                }
                else if(event.key.keysym.sym == SDLK_b) {
                    spr->theta += 2;
                }

                glClear(GL_COLOR_BUFFER_BIT);
                gl_sprite_paint(spr);
                glFlush();
                //SDL_Delay(50);
                SDL_GL_SwapBuffers();

            }

        }
    }



    gl_sprite_free(spr);
    glDisable(GL_TEXTURE_2D);


    return 0;
}


