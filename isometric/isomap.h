/* isomap.h
 * 27-02-11
 * Functions for creating/loading/drawing/destroying an isometric map.
 *
 */

#ifndef JMAP_H
#define JMAP_H

#include <GL/gl.h>
#include "SDL.h"
#include <SDL_image.h>
#include <string.h>
#include <math.h>

#define MAX_MAPS 15 /* Maximum number of maps that can be handled
                       by the game engine */

#include "isoeng.h"



struct isomap{
    unsigned int uid;  /* Unique identifier of the map */

    SDL_Rect map_rect; /* SDL_Rect describing the position, width
                          and height of map */

    unsigned int groups; /* Groups that are associated with this
                            map (and should be painted when map
                            is painted */

    int w, h;          /* Size of map (tiles) */
    int itw;           /* Size of side of one tile */
    int tw, th;        /* Size of one tile sprite (pixels) */
    int rw, rh;        /* Real size of the map (pixels) */

    double ri[2][3];      /* Real to isometric space projector */
    double ir[2][3];      /* Isometric to real space projector */

    unsigned char *map; /* 2D array of tile indicies */

    char *c_key;        /* Key that translates tile bit-masks
                             to letters */
    unsigned int *c_map; /* 2D array of collision tile bit-masks */

    SDL_Surface *tilepalette; /* Tile palette image */

    GLuint texname;    /* Texture containing the map */

    void (*i_handler)(struct isomap *,
                      struct isoeng *); /* Iteration handler that gets called
                                           on each logic frame */
    void (*ob_handler)(struct isomap *,
                       struct isoactor *); /* Handler that gets called when
                                              actor strays over the map's
                                              boundary */
};

/* Frees all resource allocated to 'map' */
void isomap_free(struct isomap *map);

/* Creates a map, located in 'map_rect', 'w' tiles wide
 * by 'h' tiles high. Actors with group number satisfying
 * 'groups' as associated with this map. The map is made
 * out of tiles 'tw' x 'tw' / sqrt(3) pixels big. The
 * constituent tile images are located in the file
 * 'filename', these tiles are referenced by letters in
 * the key-string 'k' and the map is described
 * by the order of the letters in the map-string 'm'. Returns
 * a pointer to this map, or NULL if the allocation failed */
struct isomap *isomap_create(const SDL_Rect *map_rect,
        double groups, int w, int h, int tw,
        const char *filename, const char *k, const char *m,
        const char *ck, const char *cm);

int isomap_from_string(struct isomap *map, const char *k, const char *m);

/* Paint the map onto the screen */
void isomap_paint(struct isomap *map, struct isoeng *engine, double frame);

/* Default isomap iteration handler that passes information on tile
 * collision to the actors on the map */
void isomap_iterate(struct isomap *map, struct isoeng *engine);

/* Default over-boundary handler that simply stops the actor */
void isomap_ob_handler(struct isomap *map, struct isoactor *actor);

#endif
