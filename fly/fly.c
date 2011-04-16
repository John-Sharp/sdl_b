#include "SDL.h"
#include <SDL_image.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <math.h>
#define MAP_W 640
#define MAP_H 480
#define BIRD_W 192
#define BIRD_H 122
#define BIRD_V 0.09
#define FPS 1700

void initialise(void)
{
    if(SDL_Init(SDL_INIT_VIDEO) == -1){
        fprintf(stderr, "Video initialisation failed: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDisable(GL_DEPTH);

//    glMatrixMode(GL_PROJECTION);
 //   glLoadIdentity();
    //glOrtho(0, MAP_W, MAP_H, 0.0, -1.0, 1.0);
    //glViewport(0, 0, MAP_W, MAP_H);

    return;
}

void create_tex(const char *img_name, GLuint *tex)
{
    SDL_Surface *img;

   img = IMG_Load(img_name);
   if(!img){
       fprintf(stderr, "Failed to load %s!\n", img_name);
       exit(1);
   }

    /* create an openGL texture and bind the sprite's image
     * to it */

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);


    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w,
            img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
            img->pixels);

   SDL_FreeSurface(img);

    return;
}

void paint(GLuint tex, float v, float w, float h, Uint32 t){
    double birdxpos = v *t;
    glEnable(GL_TEXTURE_2D);
		
	glEnable(GL_BLEND);


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glTranslatef(birdxpos, 0, 0);

    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);

    glTexCoord2d(0, 0);
    glVertex2d(0, 0);

    glTexCoord2f(0, 1.0);
    glVertex2d(0.0, h);

    glTexCoord2f(1.0, 1.0);
    glVertex2d(w, h);

    glTexCoord2f(1.0, 0);
    glVertex2d(w, 0);

    glEnd();
	
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_BLEND);

   // glDisable(GL_TEXTURE_2D);

    return;
}





int main(int argc, char **argv)
{
    int flags = SDL_OPENGL;
    int bpp = 0;
    int carry_on = 1;
    Uint32 c_frame = 0;
    Uint32 interval;
    Uint32 start_t, last_t, curr_t;
    Uint32 rendered_frames = 0;
    SDL_Surface *screen;
    GLuint bg, bird;
    SDL_Event selection;
	const SDL_VideoInfo* info = NULL;

    initialise();

    info = SDL_GetVideoInfo();
	bpp = info->vfmt->BitsPerPixel;

    screen = SDL_SetVideoMode(MAP_W, MAP_H, bpp, flags); 
    if(!screen){
        fprintf(stderr, "Failed to open screen!\n");
        return 1;
    }
    create_tex("images/sky.png", &bg);
    create_tex("images/bird.png", &bird);


   /* paint section */

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, MAP_W,
           MAP_H, 0, -1, 1);
   //glViewport(0.0, 0.0,
    //       MAP_W, MAP_H);
   


   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, bg);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, 0);

   glBegin(GL_QUADS);

   glTexCoord2f(0.0, 0.0);
   glVertex3f(0, 0, 0);

   glTexCoord2f(0.0, 1.0);
   glVertex3f(0, MAP_H, 0);

   glTexCoord2f(1.0, 1.0);
   glVertex3f(MAP_W, MAP_H, 0);

   glTexCoord2f(1.0, 0.0);
   glVertex3f(MAP_W, 0 , 0);

   glEnd();
   
   /*
   glBindTexture(GL_TEXTURE_2D, bird);

   glBegin(GL_QUADS);

   glTexCoord2f(0.0, 0.0);
   glVertex3f(0, 0, 0);

   glTexCoord2f(0.0, 1.0);
   glVertex3f(0, BIRD_H, 0);

   glTexCoord2f(1.0, 1.0);
   glVertex3f(BIRD_W, BIRD_H, 0);

   glTexCoord2f(1.0, 0.0);
   glVertex3f(BIRD_W, 0 , 0);

   glEnd();
*/
   glDisable(GL_TEXTURE_2D);

   glFlush();
   SDL_GL_SwapBuffers();

   start_t = last_t = SDL_GetTicks();
   while(carry_on){

       while(SDL_PollEvent(&selection)){
           if(selection.type == SDL_QUIT){
               carry_on = 0;
           }
       }

       curr_t = SDL_GetTicks();
       if(floor((double)(curr_t - start_t) /1000. * FPS) > c_frame){
      //     c_frame = floor((double)(curr_t - start_t) / 1000. * FPS);
           c_frame++;
           paint(bg, 0, MAP_W, MAP_H, curr_t);
           paint(bird, 0.1, BIRD_W, BIRD_H, curr_t);
           rendered_frames++;
       }
       glFlush();
       SDL_GL_SwapBuffers();
           
   }

   fprintf(stdout, "frames: %d\n", rendered_frames);
   fprintf(stdout, "seconds: %d \n", (curr_t - start_t) / 1000);
   fprintf(stderr, "FPS: %f\n", (float)rendered_frames/((float)(curr_t - start_t) / 1000));

   return 0;

}
