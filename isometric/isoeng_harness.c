#include "isoeng.h"
#include <stdio.h>

#define ACTOR_W 50
#define ACTOR_H 29
#define WIN_W 460
#define WIN_H 460
#define MAP_W 6
#define MAP_H 3
#define FPS 800

void printgrp(struct isogrp *grp)
{
    struct isols *lp;

    printf("\nMembers of group: %d\n", grp->groupnum);

    for(lp = grp->ls; lp != NULL; lp = lp->next){
        printf("Actor %d\n", lp->actor->uid);
    }
    printf("\n");
}

struct isomap *isomap_test()
{
    SDL_Rect map_rect = {.x = 0, .y =0, .w = WIN_W, .h = WIN_H};
    struct isomap *map;

    map = isomap_create(&map_rect, MAP_W, MAP_H, 50, "test.png",
            "abc", "cccccc"
                   "cccccc"
                   "ccaacc"
                   "ccaacc"
                   "cccccc"
                   "cccccc");

    return map;
}
   


int main(void)
{
    struct isoeng *engine;
    struct isoactor *actor[5];
    struct isogrp *grp;
    struct isomap *map;
    Uint32 start_t, curr_t;
    Uint32 c_frame = 0, p_frame = 0, elapsed_frames = 0;

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

    isoeng_actor_drop(engine, actor[0], testgrp);
    isoeng_actor_drop(engine, actor[2], testgrp);
    isoeng_actor_add(engine, actor[2], testgrp);

    printgrp(grp);

    isoeng_del_actor(engine, actor[0]);
//    isoeng_del_actor(engine, actor[1]);
    isoeng_del_actor(engine, actor[4]);

    actor[4] = isoeng_new_actor(engine, ACTOR_W,
            ACTOR_H, "blob.png", testgrp);
    actor[4] = isoeng_new_actor(engine, ACTOR_W,
            ACTOR_H, "blob.png", testgrp);
//    isoeng_del_actor(engine, actor[3]);

    printgrp(grp);

    map = isomap_test();

    actor[0]->x = 100;
    actor[0]->y = 50;
    actor[0]->px = 100;
    actor[0]->py = 50;


    start_t = SDL_GetTicks();
    while(carry_on){
        /* Do all the painting that is required */
        p_frame++;
        isomap_paint(map);
        isoactor_paint(actor[0], map, elapsed_frames);
        glFlush();
        SDL_GL_SwapBuffers();

        curr_t = SDL_GetTicks();
        elapsed_frames = ((double)(curr_t - start_t) / 1000. * FPS) - c_frame; 

        while((int)elapsed_frames--){
            c_frame++;
            while(SDL_PollEvent(&selection)){
                if(selection.type == SDL_QUIT){
                    carry_on = 0;
                }
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
