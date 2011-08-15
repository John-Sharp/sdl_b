/* isoeng.h
 * 06-03-11
 * Functions for creating/loading/destroying an isometric engine.
 *
 */

#ifndef ISOENG_H
#define ISOENG_H
#define DEBUG_MODE

#include <math.h>
#include <limits.h>
#include <GL/gl.h>
#include "SDL.h"
#include <SDL_image.h>


/* Type that is used to store the bit-fields in the
 * isoeng */
typedef unsigned long isobf_t;
/* How many 'isoeng_bf_t's are used to store one row
 * of the bit-fields for the map bit-fields */
#define MAP_BF_W 4
/* Width, in bits, of the 'isobf_t' type */
#define ISO_BFBW (CHAR_BIT * sizeof(isobf_t))


/* returns a number that is  a number, 'a', converted into the nearest number
 * that is a whole power of 2 (rounding up) */ 
#define mkp2(a) (int)powf(2.0, ceilf(logf((float)a)/logf(2.0)))

/* A pair of macros to give the (x, y) components of the projection
 * of (r_x, r_y) using the projector matrix A */
#define project_x(A, r_x, r_y) \
    ((double)A[0][0] * (double)r_x + (double)A[0][1] * (double)r_y) 

#define project_y(A, r_x, r_y) \
    ((double)A[1][0] * (double)r_x + (double)A[1][1] * (double)r_y) 

/* A pair of macros to give the (x, y) components of the projection
 * and translation of (r_x, r_y) using the augmented matrix A */
#define project_a_x(A, r_x, r_y) \
    ((double)A[0][0] * (double)r_x \
        + (double)A[0][1] * (double)r_y + (double)A[0][2])

#define project_a_y(A, r_x, r_y) \
    ((double)A[1][0] * (double)r_x \
     + (double)A[1][1] * (double)r_y + (double)A[1][2])

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	enum { RMASK = 0xff000000, GMASK = 0x00ff0000,
        BMASK = 0x0000ff00,	AMASK = 0x000000ff};
#else
	enum { RMASK = 0x000000ff,	GMASK = 0x0000ff00,
        BMASK = 0x00ff0000,	AMASK = 0xff000000};
#endif

struct isols{         /* List of actors */
    struct isoactor *actor; /* Pointer to actor */
    struct isols *next; /* Pointer to the next node of the list */
};

struct isogrp{        /* A group is a list of actors, this struct is 
                         a list of groups */
    unsigned int groupnum;       /* Identifier for this group */
    struct isols *ls; /* The list associated with this group */
    struct isogrp *next; /* The next group */
};

struct isoeng{        /* Isometric engine */
    int ctw, cth;         /* Width and height of tiles that make up 
                             collision maps used by actors */
    struct isols *actors; /* Canonical list of actors in the game */
    struct isogrp *groups; /* List of groups in the engine */

    SDL_Surface *screen; /* Main game window surface */
}; 

struct isoactor;
struct isomap;

/* Utility function to get a pixel at ('x', 'y') from a surface */
Uint32 get_pixel(SDL_Surface *surface, int x, int y);

#include "isomap.h"
#include "isoactor.h"


/* Frees all resources allocated to 'engine' */
void isoeng_free(struct isoeng *engine);

/* Creates the isomap engine */
struct isoeng *isoeng_create(unsigned int win_w, unsigned int win_h,
        int ctw, int cth);

/* Creates a new actor inside the group(s) satisfying the group number
 * 'groupnum' in 'engine'. Actor has width 'w', height 'h' and the image
 * file 'sprite_filename' contains the sprites to be used for this actor.
 * Image file 'c_sprite_filename' contains the collision sprites to be
 * used for this actor, and these sprites are of width 'cw' height 'ch'.
 * Returns reference to actor is this is successful or 0 if not. */
struct isoactor *isoeng_new_actor(struct isoeng *engine, int w, int h,
        const char *sprite_filename, int cw, int ch,
        const char *c_sprite_filename, unsigned int groupnum);

/* Deletes the actor 'actor' and frees its resources */
void isoeng_del_actor(struct isoeng *engine, struct isoactor *actor);

/* Returns the group satisfying 'groupnum' */
struct isogrp *isoeng_get_group(struct isoeng *engine, unsigned int groupnum);

/* Drops 'actor' from groups referenced by 'groupnum'. If the actor is in
 * no more groups at all then the the actor will be freed completely */
void isoeng_actor_drop(struct isoeng *engine, struct isoactor *actor,
        unsigned int groupnum);

/* Adds 'actor' to the groups referenced by 'groupnum' */
void isoeng_actor_add(struct isoeng *engine, struct isoactor *actor,
        unsigned int groupnum);

void printbitssimple(unsigned long n);
#endif
