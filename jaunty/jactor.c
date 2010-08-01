//functions for creating and manipulating an actor
#include <stdlib.h>
#include <math.h>
#include "SDL.h"
#include <SDL_image.h>
#include "jactor.h"
#include "jen.h"
#include "jutils.h"

void jactor_free(jactor *actor)
{
    SDL_FreeSurface(actor->sprite);
    free(actor);
}

jactor *jactor_create(int w, int h, const char *sprite_filename, double v_max,
                      double a, void (*c_handler)(jactor *, jcoll *))
{
    SDL_Surface *temp;
    jactor *actor;


    if((actor = malloc(sizeof(*actor))) == NULL)
        return NULL; 

    temp = IMG_Load(sprite_filename);

    if(!temp){
        fprintf(stderr, "Error! Could not load %s\n", sprite_filename);
        jactor_free(actor);
        return NULL;
    } 

    actor->sprite = SDL_DisplayFormatAlpha(temp);
    SDL_FreeSurface(temp);
    if(!actor->sprite){
        fprintf(stderr, "Error! Could not convert %s\n", sprite_filename);
        return NULL;
    }

    actor->w = w;
    actor->h = h;
    actor->v_max = v_max;
    actor->a = a;

    actor->v_x = 0;
    actor->v_y = 0;
    actor->x = 0;
    actor->y =0;
    actor->a_x = 0;
    actor->a_y = 0;

    actor->c_handler = c_handler;

    return actor;
}

void jactor_set_direction(jactor *actor, double x, double y)
{
    double magnitude;

    //at the moment velocity is set to 0 every time the direction 
    //changes, maybe need to change this in future
    actor->v_x = 0;
    actor->v_y = 0;

    magnitude = sqrt(pow(x, 2) + pow(y, 2));
    actor->a_x = actor->a * x / magnitude;
    actor->a_y = actor->a * y / magnitude;


    return;
}

void jactor_iterate(jactor *actor)
{
    actor->px = actor->x;
    actor->py = actor->y;


    actor->v_x += actor->a_x;
    actor->v_y += actor->a_y;

    actor->x += actor->v_x; 
    actor->y += actor->v_y;

    return;
}

SDL_Rect jactor_paint(jactor *actor, SDL_Surface *screen, double frames_so_far)
{
    double fframe = frames_so_far - floor(frames_so_far);
    SDL_Rect src, dst;


    src.x = 0;
    src.y = 0;
    src.w = actor->w;
    src.h = actor->h;

    actor->gx  = actor->px * (1-fframe) + fframe * actor->x;
    actor->gy  = actor->py * (1-fframe) + fframe * actor->y;

    dst.x = actor->gx - actor->w/2;
    dst.y = actor->gy - actor->h/2;
    dst.w = actor->w;
    dst.h = actor->h;

    SDL_BlitSurface(actor->sprite, &src, screen, &dst);

    return dst;
}


/* TODO: make the detection work by searching the entire path traced out
 * by the actors from the previous frame to the next frame */
int jactor_collision_detect(jactor *actor1, jactor *actor2, jcoll *c_info)
{
    double separation;
    /* the angle between the line joining the centres of the two 
     * colliding actors and the x-axis */
    double theta;

    separation = magnitude(actor1->y - actor2->y, actor1->x - actor2->x);

    if(separation < actor1->w || separation < actor2->w){
        theta = atan((actor2->y - actor1->y)/(actor2->x - actor1->y));
        c_info->x = actor1->x + actor1->w * cos(theta);
        c_info->y = actor1->y + actor1->w * sin(theta);
        c_info->actor = actor2;
        c_info->c_type = A_A;
        return 1;
    }

    return 0;
}
