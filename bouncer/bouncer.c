#include "../jaunty/jmap.h"
#include "../jaunty/jactor.h"
#include "../jaunty/jen.h"
#include "../jaunty/jdirty.h"
#include "../jaunty/jutils.h"
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

    m = "aaaaaaaaaaaaaaaa"
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


void dot_collider(jactor *a, jcoll *c_info)
{

    if(c_info->c_type == A_T){
        if(c_info->side & TOP){
            a->x = c_info->x;
            a->px = c_info->x;
            a->py = c_info->y;
            a->y = c_info->y - 1;
            a->v_y = -0.999*a->v_y;
        }else if(c_info->side & BOTTOM){
            a->x = c_info->x;
            a->px = c_info->x;
            a->py = c_info->y;
            a->y = c_info->y + 1;
            a->v_y = -0.999*a->v_y;
        }
        if(c_info->side & LEFT){
            a->px = c_info->x;
            a->py = c_info->y;
            a->x = c_info->x-1;
            a->y = c_info->y;
            a->v_x = -0.999*a->v_x;
        }else if(c_info->side & RIGHT){
            a->px = c_info->x;
            a->py = c_info->y;
            a->x = c_info->x +1;
            a->y = c_info->y;
            a->v_x = -0.999*a->v_x;
        }
    }else{
        double m1 = a->m, m2 = c_info->actor->m;
        /* angle between line joining centres of two colliding
         * actors and the x-axis */
        double theta = atan((c_info->actor->y - a->y)/
                (c_info->actor->x - a->x));

        /* velocity resolved parallel to the line of collision, prior
         * to collision (element [0]) and a variable for storing the 
         * velocity after the collision (element [1]) */
        double v1p[2] = {a->v_x * cos(theta) + a->v_y * sin(theta), 0};
        double v2p[2] = {c_info->actor->v_x * cos(theta) + 
            c_info->actor->v_y * sin(theta), 0};

        /* velocity resolved transverse to the line of collision */
        double v1t = -a->v_x * sin(theta) + a->v_y * cos(theta);
        double v2t = -c_info->actor->v_x * sin(theta) + 
            c_info->actor->v_y * cos(theta);


        v1p[1] = v1p[0] * (m1 - m2)/(m1 + m2)
            + v2p[0] * 2 * m2 / (m1 + m2);

        v2p[1] = v2p[0] * (m2 - m1)/(m1 + m2)
            + v1p[0] * 2 * m1 / (m1 + m2);

        /* transforming back into the original set of x and y
         * axes */
        a->v_x = v1p[1] * cos(-theta) + v1t * sin(-theta);
        a->v_y = -v1p[1] * sin(-theta) + v1t * cos(-theta);

        c_info->actor->v_x = v2p[1] * cos(-theta) + v2t * sin(-theta);
        c_info->actor->v_y = -v2p[1] * sin(-theta) + v2t * cos(-theta);

    }

    return;
}


void cast_actors(jen *engine)
{
    jactor *dot;

    //set up 'dot' actor
    dot = jactor_create(50, 50, "circle.png", 10, 2, NULL);
    dot->px = dot->x = 100;
    dot->py = dot->y = 250;
    dot->v_y = 0;
    dot->v_x = -30;
    dot->m = 5;
    dot->a_x = 0;
    dot->a_y = 0;
    dot->c_handler = dot_collider;

    /* register the actor */
    jen_add_jactor(engine, dot);

    /* set up the second actor */
    dot = jactor_create(50, 50, "circle2.png", 10, 2, NULL);
    dot->px = dot->x = 200;
    dot->py = dot->y = 250;
    dot->v_y = 0;
    dot->v_x = 0;
    dot->a_x = 0;
    dot->m = 5;
    dot->a_y = 0;
    dot->c_handler = dot_collider;

    jen_add_jactor(engine, dot);

    /* set up the second actor */
    dot = jactor_create(50, 50, "circle2.png", 10, 2, NULL);
    dot->px = dot->x = 250;
    dot->py = dot->y = 250;
    dot->v_y = 0;
    dot->v_x = 0;
    dot->a_x = 0;
    dot->m = 5;
    dot->a_y = 0;
    dot->c_handler = dot_collider;

    jen_add_jactor(engine, dot);

    /* set up the second actor */
    dot = jactor_create(50, 50, "circle2.png", 10, 2, NULL);
    dot->px = dot->x = 300;
    dot->py = dot->y = 250;
    dot->v_y = 0;
    dot->v_x = 0;
    dot->a_x = 0;
    dot->m = 5;
    dot->a_y = 0;
    dot->c_handler = dot_collider;

    jen_add_jactor(engine, dot);

    return;
}

