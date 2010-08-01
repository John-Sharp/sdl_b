/* Keeps track of parts of a background map that has changed, for passing 
 * to SDL_UpdateRects for updating the screen. These so called
 * 'dirty rectangles' are stored in an array. This dirty array stores
 * the dirty rects from the last frame of the animation in its first
 * elements, followed by pointers to the dirty rects of the
 * current frame in the animation in the elements that follow. This allows
 * the cleaning up of 'trails' left by moving things if so desired. */ 

#ifndef JDIRTY_H
#define JDIRTY_H

#include "SDL.h"

/* Maximum of SDL_Rects that can be held for a frame */
#define DSIZE 10

/* makes the dirty rect array ready for use from scratch and returns
 * a pointer to the dirty rect array  */
SDL_Rect *jdirty_reset(void);

/* add a rectangle to the dirty rect array, returns number of rects stored in 
 * the array so far (including the last frame's) */
int jdirty_add(SDL_Rect rect);

/* returns the  number of rectangles stored in the array for the
 * previous frame/current frame respectively */
int jdirty_get_prev(void);
int jdirty_get_curr(void);

/* makes the current frame into the past frame */
void jdirty_iterate(void);

#endif
