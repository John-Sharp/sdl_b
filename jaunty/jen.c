/* The main jaunty engine, responsible for keeping track of 
 * levels, actors, etc... */



#include "jen.h"
#include <stdlib.h>
#include <math.h>

void jen_free(struct jen *engine)
{
    jactor_ls *q = engine->actors;
    int i = 0;

    /* freeing the actor list */
    while(q != NULL){
        jen_del_jactor(engine, q->actor);
        q = engine->actors;
        i++;

    }
    fprintf(stderr, "jo%d\n", i);
    
    free(engine);
}

jen *jen_create(void)
{
    jen *jenp;

    if((jenp = malloc(sizeof(*jenp))) == NULL)
        return NULL;

    if((jenp->actor_list = malloc(sizeof(*(jenp->actor_list)))) == NULL)
        return NULL;

    jenp->actor_list->list = NULL;
    jenp->actor_list->next = NULL;

    if((jenp->actors = malloc(sizeof(*(jenp->actors)))) == NULL)
        return NULL;

    jenp->actors->actor = NULL;
    jenp->actors->next = NULL;
    jenp->num_groups = 0;

    return jenp;
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


int jen_add_jactor(jen *engine, jactor *actor, unsigned char group_num)
{
    int n = 0, i = 0;
    jactor_ls *p;
    jactor_ls_ls *q;
    int real_group_num = 0;

    actor->group_num = group_num;

    real_group_num = trailing_zeroes(group_num);


    /* Add the actor to the engine's main actor list */
    if((p=engine->actors)->actor == NULL){
        p->actor = actor;
        n++;
    }else{
        n = 1;
        for(;p->next != NULL; p=p->next){
            n++;
        }
        if((p->next = malloc(sizeof(*p))) == NULL)
            return -1;
        p = p->next;
        p->actor = actor;
        p->next = NULL;
    }

    q = engine->actor_list;

    /* Add a new node for this group's actor list, if this actor's 
     * group happens to be a new group */
    for(i = 0; i < real_group_num; i++){
        if(q->next == NULL){
            if((q->next = malloc(sizeof(*q))) == NULL)
                return -1;
            q = q->next;
            q->group_num = i;
            q->list = NULL;
            q->next = NULL;
        }else{
            q = q->next;
        }
    }

    if((p = q->list) == NULL){
        fprintf(stderr, "Adding new list; %d\n", group_num);
        q->group_num = group_num;
        if((p = q->list = malloc(sizeof(*p))) == NULL)
            return -1;
        p->next = NULL;
        p->actor = actor;
    }else{
        for(; p->next != NULL; p = p->next){
        }
        if((p->next = malloc(sizeof(*p))) == NULL)
            return -1;
        p = p->next;
        p->next = NULL;
        p->actor = actor;
    }

    /* Add the actor to any pre-existing groups that it
     * should belong to */
    for(q = engine->actor_list; q != NULL; q = q->next){
        if(group_num & q->group_num && group_num != q->group_num){
            fprintf(stderr, "Adding group %d to group %d\n", group_num, q->group_num);
            if(q->list){
                p = q->list->next;
                if((q->list->next = malloc(sizeof(*p))) == NULL){
                    fprintf(stderr, "Error allocating memory\n");
                    exit(1);
                }

                q->list->next->next = p;
                q->list->next->actor = actor;
            }else{
                if((q->list = malloc(sizeof(*p))) == NULL){
                    fprintf(stderr, "Error allocating memory\n");
                    exit(1);
                }
                q->list->next = NULL;
                q->list->actor = actor;
            }
        }
    }





    engine->num_groups = (engine->num_groups > real_group_num ?
            engine->num_groups : real_group_num);


    return n;
}

/* Drop actor 'a' from the actor list 'ls'. 
 * Returns 1 if successful, -1 if not. */
int jactor_ls_drop_actor(jactor_ls **lsp, jactor* a)
{
    jactor_ls *p, *q, *ls = *lsp;

    if(ls == NULL)
        return -1;

    /* If the first node contains the actor that should be deleted
     * then copy the actor stored in the second node to the first
     * and delete the second. Do this since the first node is the
     * reference to the list as a whole */
    if(ls->actor->uid == a->uid){
        if(ls->next != NULL){
            ls->actor = ls->next->actor;
            p = ls->next;
            ls->next = ls->next->next;
            free(p);
        }else{
            fprintf(stderr, "hanging round here\n");
            //free(lsp);
            fprintf(stderr, "here\n");
            *lsp = NULL;
        }

        return 1;
    }

    q = p = ls;
    for(p = ls->next; p != NULL; p = p->next){
        /* Check if this is the actor that should be dropped */
        if(p->actor->uid == a->uid){
            q->next = p->next; 
            free(p);
            return 1;
        }
        q = p;
    }

    return -1;
} 

int jen_del_jactor(jen *engine, jactor *actor)
{
    jactor_ls_ls *p;

    /* Delete the actor from the main game list */
    jactor_ls_drop_actor(&(engine->actors), actor);
    
    /* Delete the actor from all the groups
     * it happens to be in */
    for(p = engine->actor_list; p != NULL; p = p->next){
        if(p->group_num & actor->group_num){
            if(jactor_ls_drop_actor(&(p->list), actor) == -1){
                fprintf(stderr, "Error dropping actor\n");
                return -1;
            }
        }
    }



    /* Delete the actor itself */
    jactor_free(actor);

    /* Reduce the number of actors by one */
    engine->num_actors--;

    return engine->num_actors;
}

//Depreceated; can now just call jen_get_groups
/*
jactor_ls *jen_get_group(jen *engine, unsigned char group_num)
{
    jactor_ls_ls *p = engine->actor_list;
    int i, real_group_num;

    real_group_num = trailing_zeroes(group_num);

    for(i = 0; i < real_group_num; i++){
        p = p->next;
    }

    return p->list;
}
*/

/* Internal function for use by jen_get_groups that returns a dynamically
 * allocated copy of the group that must be freed later on. */
static jactor_ls *jen_get_group_copy(jen *engine, int real_group_num)
{
    jactor_ls_ls *p;
    jactor_ls *q, *r, *s;
    int i;
    int first_time = 1;

    p = engine->actor_list;

    for(i = 0; i < real_group_num; i++){
        p = p->next;
    }

    for(q = p->list; q != NULL; q = q->next){

        if(first_time){
            if((r = malloc(sizeof(*r))) == NULL){
                fprintf(stderr, "Out of memory. \n");
                exit(1);
            }
            first_time = 0;
            s = r;
        }else{
            if((r->next = malloc(sizeof(*r))) == NULL){
                fprintf(stderr, "Out of memory.\n");
                exit(1);
            }
            r = r->next;
        }
        r->actor = q->actor;
        r->next = NULL;
    }


    return s;
}

/* Internal function to construct a new actor list that is a composite
 * of a number of individual groups */
static jactor_ls *jen_get_groups_new(jen *engine, unsigned char group_num)
{
    jactor_ls *list, *list_last, *p;
    int next_group, first_time = 1;

    while(group_num != 0){
        next_group = trailing_zeroes(group_num);
        /* Sets the last trailing_zeroes + 1 bits of the group_num
         * to be 0 */
        group_num = ((group_num  &  (~0<<(next_group + 1))));

        if(first_time){
            list = jen_get_group_copy(engine, next_group);
            list_last = list;
            first_time = 0;
        }else{
            list_last->next = jen_get_group_copy(engine, next_group);
        }

        for(p = list_last; p->next != NULL; p = p->next){
            continue;
        }
        list_last = p;
    }

    return list;
}

jactor_ls *jen_get_groups(jen *engine, unsigned char group_num)
{
    jactor_ls_ls *q;

    /* Check if the group has already been calculated and 
     * catalogued, and if so return the group */
    for(q = engine->actor_list; q->next != NULL; q = q->next){
        if(q->group_num == group_num){
            return q->list;
        }
    }

    if(q->group_num == group_num){
        return q->list;
    }

    /* The group has not already been calculated so
     * dynamically allocate the group and append it 
     * to the engine's list of groups */

    if((q->next = malloc(sizeof(*q))) == NULL){
        fprintf(stderr, "Problem allocating memory "
                "for new group.\n");
        return NULL;
    }

    q = q->next;
    q->next = NULL;
    q->list = jen_get_groups_new(engine, group_num);
    q->group_num = group_num;

    return q->list;
}

struct changed_states jen_state_change_detect(struct jen *engine)
{
    static unsigned int last_state = 0;
    struct changed_states changed_states;
    unsigned int this_state = engine->game_state;

    changed_states.on = ~last_state & (last_state ^ this_state);
    changed_states.off = last_state & (last_state ^ this_state);

    last_state = this_state;

   return changed_states; 
}
