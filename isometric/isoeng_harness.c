#include "isoeng.h"
#include <stdio.h>

#define ACTOR_W 50
#define ACTOR_H 29
#define WIN_W 460
#define WIN_H 460
#define MAP_W 8
#define MAP_H 5
#define FPS 3000

void printgrp(struct isogrp *grp)
{
    struct isols *lp;

    printf("\nMembers of group: %d\n", grp->groupnum);

    for(lp = grp->ls; lp != NULL; lp = lp->next){
        printf("Actor %d\n", lp->actor->uid);
    }
    printf("\n");
}

struct isomap *isomap_test(unsigned int groups)
{
    SDL_Rect map_rect = {.x = 100, .y =40, .w = WIN_W, .h = WIN_H};
    struct isomap *map;

    map = isomap_create(&map_rect, groups, MAP_W, MAP_H,
            50, "test.png",
            "abc", "aacccccc"
                   "accccccc"
                   "cccccccc"
                   "cccccccc"
                   "ccccccca");

    return map;
}
   


int main(void)
{
    struct isoeng *engine;
    struct isoactor *actor[5];
    struct isogrp *grp;
    struct isomap *map;
    Uint32 start_t, curr_t;
    Uint32 c_frame = 0, p_frame = 0;
    double elapsed_frames = 0;
    int ef = 0;

    int testgrp = 1, testgrp2 = 1<<1;
    int carry_on = 1;
    SDL_Event selection;

    engine = isoeng_create(WIN_W, WIN_H);

    actor[0] = isoeng_new_actor(engine, ACTOR_W,
            ACTOR_H, "blob.png", testgrp);
    actor[1] = isoeng_new_actor(engine, ACTOR_W,
            ACTOR_H, "blob.png", testgrp);
    actor[2] = isoeng_new_actor(engine, ACTOR_W,
            ACTOR_H, "blob.png", testgrp2);
    actor[3] = isoeng_new_actor(engine, ACTOR_W,
            ACTOR_H, "blob.png", testgrp | testgrp2);
    actor[4] = isoeng_new_actor(engine, ACTOR_W,
            ACTOR_H, "blob.png", 1 << 4);


    grp = isoeng_get_group(engine, testgrp);
    grp = isoeng_get_group(engine, testgrp | 1 << 4);
    grp = isoeng_get_group(engine, testgrp);

    printgrp(grp);
    isoeng_actor_drop(engine, actor[0], testgrp);
    isoeng_actor_drop(engine, actor[2], testgrp);
    isoeng_actor_add(engine, actor[2], testgrp);

    printgrp(grp);

    isoeng_del_actor(engine, actor[0]);
    isoeng_del_actor(engine, actor[4]);

    actor[4] = isoeng_new_actor(engine, ACTOR_W,
            ACTOR_H, "blob.png", 1<<4);

    printgrp(grp);

    map = isomap_test(1 << 4);

    actor[4]->x = actor[4]->px = 0;
    actor[4]->y = actor[4]->py = 0;

    actor[4]->vx = 0.0025;
    actor[4]->vy = 0.001;





    start_t = SDL_GetTicks();

    while(carry_on){

        glClear(GL_COLOR_BUFFER_BIT);


        /* Do all the painting that is required */
        p_frame++;
        
        isomap_paint(map, engine, elapsed_frames);

        glFlush();
        SDL_GL_SwapBuffers();

        /* See whether it is time for a logic frame */
        curr_t = SDL_GetTicks();
        elapsed_frames = ((double)(curr_t - start_t) / 1000. * FPS) - c_frame; 
        ef = (int)elapsed_frames;

        /* Work through all the logic frames */
        while(ef--){
            struct isols *pl;

            c_frame++;

            SDL_PumpEvents();
            if(SDL_PeepEvents(&selection, 1,
                        SDL_GETEVENT, SDL_EVENTMASK(SDL_QUIT))){
                carry_on = 0;
            }

            /* Call iterator for all actors */
            for(pl = engine->actors; pl != NULL; pl = pl->next){
                isoactor_iterate(pl->actor, engine);
            }

        }
    }

#ifdef DEBUG_MODE
    fprintf(stderr, "FPS (logic): %f\n", (float)c_frame/((float)(curr_t - start_t)/1000));
    fprintf(stderr, "FPS (rendered): %f\n", (float)p_frame/((float)(curr_t - start_t)/1000));
#endif


    isoeng_free(engine);

    return 0;
}
