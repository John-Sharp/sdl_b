/* isoeng.c
 * 09-03-11
 * Functions for creating/loading/destroying an isometric engine.
 *
 */
#include "isoeng.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void isogrp_free(struct isogrp *group);
static void isoactor_free(struct isoactor *actor);

void isoeng_free(struct isoeng *engine)
{
    struct isogrp *p;
    struct isols *q;

    for(p = engine->grp_ls; p != NULL; p = p->next){
       isogrp_free(p);
    }

    for(q=engine->actor_ls; q!=NULL; q=q->next){
        isoactor_free(q->actor);
        free(q);
    }

} 

void isogrp_free(struct isogrp *group)
{
    struct isols *p;

    for(p=group->list; p!=NULL; p=p->next){
        free(p);
    }
}

void isoactor_free(struct isoactor *actor)
{
#ifdef DEBUG_MODE
    fprintf(stderr, "Deleting actor uid: %d\n", actor->uid);
#endif

    free(actor);
}

struct isoeng *isoeng_create(void)
{
    struct isoeng *engine;

    if((engine=malloc(sizeof(*engine)))==NULL){
        fprintf(stderr, "Error allocating memory for game engine\n");
        exit(1);
    }

    engine->actor_ls = NULL;
    engine->grp_ls = NULL;

    return engine;
}

/* Function that returns the number of consecutive  0's
 * that trail a binary integer. Used in the conversion
 * of the bitfields that are used to reference actor 
 * groups into integers that indicate the position of the 
 * actor group in the actor group list. */  
static int trailing_zeroes(unsigned char group_num)
{
    /* Number of trailing zeroes */
    int tz_num = 0;

    for(; (group_num & 1) != 1; group_num >>= 1){
        tz_num += 1;
    }

    return tz_num;
}


static struct isogrp *isogrp_add_node(struct isogrp *grp, int grp_num,
        struct isoactor *actor)
{
    struct isogrp *next;
    struct isogrp **newnode;
    struct isogrp *nn;

    if(grp == NULL){
        next = NULL;
        newnode = &nn;
    }else{
        next = grp->next;
        newnode = &(grp->next);
    }

    /* Allocate group stub */
    if((*newnode = malloc(sizeof(*(*newnode)))) == NULL){
        fprintf(stderr, "Error allocating node for group %d\n", grp_num);
        exit(1);
    }

    (*newnode)->next = next;
    (*newnode)->grp_num = grp_num;

    /* Add list to group stub */
    if(((*newnode)->list = malloc(sizeof(*((*newnode)->list)))) == NULL){
        fprintf(stderr, "Error allocating list for group %d\n", grp_num);
        exit(1);
    }

    (*newnode)->list->next = NULL;

    /*  Add actor to list */
    (*newnode)->list->actor = actor;

#ifdef DEBUG_MODE

    if(grp == NULL)
        fprintf(stderr, "Put actor in group: %d (start stub)\n", grp_num);
    else
        fprintf(stderr, "Put actor in group: %d (new stub)\n", grp_num);
#endif

    return *newnode;
}

struct isols *isols_add_actor(struct isols *ls, struct isoactor *actor)
{
    struct isols *next;
    next = ls;

    if((ls = malloc(sizeof(*(ls)))) == NULL){
                    fprintf(stderr, "Error allocating list node\n");
                    exit(1);
                }

    ls->next = next;
    ls->actor = actor;
   return ls;
} 


struct isoactor *isoeng_new_actor(struct isoeng *engine, int groupnum)
{
    struct isoactor *actor;
    struct isols *p, *q=NULL;
    int uid=0;

    if((actor=malloc(sizeof(*actor)))==NULL){
        fprintf(stderr, "Error allocating memory for actor.\n");
        exit(1);
    }

    for(p=engine->actor_ls; p!=NULL; p=p->next){
        q=p;
        uid++;
    }

    actor->uid=uid;
    actor->groups=groupnum;

#ifdef DEBUG_MODE
    fprintf(stderr, "\nCreating actor %d\n", actor->uid);
#endif

    /* Put the actor in the global actor list */
    if(q==NULL){
        if((engine->actor_ls=malloc(sizeof(*(engine->actor_ls))))==NULL){
            fprintf(stderr, "Error allocating memory for actor list.\n");
            exit(1);
        }
        p=engine->actor_ls;
    }else{
        if((q->next=malloc(sizeof(*q)))==NULL){
            fprintf(stderr, "Error allocating memory for actor list.\n");
            exit(1);
        }
        p=q->next;
    }

    p->actor=actor;
    p->next=NULL;

#ifdef DEBUG_MODE
    fprintf(stderr, "\nList of engine's actors: ");
    for(q=engine->actor_ls; q!=NULL; q=q->next){
        fprintf(stderr, "%d, ", q->actor->uid);
    }
    fprintf(stderr, "\n");
#endif

    /* Put the actor in its groups */
    struct isogrp *gp;

    for(gp = engine->grp_ls; gp != NULL; gp = gp->next){
        if(groupnum & gp->grp_num){
            gp->list = isols_add_actor(gp->list, actor);
        }
    }

