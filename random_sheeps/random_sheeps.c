/* random_sheeps.c
 * 03-06-11
 * Random sheep moving demo.
 *
 */

#include "../isometric/isoeng.h"
#include <stdio.h>
#include <stdlib.h>

#define ACTOR_W 111
#define ACTOR_H 68
#define NUM_SHEEPS 6
#define C_ACTOR_W 154
#define C_ACTOR_H 94
#define WIN_W 720
#define WIN_H 460
#define MAP_W 12
#define MAP_H 12
#define TILE_W 60
#define VMAG 0.01
#define SHEEP_LEFT 0
#define SHEEP_RIGHT 1



#define FPS 6500
#define CTW 5
#define CTH 5

void sheep_i_handler(struct isoactor *actor, struct isoeng *eng)
{
    if(actor->vx < 0)
        actor->show_sprite = SHEEP_LEFT;
    else
        actor->show_sprite = SHEEP_RIGHT;
}

void sheep_ob_handler(struct isomap *map, struct isoactor *actor)
{
    if(actor->x / map->itw > map->w ||
       actor->x / map->itw < 0){
        actor->x = actor->px;
        actor->vx = -actor->vx;
    }else{
        actor->y = actor->py;
        actor->vy = -actor->vy;
    }
    return;
}

struct isomap *sheep_map(unsigned int groups)
{
    SDL_Rect map_rect = { .x = 0, .y = 0, .w = WIN_W, .h = WIN_H};
    char key[] = "abcdef";
    char map_str[] = "deaaabafaabe"
                     "abaffaacdefa"
                     "efdefaaabbdf"
                     "eeaaaadfafde"
                     "eeaaaadfafde"
                     "bcabaffaaffd"
                     "abaffaacdefa"
                     "efdefaaabbdf"
                     "abaffaacdefa"
                     "deaaabafaabe"
                     "deaaabafaabe"
                     "abaffaacdefa"
                     "bcabaffaaffd";
    char c_map_str[] = "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa"
                       "aaaaaaaaaaaa";

    struct isomap *map;

    map = isomap_create(&map_rect, groups, MAP_W, MAP_H,
            TILE_W, "images/grass.png", key, map_str, key, c_map_str);

    map->ob_handler = sheep_ob_handler;

    return map;
}

void sheep_hit_handler(struct isoactor *a1, struct isoactor *a2)
{
    double mag = sqrt(pow((a1->x - a2->x),2) + pow((a1->y - a2->y), 2));

    a1->vx = VMAG * (a1->x - a2->x)/mag;
    a1->vy = VMAG * (a1->y - a2->y)/mag;
    fprintf(stderr, " SHEEP COLLISION!!\n");

    return;
}

int main(void)
{
    struct isoeng *engine;
    struct isoactor *sheep;
    struct isomap *map;
    unsigned int shp_grp = 1<<1;
    int c_frame = 0, ef = 0;
    int carry_on = 1;

    Uint32 start_t, curr_t;
    double elapsed_frames = 0;

    engine = isoeng_create(WIN_W, WIN_H, CTW, CTH);
    map = sheep_map(shp_grp);

    int i;
    for(i = 0; i < NUM_SHEEPS; i++){
        double theta = (double)rand();

        sheep = isoeng_new_actor(engine, ACTOR_W, ACTOR_H, 
                "images/sheep.png", C_ACTOR_W, C_ACTOR_H,
                "images/c_sheep.png", shp_grp);

        sheep->x = sheep->px = rand() % (map->w * map->itw);
        sheep->y = sheep->py = rand() % (map->h * map->itw);
        sheep->vx = VMAG * cos(theta);
        sheep->vy = VMAG * sin(theta);
        set_actor_handler(sheep, map, shp_grp, sheep_hit_handler);
        sheep->i_handler = sheep_i_handler;
    }

    start_t = SDL_GetTicks();

    while(carry_on){
        glClear(GL_COLOR_BUFFER_BIT);

        isomap_paint(map, engine, elapsed_frames);

        glFlush();
        SDL_GL_SwapBuffers();

        /* See whether it is time for a logic frame */
        curr_t = SDL_GetTicks();
        elapsed_frames = ((double)(curr_t - start_t) / 1000. * FPS) - c_frame; 
        ef = (int)elapsed_frames;

        while(ef--){
            struct isols *pl;

            c_frame++;

            SDL_PumpEvents();

            SDL_Event selection;
            if(SDL_PeepEvents(&selection, 1,
                        SDL_GETEVENT, SDL_EVENTMASK(SDL_QUIT))){
                carry_on = 0;
            }


            /* Call iterator for all actors */
            for(pl = engine->actors; pl != NULL; pl = pl->next){
                isoactor_iterate(pl->actor, engine);
            }

            /* Call iterator for map */
            isomap_iterate(map, engine);

        }
    }

    isoeng_free(engine);

    return 0;
}

