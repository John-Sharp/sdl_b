/* isoactor.h 
 * 16-04-11
 * Functions for creating/animating/destroying a character in a game 
 *
 */

#ifndef ISOACTOR_H
#define ISOACTOR_H

#include "isomap.h"
#define MAX_SPRITES 15 /* Maximum number of sprite frames that an actor
                          can have */

struct isoactor{       /* A character in the game */
    unsigned int uid;           /* Unique identifier for actor */
    GLfloat x, y;               /* x, y coordinates of actor */
    GLfloat px, py;             /* x, y coordinates from previous frame */
    GLfloat gx, gy;             /* x, y coordinates of actor's interpolated
                                  position in fractional frame */

    double vx, vy;              /* x and y components of velocity */
    double ax, ay;              /* x and y components of acceleration */
                                   
    int w, h;                   /* Width and height of actor */
    int p2w, p2h;               /* Width and height of actor to nearest power
                                   of two (as required by openGL */

    unsigned int number_of_sprites; /* Total number of sprites associated with
                                       this actor */

    unsigned int show_sprite;   /* Number of the sprite that is to shown */

    GLuint textures[MAX_SPRITES]; /* Array of textures that are this actor's
                                     sprites */

    unsigned int groups;        /* Bit-field of the groups the
                                   actor is a member of */

    void (*i_handler)(struct isoactor *, \
            struct isoeng *); /* Iteration handler that gets called on
                                 each logic frame */
};

/* Frees the resources allocated to the actor */
void isoactor_free(struct isoactor *actor);

/* Creates a new actor, of width 'w', height 'h', using the image file located
 * at 'sprite_filename' to source the sprite images */ 
struct isoactor *isoactor_create(int w, int h, const char *sprite_filename);

/* Paint the actor, on the map 'map'. 'frame' is the frame that the game is 
 * currently on */
int isoactor_paint(struct isoactor *actor, struct isomap *map, double frame);

/* Function that gets called on each logic frame */
void isoactor_iterate(struct isoactor *actor, struct isoeng *engine);

#endif
