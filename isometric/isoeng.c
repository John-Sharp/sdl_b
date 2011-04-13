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

    for(p=group->list; p==NULL; p=p->next){
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

int isols_add_node(struct isols *ls, struct isoactor *actor)
{
    struct isols *next;
    next = ls->next;

    if((ls->next = malloc(sizeof(*(ls->next)))) == NULL){
                    fprintf(stderr, "Error allocating list node\n");
                    exit(1);
                }

    ls->next->next = next;
    ls->next->actor = actor;
   return 1;
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
    fprintf(stderr, "Actor uid: %d\n", actor->uid);
    fprintf(stderr, "Actor groups: %d\n", actor->groups);
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
    fprintf(stderr, "List of engine's actors:\n");
    for(q=engine->actor_ls; q!=NULL; q=q->next){
        fprintf(stderr, "%d, ", q->actor->uid);
    }
    fprintf(stderr, "\n");
#endif

    /* Put the actor in its groups */


    struct isogrp *marker = engine->grp_ls, *marker_pv = engine->grp_ls;
    int orig_groupnum = groupnum;
    while(groupnum != 0){
        int next_group;
        int fieldnum;

        next_group = trailing_zeroes(groupnum);
        /* Sets the last trailing_zeroes + 1 bits of groupnum
         * to be 0 */
        groupnum = ((groupnum  &  (~0<<(next_group + 1))));

        fieldnum = 1<<next_group;

#ifdef DEBUG_MODE
        fprintf(stderr, "Adding actor into group: %d\n", fieldnum);
#endif

        for(; marker != NULL; marker = marker->next){
            if(marker->grp_num > fieldnum){
                /* Need to add new group stub, before current marker */

                isogrp_add_node(marker_pv, fieldnum, actor); 
                marker_pv = marker;

                break;
            }else if(marker->grp_num == fieldnum){
                /* Need to add actor to this group */
                struct isols *lp, *lq;

                for(lp = marker->list; lp != NULL; lp = lp->next){
                    lq = lp;
                }
                isols_add_node(lq, actor);

#ifdef DEBUG_MODE
                fprintf(stderr, "Put actor in group: %d (pre-existing)\n",
                        fieldnum);
#endif
                marker_pv = marker;
                break;
            }else if(marker->grp_num & orig_groupnum){
                /* This is a composite group that the actor needs to be in */
                /* Need to add actor to this group */
                struct isols *lp, *lq;

                for(lp = marker->list; lp != NULL; lp = lp->next){
                    lq = lp;
                }
                isols_add_node(lq, actor);

/* Maybe need to put a debug message in here */
#ifdef DEBUG_MODE
                fprintf(stderr, "Adding actor into composite group %d\n",
                        fieldnum);
#endif


            }

            marker_pv = marker;
        }

        if(engine->grp_ls == NULL){
            engine->grp_ls = isogrp_add_node(NULL, fieldnum, actor);


        }else if(marker_pv->next == NULL && marker_pv->grp_num != fieldnum){
            isogrp_add_node(marker_pv, fieldnum, actor);
        }

    }
#ifdef DEBUG_MODE
    fprintf(stderr, "List of engine's groups: ");
    for(marker = engine->grp_ls; marker != NULL; marker = marker->next){
        fprintf(stderr, "%d, ", marker->grp_num);
    }
    fprintf(stderr, "\n");
#endif


    return actor;
}

struct isols *isoeng_get_group(struct isoeng *engine, int groupnum)
{
    struct isogrp *gp, *gq;
    struct isols *target, *newnode, *oldnode, *lp;

    /* Loop through all the engine's groups, checking to see whether 
     * the group has been previously created */
    for(gp = engine->grp_ls; gp != NULL; gp = gp->next){
        printf("gp num: %d\n", gp->grp_num);
        if(gp->grp_num == groupnum){
            printf("hello?\n");
            return gp->list;
        }
        else if(gp->grp_num > groupnum)
            break;
    }

    oldnode = newnode = NULL;

    /* The group has not been previously created and so must be made... */
    for(lp = engine->actor_ls; lp != NULL; lp = lp->next){
        if(lp->actor->groups & groupnum){ /* this should be included in
                                          composite group */
            fprintf(stderr, "Including actor %d\n", lp->actor->uid);
            if((newnode = malloc(sizeof(*newnode))) == NULL){
                fprintf(stderr, "Could not allocate memory for "
                        "composite group %d\n", groupnum);
                exit(1);
            }
            if(oldnode == NULL){
                target = newnode;
                oldnode = newnode;
            }else{
                printf("hojkjlk\n");
                oldnode->next = newnode;
            }
            newnode->actor = lp->actor;
            newnode->next = NULL;
        }
    }


    gq = NULL; /* Will become pointer to previous group node */

    /* Register new composite group with the engine */
    for(gp = engine->grp_ls; gp != NULL; gp = gp->next){
        if(gp->grp_num > groupnum){ /* Composite group needs to be added
                                       to list before this point */

            if(gq == NULL){
                if((engine->grp_ls = malloc(sizeof(*gp))) == NULL){
                    fprintf(stderr, "Could not allocate memory for "
                            " registering new composite group %d\n", groupnum);
                    exit(1);
                }
                engine->grp_ls->grp_num = groupnum;
                engine->grp_ls->list = target;
                engine->grp_ls->next = gp;

                return target;
            }
            if((gq->next = malloc(sizeof(*gq))) == NULL){
                fprintf(stderr, "Could not allocate memory for "
                        " registering new composite group %d\n", groupnum);
                exit(1);
            }
            gq->next->grp_num = groupnum;
            gq->next->list = target;
            gq->next->next = gp;
            return target;
        }
        gq = gp;
    }

    /* Composite group needs to be added onto end of group list */
    if((gq->next = malloc(sizeof(*gq))) == NULL){
        fprintf(stderr, "Could not allocate memory for "
                        " registering new composite group %d\n", groupnum);
                exit(1);
            }
    gq->next->grp_num = groupnum;
    gq->next->list = target;
    gq->next->next = gp;
    return target;


#ifdef DEBUG_MODE
    fprintf(stderr, "Should not reach here.\n");
#endif

    return NULL;
            
}

/* TODO */

struct isols *isols_del_node(struct isols *ls)
{
    if(ls->next == NULL){
        free(ls->actor);
        free(ls);
        return NULL;
    }

    ls->next = ls->next->next;
    ls->actor = ls->next->actor;
    free(ls->next);

    free(ls->next);

    return ls;
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

    return;
}

void struct isoeng_actor_drop(struct isoeng *engine, struct isoactor *actor,
        int groupnum)
{

    return;
}
