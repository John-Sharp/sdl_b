/* The main jaunty engine, responsible for keeping track of 
 * levels, actors, etc... */
#ifndef JEN_H
#define JEN_H
#include "SDL.h"
#include <SDL_image.h>
#include <GL/gl.h>
#include "jutils.h"

/* Constant that must be used to terminate list of groups
 * given to jen_get_groups */
#include <limits.h>
#define END_LIST INT_MAX

#include <stdarg.h>


typedef struct jactor_ls_ls jactor_ls_ls;
typedef struct jactor_ls jactor_ls;
typedef struct jmap jmap;


/* Defines what an actor is and functions that 
 * pertain to it */
#include "jactor.h"

#include "jmap.h"

#include "jplayer.h"

struct changed_states{
    unsigned int on;
    unsigned int off;
};

struct jactor_ls
{
    jactor *actor;
    jactor_ls *next;
};

struct jactor_ls_ls{
    jactor_ls *list;
    jactor_ls_ls *next;
    /* Id number of the group */
    unsigned char group_num; 
};

struct jen{
    /* List of groups of actors in game */
    jactor_ls_ls *actor_list;

    /* Number of actors in game */
    int num_actors;

    /* Number of groups in game */
    int num_groups;

    /* List of all actors in game */
    jactor_ls *actors;

    /* The current player who is playing */
    struct jplayer *current_player;

    /* Bitfield for describing the state of the game */
    unsigned int game_state;

};


/* Frees all resources allocated to 'engine' (including all actors who 
 * were catalogued) */
void jen_free(jen *engine);

/* Instantiates the jaunty engine */
jen *jen_create(void);

/* Adds an actor to the engine, returns the number of actors so far,
 * or -1 upon error */
int jen_add_jactor(jen *engine, jactor *actor, unsigned char group_num);

/* Deletes an actor from the engine, and the frees the resources from
 * that actor, returns the number of actors left or -1 */
int jen_del_jactor(jen *engine, jactor *actor);

/* Get the actor list that makes up group 'group_num', returns
 * NULL on error */
// Depreceated; can now use jen_get_groups
//jactor_ls *jen_get_group(jen *engine, unsigned char group_num);

jactor_ls *jen_get_groups(jen *engine, unsigned char group_num);

/* Detects changes in the engine's game state. Returns a
 * bitwise OR of all the states that weren't set the last
 * time the function was called */
struct changed_states jen_state_change_detect(struct jen *engine);

#endif
