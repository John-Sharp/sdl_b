#include "../jaunty/jmap.h"
#include "../jaunty/jactor.h"
#include "SDL.h"
#include <sys/types.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <math.h>

enum {
    SCREEN_W = 800,
    SCREEN_H = 600,
    TILE_W = 50,
    TILE_H = 50
};

void load_level(jmap *bg_map, int level)
{
    const char *k;
    const char *m;
    const char *ck;
    const char *cm;

    k = "abc";

    m = "acccccccccccccca"
        "acccccccccccccca"
        "acccccccccccccca"
        "acccccccccccccca"
        "aaaaaaaaaaaaaaaa"
        "acccccccccccccca"
        "acccccccccccccca"
        "acccccccccccccca"
        "aaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaa";
    

    jmap_from_string(bg_map, k, m);

    ck = "ac";
 
    cm = "aaaaaaaaaaaaaaaa"
         "acccccccccccccca"
         "acccccccccccccca"
         "acccccccccccccca"
         "acccccccccccccca"
         "acccccccccccccca"
         "acccccccccccccca"
         "acccccccccccccca"
         "aaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaa";

    jmap_c_map_from_string(bg_map, ck, cm);
        

    return;
}

int main(int argc, char **argv)
{
    int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;
    SDL_Surface *screen, *buffer;
    SDL_Rect paint_area;
    jmap *bg_map;
    jactor *dot;
    int bpp = 0;
    double fps = 30;
    int last_tick;
    int start_time, end_time;
    SDL_Rect extent;
    SDL_Rect update_areas[2];
    double frames_so_far;
    int rendered_frames = 0, logic_frames = 0;
    int carry_on = 1;

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    //set up screen
    screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
    if(!screen)
    {
        fprintf(stderr, "Failed to open screen!\n");
        return 1;
    }

    buffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
				screen->w, screen->h,
				screen->format->BitsPerPixel,
				screen->format->Rmask,
				screen->format->Gmask,
				screen->format->Bmask,
				screen->format->Amask);


    //set up background map
    if(!(bg_map = jmap_create(SCREEN_W / TILE_W, SCREEN_H / TILE_H))){
        fprintf(stderr, "Error could not open jmap!\n");
        return 1;
    }
    if(jmap_load_tilepalette(bg_map, "testmap.png", TILE_W, TILE_H) == -1){
        fprintf(stderr, "Error could not load the tile palette!\n");
        return 1;
    }
    load_level(bg_map, 1);

    paint_area.x = 450;
    paint_area.y = 15;
    paint_area.w = 596;
    paint_area.h = 600;
    jmap_paint(bg_map, buffer, NULL);
    SDL_BlitSurface(buffer, NULL, screen, NULL);
    SDL_Flip(screen);


    //set up 'dot' actor
    dot = jactor_create(50, 50, "circle.png", 10, 2, NULL);
    dot->px = dot->x = 600;
    dot->py = dot->y = 250;
    dot->v_y = 37.7;
    dot->v_x = 10;
    dot->a_x = 0;
    dot->a_y = 0;


    jactor_paint(dot, buffer, 0);
    rendered_frames = 0;

    start_time = last_tick = SDL_GetTicks();
    while(carry_on){
        int tick, elapsed_whole_frames;
        double frames, dt;
        SDL_Event event;

        while(SDL_PollEvent(&event))
            if(event.type == SDL_QUIT)
                carry_on = 0;
                //exit(0);

        tick = SDL_GetTicks();
        dt = (tick - last_tick) * 0.001;
        frames = dt * fps;
        elapsed_whole_frames = floor(frames_so_far + frames) - 
            floor(frames_so_far);

        while(elapsed_whole_frames--){
            jcoll c_info;
            
            jactor_iterate(dot);
            logic_frames++;

            if(jmap_collision_detect(bg_map, dot, &c_info, 1)){
                if(c_info.side & TOP){
                    dot->x = c_info.x;
                    dot->px = c_info.x;
                    dot->py = c_info.y;
                    dot->y = c_info.y - 1;
                    dot->v_y = -0.999*dot->v_y;
                }else if(c_info.side & BOTTOM){
                    dot->x = c_info.x;
                    dot->px = c_info.x;
                    dot->py = c_info.y;
                    dot->y = c_info.y + 1;
                    dot->v_y = -0.999*dot->v_y;
                }
                if(c_info.side & LEFT){
                    dot->px = c_info.x;
                    dot->py = c_info.y;
                    dot->x = c_info.x-1;
                    dot->y = c_info.y;
                    dot->v_x = -0.6*dot->v_x;
                }else if(c_info.side & RIGHT){
                    dot->px = c_info.x;
                    dot->py = c_info.y;
                    dot->x = c_info.x +1;
                    dot->y = c_info.y;
                    dot->v_x = -0.6*dot->v_x;
                }
            }

        }
        frames_so_far += frames;

        jmap_paint(bg_map, buffer, &extent);
        memmove(&update_areas, &extent, sizeof(extent)); 
        SDL_BlitSurface(buffer, &extent, screen, &extent);
        extent = jactor_paint(dot, buffer, frames_so_far);
        memmove(&update_areas + 1, &extent, sizeof(extent));
        SDL_BlitSurface(buffer, &extent, screen, &extent);
        //SDL_BlitSurface(buffer, NULL, screen, NULL);
        SDL_UpdateRects(screen, 2, update_areas);

        //SDL_Flip(screen);
        rendered_frames++;
        last_tick = tick;
    }
    end_time = SDL_GetTicks();

    printf("\nLogic frames: %.2f\n", logic_frames * 1000.0 / (end_time - 
               start_time)); 

    printf("\nRendered frames: %.2f\n", rendered_frames * 1000.0 / (end_time - 
               start_time));

    return 0;
}
