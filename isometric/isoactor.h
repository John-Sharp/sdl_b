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


enum isoside{ /* Contains the sides of a tile that can be collided with */
    LEFT,
    RIGHT,
    TOP,
    BOTTOM 
};

struct map_handle_ls{ /* A list of map collision handlers */
    struct map_handle_ls *next;
    struct isomap *map;
    unsigned int tiles; /* Bit-wise OR of all tiles that this collision handler
                           should be fired for */
    void (*map_handler)(struct isoactor *actor, struct isomap *map,
            unsigned int tile, enum isoside side); /* Pointer to the 
                                                      collision handler
                                                      function */
};

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

    struct map_handle_ls *map_handlers[MAX_MAPS];
                                /* Array of handle lists (one for each
                                   map), each list contains a list of 
                                   collision handlers detailing what should
                                   be done for collision with certain tiles
                                   on the map */ 
};

/* Frees the resources allocated to the actor */
void isoactor_free(struct isoactor *actor);

/* Creates a new actor, of width 'w', height 'h', using the image file located
 * at 'sprite_filename' to source the sprite images */ 
struct isoactor *isoactor_create(int w, int h, const char *sprite_filename);

/* Sets a collision handler for the actor. The handler is such that it 
 * will be called everytime 'actor' steps on one of 'tiles' in 'map'.
 * The tiles are given as a string in the same way as the map gets
 * specified, and the same key as was used to specify 'map' is used
 * to look up the tiles */ 
void set_map_handler(struct isoactor *actor, struct isomap *map, 
        const char *tiles,
        void (*handler)(struct isoactor *,
                        struct isomap *,
                        unsigned int, enum isoside));

/* Unsets the handler 'handle' that gets fired when 'actor' steps on
 * one of the 'tiles' in 'map' */
void uset_map_handler(struct isoactor *actor, struct isomap *map, 
        const char *tiles,
        void (*handler)(struct isoactor *,
                        struct isomap *,
                        unsigned int, enum isoside));

/* Paint the actor, on the map 'map'. 'frame' is the frame that the game is 
 * currently on */
int isoactor_paint(struct isoactor *actor, struct isomap *map, double frame);

/* Function that gets called on each logic frame */
void isoactor_iterate(struct isoactor *actor, struct isoeng *engine);

#endif
