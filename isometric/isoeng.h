/* isoeng.h
 * 06-03-11
 * Functions for creating/loading/destroying an isometric engine.
 *
 */

#ifndef ISOENG_H
#define ISOENG_H
#define DEBUG_MODE

struct isoactor{       /* A character in the game */
    int uid;           /* Unique identifier for actor */
    int groups;        /* Bit-field of the groups the actor is a member of */
};

struct isols{         /* List of actors */
    struct isoactor *actor; /* Pointer to actor */
    struct isols *next; /* Pointer to the next node of the list */
};

struct isogrp{        /* A group is a list of actors, this struct is 
                          a list of groups */
    int grp_num;       /* Identifier for this group */
    struct isols *list; /* The list associated with this group */
    struct isogrp *next; /* The next group */
};

struct isoeng{        /* Isometric engine */
    struct isols *actor_ls; /*Canonical list of actors in the game */
    struct isogrp *grp_ls; /* List of groups in the engine */
}; 

/* Frees all resources allocated to 'engine' */
void isoeng_free(struct isoeng *engine);

/* Creates the isomap engine */
struct isoeng *isoeng_create(void);

/* Creates a new actor inside the group(s)
 * satisfying the groupnum 'groupnum' in 
 * 'engine'. Returns reference to actor is this is successful or 0 if not */
struct isoactor *isoeng_new_actor(struct isoeng *engine, int groupnum);

/* Deletes the actor 'actor' and frees its resources */
void isoeng_del_actor(struct isoeng *engine, struct isoactor *actor);

/* Returns the group satisfying 'groupnum' */
struct isols *isoeng_get_group(struct isoeng *engine, int groupnum);

/* Drops 'actor' from groups referenced by 'groupnum'. If the actor is in
 * no more groups at all then the the actor will be freed completely */
void isoeng_actor_drop(struct isoeng *engine, struct isoactor *actor,
        int groupnum);

/* Adds 'actor' to the groups referenced by 'groupnum' */
void isoeng_actor_add(struct isoeng *engine, struct isoactor *actor,
        int groupnum);

#endif
