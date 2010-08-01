/* The main jaunty engine, responsible for keeping track of 
 * levels, actors, etc... */
#include "jen.h"
#include "jactor.h"
#include <stdlib.h>

void jen_free(jen *engine)
{
    jactor_ls *p, *q = engine->actors;

    /* freeing the actor list */
    
    while(q != NULL){
        p = q->next;
        jactor_free(q->actor);
        free(q);
        q = p;
    }

    free(engine);

}

jen *jen_create(void)
{
    jen *jenp;

    if((jenp = malloc(sizeof(*jenp))) == NULL)
        return NULL;

    jenp->actors = NULL;

    return jenp;
}


int jen_add_jactor(jen *engine, jactor *actor)
{
    int n = 0;
    jactor_ls *p;

    if(engine->actors == NULL){
        if((engine->actors = malloc(sizeof(*(engine->actors)))) == NULL)
            return -1;
        (engine->actors)->actor = actor;
        (engine->actors)->next = NULL;
        return n;
    }

    for(p = engine->actors; p->next != NULL; p = p->next){
        n++;
    }

    
    if((p->next = malloc(sizeof(*p))) == NULL)
       return -1;

    p = p->next;
    p->actor = actor;
    p->next = NULL;

    return n;
}

