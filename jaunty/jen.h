/* The main jaunty engine, responsible for keeping track of 
 * levels, actors, etc... */
#ifndef JEN_H
#define JEN_H
#include "jactor.h"

typedef struct jactor_ls jactor_ls;
typedef struct jen jen;

struct jactor_ls
{
    jactor *actor;
    jactor_ls *next;
};


struct jen
{
    /* List of actors in game */
    jactor_ls *actors;
};


/* Frees all resources allocated to 'engine' (including all actors who 
 * were catalogued) */
void jen_free(jen *engine);

/* Instantiates the jaunty engine */
jen *jen_create(void);

/* Adds an actor to the engine, returns the number of actors so far,
 * or -1 upon error */
int jen_add_jactor(jen *engine, jactor *actor);

#endif