    return actor;
}

/* Returns a list of all actors in ls with a actor->groupnum that satisfies
 * a bitwise AND with groupnum */
struct isols *isols_matching_groupnum(struct isols *ls, unsigned int groupnum)
{
    if(ls == NULL){
        return NULL;
    }

    if(ls->actor->groups & groupnum){
        struct isols *newnode;

        if((newnode = malloc(sizeof(*newnode))) == NULL){
            fprintf(stderr, "Could not allocate memory for group %d\n", groupnum);
        }
        newnode->actor = ls->actor;
        newnode->next = isols_matching_groupnum(ls->next, groupnum);
        return newnode;
    }

    return isols_matching_groupnum(ls->next, groupnum);
}

/* Retuns a new group containing the list 'ls' and the list of groups will
 * be ordered with asscending groupnums */
struct isogrp *isogrp_add_ls(struct isogrp *grp, struct isols *ls,
        unsigned int groupnum)
{
    struct isogrp *p;


    if(grp == NULL){
    fprintf(stderr, "ADDING... %d\n", groupnum);
        if((grp = malloc(sizeof(*grp))) == NULL){
            fprintf(stderr, "Could not allocate memory for "
                    "registering new composite group %d\n", groupnum);
            exit(1);
        }
        grp->list = ls;
        grp->next = NULL;
        grp->grp_num = groupnum;

        return grp;
    }


    if(grp->grp_num > groupnum){
        if((p = malloc(sizeof(*p))) == NULL){
            fprintf(stderr, "Could not allocate memory for "
                    " registering new composite group %d\n", groupnum);
            exit(1);
        }
        p->list = grp->list;
        p->next = grp->next;
        p->grp_num = grp->grp_num;
        grp->list = ls;
        grp->next = p;
        grp->grp_num = groupnum;

        return grp;
    }

    return isogrp_add_ls(grp->next, ls, groupnum);
}

struct isogrp *isoeng_get_group(struct isoeng *engine, int groupnum)
{
    struct isogrp *gp;
    struct isols *target;

    /* Loop through all the engine's groups, checking to see whether 
     * the group has been previously created */
    for(gp = engine->grp_ls; gp != NULL; gp = gp->next){
        printf("gp num: %d\n", gp->grp_num);
        if(gp->grp_num == groupnum){
            printf("hello?\n");
            return gp;
        }
        else if(gp->grp_num > groupnum)
            break;
    }

    /* The group has not been previously created and so must be made... */
    target = isols_matching_groupnum(engine->actor_ls, groupnum);
    gp = isogrp_add_ls(engine->grp_ls, target, groupnum);
    if(engine->grp_ls == NULL){
        fprintf(stderr, "grp_ls was NULL\n");
        engine->grp_ls = gp;
    }

    return(gp);
    

#ifdef DEBUG_MODE
    fprintf(stderr, "Should not reach here.\n");
#endif

    return NULL;
            
}

/* TODO */

struct isols *isols_del_actor(struct isols *ls, struct isoactor *actor)
{
    if(ls == NULL){
        return NULL;
    }

        fprintf(stderr, "uid is %d\n", ls->actor->uid);
    if(ls->actor->uid == actor->uid){
        struct isols *p = ls->next;
        free(ls);
        fprintf(stderr, "p is %d\n", p);
        return p;
    }

    ls->next = isols_del_actor(ls->next, actor);
    return ls;
}

struct isogrp *isogrp_del_node(struct isogrp **grpp)
{
    struct isogrp *grp = *grpp;

    if(grp->next == NULL){
        free(grp->list);
        free(grp);
        *grpp = NULL;
        return NULL;
    }

    grp->next = grp->next->next;
    grp->list = grp->next->list;
    grp->grp_num = grp->next->grp_num;

    free(grp->list);
    free(grp->next);

    return grp;
}

struct isols *isols_find_actor(struct isols *ls, struct isoactor *actor)
{
    struct isols *p;

    for(p = ls; p != NULL; p = p->next){
        if(p->actor->uid == actor->uid){
            return p;
        }
    }
    return NULL;
}

void isoeng_del_actor(struct isoeng *engine, struct isoactor *actor)
{
    struct isogrp *p;

#ifdef DEBUG_MODE
    fprintf(stderr, "Deleting actor %d\n", actor->uid);
#endif

    for(p = engine->grp_ls; p != NULL; p = p->next){
        
            fprintf(stderr, "HOUJIOUL %d", p->grp_num);
        if(p->grp_num & actor->groups){

            fprintf(stderr, "HOUJIOUL %d", p->grp_num);
           p->list = isols_del_actor(p->list, actor);
           printf("pnext: %d\n", p->list->next);

           if(p->list == NULL){ 
#ifdef DEBUG_MODE
               fprintf(stderr, "List has been completely deleted\n");
#endif
           }
       }
    }

    isols_del_actor(engine->actor_ls, actor);
    free(actor);

    return;
}

void isoeng_actor_drop(struct isoeng *engine, struct isoactor *actor,
        int groupnum)
{

    return;
}
