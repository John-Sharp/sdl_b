#include "SDL.h"
#include <stdlib.h>
#include "jdirty.h"

/* variables for keeping track of number of dirty rectangles in both the
 * old and the new frames */
static int jdirty_prev = 0;
static int jdirty_curr = 0;

/* array with enough room for an array of dirty rectangles in both the
 * old and the new frames */
static SDL_Rect jdirty_array[2*DSIZE];


SDL_Rect *jdirty_reset(void)
{
    jdirty_prev= 0;
    jdirty_curr= 0;

    return jdirty_array;
}

int jdirty_add(SDL_Rect rect){
    if(jdirty_prev + jdirty_curr > 2 * DSIZE -1)
        return -1;
    jdirty_array[jdirty_prev + jdirty_curr] = rect;
    jdirty_curr++;
    
    return jdirty_curr + jdirty_prev;
}

void jdirty_iterate(void)
{
    memmove(jdirty_array, jdirty_array + jdirty_prev,
            (sizeof(jdirty_array)/(2 * DSIZE)) * jdirty_curr); 
    jdirty_prev = jdirty_curr;
    jdirty_curr = 0;

    return;
}


int jdirty_get_prev(void)
{
    return jdirty_prev;
}

int jdirty_get_curr(void)
{
    return jdirty_curr;
}

