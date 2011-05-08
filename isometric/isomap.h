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

#include "isoeng.h"

struct isomap{
    SDL_Rect map_rect; /* SDL_Rect describing the position, width
                          and height of map */

    unsigned int groups; /* Groups that are associated with this
                            map (and should be painted when map
                            is painted */

    int w, h;          /* Size of map (tiles) */
    int tw, th;        /* Size of one tile (pixels) */
    int rw, rh;        /* Real size of the map (pixels) */

    double ri[2][3];      /* Real to isometric space projector */
    double ir[2][3];      /* Isometric to real space projector */

    unsigned char *map; /* 2D array of tile indicies */

    SDL_Surface *tilepalette; /* Tile palette image */

    GLuint texname;    /* Texture containing the map */
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
        double groups,
        int w, int h, int tw, const char *filename,
        const char *k, const char *m);

int isomap_from_string(struct isomap *map, const char *k, const char *m);

/* Paint the map onto the screen */
void isomap_paint(struct isomap *map, struct isoeng *engine, double frame);

#endif
