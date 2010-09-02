//jmap.h
//Functions for creating/loading/drawing/destroying quite a generic map.

#ifndef	JMAP_H
#define	JMAP_H

#include "SDL.h"
#include "jactor.h"
#include "jen.h"

//typedef struct jcoll jcoll;
//typedef enum jsides jsides;

typedef struct jmap
{
	int		w, h;		/* Size of map (tiles) */
    int p2w, p2h;       /* Real size of map in pixels, rounded up to the
                         * power of two as required by openGL */
	unsigned char *map;		/* 2D array of tile indices */
    unsigned char *c_map; /*2D array of collision tags*/

	int		tw, th;		/* Size of one tile (pixels) */

	SDL_Surface	*tilepalette;		/* Tile palette image */

    GLuint tex_name; /* texture containing the map */
} jmap;



//frees all resources allocated to 'map'
void jmap_free(jmap *map);

//creates a map, 'w' tiles wide by 'h' tiles high,
//returns a pointer to this map, or NULL if the 
//allocation failed.
jmap *jmap_create(int w, int h);

//loads a tilepalette for 'map', from the png file 'filename', which should
//contain a series of background tiles 'tw' pixels wide by 'th' high.
//Returns 0 if successful or -1 if some sort of error occured
int jmap_load_tilepalette(jmap *map, const char *filename, int tw, int th);


//Takes a string of characters 'k' (the key) whose first character
//corresponds to the tile of index 0 in 'map's tilepalette, second
//character to index 1 in 'map's tilepalette and so on.
//
//Takes a second string 'm', whose length is the total number of tiles
//that make up 'map' (i.e. 'map'.w * 'map'.h), and which is exclusively
//made up of characters contained in the key 'k'.
//
//Fills up the 2D array of tile indices, 'map'.map, by systematically
//reading through 'm' and for each character using the tile index that
//corresponds to that character in the key 'k'.    
void jmap_from_string(jmap *map, const char *k, const char *m);

//Same as above, but for a collision map, as opposed to the background
//image dealt with by the previous function
void jmap_c_map_from_string(jmap *map, const char *k, const char *m);

/* Detects if 'actor' has collided with a tile matching the 'tile_mask' 
 * on 'map' in this logic frame. Returns the jside of this collision
 * and if c_info is not null populates the collision info with information
 * about the collision */
jsides jmap_collision_detect(jmap *map, jactor *actor, jcoll *c_info,
        int tile_mask);

//returns the index of the first tile touched, going from (x1, y1) to 
//(x2, y2), that matches the description provided in the 'tile_mask'
int jmap_first_tile_touched(jmap *map, double x1, double y1,
        double x2, double y2, int tile_mask);

//returns the side of the background tile 'tile_index' that is intersected
//by the line between (x1, y1) and (x2, y2) (or NULL if no intersection
//takes place. The addresses pointed to by 'x' and 'y' are set to the 
//point of intersection
jsides jmap_tile_intersection_point(jmap *map, unsigned char tile_index,
        double x1, double y1, double x2, double y2, 
        double *x, double *y);

//paints the map onto the screen
void jmap_paint(jmap *map);


#endif


