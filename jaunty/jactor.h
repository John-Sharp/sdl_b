//functions for creating and manipulating an actor
#ifndef JACTOR_H
#define JACTOR_H

#include "SDL.h"

typedef struct jactor jactor;

struct jactor
{
    //width and height of actor in pixels
    int w;
    int h;

    //previous logic frames position
    double px, py;

    //interpolated position
    double gx, gy;

    //position, velocity and acceleration of 
    //actor
    double x, y, v_x, v_y, a_x, a_y;

    //magnitude of acceleration
    double a;
    //maximum speed of actor
    double v_max;

    //collision handling function
    void (*c_handler)(jactor *actor) ;

    //picture of the actor
    SDL_Surface *sprite;
};

//frees all resources allocated to 'actor'
void jactor_free(jactor *actor);

//creates an actor
jactor *jactor_create(int w, int h, const char *sprite_filename,
                      double v_max, double a, void (*c_handler)(jactor *));

//gives the 'actor' a direction of movement
void jactor_set_direction(jactor *actor, double x, double y);

//computes the new position of 'actor', after a logical frame has passed 
void jactor_iterate(jactor *actor);

//blits the 'actor' onto the 'screen'
SDL_Rect jactor_paint(jactor *actor, SDL_Surface *screen, double frames_so_far);
#endif  
