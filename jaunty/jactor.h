//functions for creating and manipulating an actor
#ifndef JACTOR_H
#define JACTOR_H

#include "SDL.h"

typedef struct jactor jactor;

/* flag for storing whether collision was actor-actor or actor-tile */
typedef enum{
    A_A = 0,
    A_T = 1
} jctype;



typedef enum {
    NONE = 0,
    TOP = 0x1,
    TOPRIGHT = 0x11,
    RIGHT = 0x10,
    BOTTOMRIGHT = 0x110,
    BOTTOM = 0x100,
    BOTTOMLEFT = 0x1100,
    LEFT = 0x1000,
    TOPLEFT = 0x1001,
    MIDDLE = 0x10000
} jsides;


typedef struct jcoll
{
    double x, y; /* x, y coordinates of the collision */
    jctype c_type; /* flag to show whether collision was actor-actor or 
                      actor-tile */
    unsigned char c_index; /* Index of the tile that was collided with */
    jsides side; /* Side of the tile that was collided with */
    jactor *actor; /* the actor that was collided with */

} jcoll;
 
struct jactor
{
    //width and height of actor in pixels
    int w;
    int h;

    //the mass of the actor
    double m;

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
    void (*c_handler)(jactor *, jcoll *);

    //picture of the actor
    SDL_Surface *sprite;
};

//frees all resources allocated to 'actor'
void jactor_free(jactor *actor);

//creates an actor
jactor *jactor_create(int w, int h, const char *sprite_filename,
                      double v_max, double a,
                      void (*c_handler)(jactor *, jcoll *));

//gives the 'actor' a direction of movement
void jactor_set_direction(jactor *actor, double x, double y);

//computes the new position of 'actor', after a logical frame has passed 
void jactor_iterate(jactor *actor);

//blits the 'actor' onto the 'screen'
SDL_Rect jactor_paint(jactor *actor, SDL_Surface *screen, double frames_so_far);

/* returns 1 if actor1 is colliding with actor2 and fills the jcoll struct 
 * pointed to with information about the colission that has taken place */
int jactor_collision_detect(jactor *actor1, jactor *actor2, jcoll *c_info);


#endif  