int main(int argc, char **argv)
{
    int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;
    SDL_Surface *screen, *buffer;
    jmap *bg_map;
    int bpp = 0;
    double fps = 20;
    int last_tick;
    int start_time, end_time;
    SDL_Rect extent = {.x = 0, .y=0, .w = 0, .h = 0};
    SDL_Rect *dirty_r;
    double frames_so_far;
    int rendered_frames = 0, logic_frames = 0;
    int carry_on = 1;
    jen *engine;


    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    //set up screen
    screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
    if(!screen){
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

    /* set up the engine */
    if(!(engine = jen_create())){
        fprintf(stderr, "Unable to allocate resources to engine\n");
        return 1;
    }


    dirty_r = jdirty_reset();

    /* set up background map */
    if(!(bg_map = jmap_create(SCREEN_W / TILE_W, SCREEN_H / TILE_H))){
        fprintf(stderr, "Error could not open jmap!\n");
        return 1;
    }
    if(jmap_load_tilepalette(bg_map, "testmap.png", TILE_W, TILE_H) == -1){
        fprintf(stderr, "Error could not load the tile palette!\n");
        return 1;
    }

    load_level(bg_map, 1);

    jmap_paint(bg_map, buffer, NULL);
    SDL_BlitSurface(buffer, NULL, screen, NULL);
    SDL_Flip(screen);


    cast_actors(engine);

    rendered_frames = 0;

    start_time = last_tick = SDL_GetTicks();
    
    while(carry_on){
        int tick, elapsed_whole_frames, i;
        double frames, dt;
        SDL_Event event;
        jactor_ls *actor_ls;

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
            
            logic_frames++;

            for(actor_ls = engine->actors;
             actor_ls != NULL; actor_ls = actor_ls->next){ 

                jactor *a = actor_ls->actor;
                jactor_ls *p;

                jactor_iterate(a);
                if(jmap_collision_detect(bg_map, a, &c_info, 1)){
                    a->c_handler(a, &c_info);
                }
                for(p = actor_ls->next; p != NULL; p = p->next){
                    if(jactor_collision_detect(a, p->actor, &c_info)){
                        a->c_handler(a, &c_info);
                    }
                }
            }

        }
        
        frames_so_far += frames;

        
        /* overwrite with the background what was written last time */
        for(i = 0; i < jdirty_get_prev(); i++){
            jmap_paint(bg_map, buffer, &dirty_r[i]);
            SDL_BlitSurface(buffer, &dirty_r[i], screen, &dirty_r[i]);
        }
        
        /* draw all the actors onto the screen */
        for(actor_ls = engine->actors;
         actor_ls != NULL; actor_ls = actor_ls->next){
            extent = jactor_paint(actor_ls->actor, buffer, frames_so_far);
            jdirty_add(extent);
            SDL_BlitSurface(buffer, &extent, screen, &extent);
        }


        SDL_UpdateRects(screen, jdirty_get_prev() + jdirty_get_curr(),
                dirty_r);

        jdirty_iterate();

        rendered_frames++;
        last_tick = tick;
        //SDL_Delay(10);
        
    }
 
    end_time = SDL_GetTicks();

    printf("\nLogic frames: %.2f\n", logic_frames * 1000.0 / (end_time - 
               start_time)); 

    printf("\nRendered frames: %.2f\n", rendered_frames * 1000.0 / (end_time - 
               start_time));

    /* free resources */
    jen_free(engine);

    return 0;
}
