/* isomap_harness.c
 * 27-02-2011
 * Test harness for isomap module.
 *
 */

#include "SDL.h"
#include <SDL_image.h>
#include <GL/gl.h>
#include "isomap.h"
#include <stdlib.h>
#include <math.h>
#define MAP_W 6 
#define MAP_H 3 
#define WIN_W 460
#define WIN_H 460
#define FPS 800

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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0.0, -1.0, 1.0);
    glViewport(0, 0, WIN_W, WIN_H);

    return;
}

int main(int argc, char **argv)
{
    int flags = SDL_OPENGL;
    int bpp = 0;
    int carry_on = 1;
    Uint32 c_frame = 0;
    Uint32 start_t, last_t, curr_t;
    Uint32 rendered_frames = 0;
    SDL_Surface *screen;
    struct isomap *map;
    SDL_Rect map_rect = {.x = 100, .y = 100, .w = WIN_W, .h = WIN_H};
    SDL_Event selection;
	const SDL_VideoInfo* info = NULL;

    initialise();

    info = SDL_GetVideoInfo();
	bpp = info->vfmt->BitsPerPixel;

    screen = SDL_SetVideoMode(WIN_W, WIN_H, bpp, flags); 
    if(!screen){
        fprintf(stderr, "Failed to open screen!\n");
        return 1;
    }

    /* Load the map */
    map = isomap_create(&map_rect, MAP_W, MAP_H, 50, "test.png",
            "abc", "cccccc"
                   "cccccc"
                   "ccaacc"
                   "ccaacc"
                   "cccccc"
                   "cccccc");
   
   start_t = last_t = SDL_GetTicks();
   while(carry_on){

       while(SDL_PollEvent(&selection)){
           if(selection.type == SDL_QUIT){
               carry_on = 0;
           }
       }

       curr_t = SDL_GetTicks();
       if(floor((double)(curr_t - start_t) / 1000. * FPS) > c_frame){
           c_frame++;
           isomap_paint(map);
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
