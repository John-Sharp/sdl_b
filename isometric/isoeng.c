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

void isoeng_free(struct isoeng *engine)
{
    struct isogrp *p;
    struct isols *q;

    for(p = engine->groups; p != NULL; p = p->next){
       isogrp_free(p);
    }

    for(q=engine->actors; q!=NULL; q=q->next){
        isoactor_free(q->actor);
        free(q);
    }

} 

void isogrp_free(struct isogrp *group)
{
    struct isols *p;

    for(p=group->ls; p!=NULL; p=p->next){
        free(p);
    }
}

void initialise_video(struct isoeng *engine, unsigned int win_w,
        unsigned int win_h)
{
    int flags = SDL_OPENGL;
    const SDL_VideoInfo *info = NULL;
    
    if(SDL_Init(SDL_INIT_VIDEO) == -1){
        fprintf(stderr, "Video initialisation failed: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    info = SDL_GetVideoInfo();
    engine->screen = SDL_SetVideoMode(win_w, win_h, info->vfmt->BitsPerPixel, flags);
    if(!engine->screen){
        fprintf(stderr, "Failed to open screen!\n");
        exit(1);
    }

    /* OpenGL initialisation */
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, win_w, win_h, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDisable(GL_DEPTH);  /* This is a 2d program, no need for depth test */

    return;
}

struct isoeng *isoeng_create(unsigned int win_w, unsigned int win_h)
{
    struct isoeng *engine;

    if((engine=malloc(sizeof(*engine)))==NULL){
        fprintf(stderr, "Error allocating memory for game engine\n");
        exit(1);
    }

    engine->actors = NULL;
    engine->groups = NULL;

    initialise_video(engine, win_w, win_h);

    return engine;
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


struct isoactor *isoeng_new_actor(struct isoeng *engine, int w, int h,
        const char *sprite_file_name, unsigned int groupnum)
{
    struct isoactor *actor;
    struct isols *p, *q = NULL;
    static unsigned int uid = 0;

    actor = isoactor_create(w, h, sprite_file_name);

    for(p = engine->actors; p != NULL; p = p->next){
        q = p;
    }

    actor->uid = uid;
    uid++;
    actor->groups = groupnum;

#ifdef DEBUG_MODE
    fprintf(stderr, "\nCreating actor %d\n", actor->uid);
#endif

    /* Put the actor in the global actor list */
    engine->actors = isols_add_actor(engine->actors, actor);

#ifdef DEBUG_MODE
    fprintf(stderr, "List of engine's actors: ");
    for(q=engine->actors; q!=NULL; q=q->next){
        fprintf(stderr, "%d, ", q->actor->uid);
    }
    fprintf(stderr, "\n");
#endif

    /* Put the actor in its groups */
    struct isogrp *gp;

    for(gp = engine->groups; gp != NULL; gp = gp->next){
        if(groupnum & gp->groupnum){
            gp->ls = isols_add_actor(gp->ls, actor);
        }
    }

    return actor;
}

/* Returns a ls of all actors in ls with a actor->groupnum that satisfies
 * a bitwise AND with groupnum */
struct isols *isols_matching_groupnum(struct isols *ls, unsigned int groupnum)
{
    if(ls == NULL){
        return NULL;
    }

    if(ls->actor->groups & groupnum){
        struct isols *newnode;

        if((newnode = malloc(sizeof(*newnode))) == NULL){
            fprintf(stderr, "Could not allocate memory for group %d\n",
                    groupnum);
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
        if((grp = malloc(sizeof(*grp))) == NULL){
            fprintf(stderr, "Could not allocate memory for "
                    "registering new composite group %d\n", groupnum);
            exit(1);
        }
        grp->ls = ls;
        grp->next = NULL;
        grp->groupnum = groupnum;

        return grp;
    }


    if(grp->groupnum > groupnum){
        if((p = malloc(sizeof(*p))) == NULL){
            fprintf(stderr, "Could not allocate memory for "
                    " registering new composite group %d\n", groupnum);
            exit(1);
        }
        p->ls = grp->ls;
        p->next = grp->next;
        p->groupnum = grp->groupnum;
        grp->ls = ls;
        grp->next = p;
        grp->groupnum = groupnum;

        return grp;
    }

    grp->next = isogrp_add_ls(grp->next, ls, groupnum);
    return grp->next;
}

struct isogrp *isoeng_get_group(struct isoeng *engine, unsigned int groupnum)
{
    struct isogrp *gp;
    struct isols *target;

    /* Loop through all the engine's groups, checking to see whether 
     * the group has been previously created */
    for(gp = engine->groups; gp != NULL; gp = gp->next){
        if(gp->groupnum == groupnum){
            return gp;
        }
        else if(gp->groupnum > groupnum)
            break;
    }

    /* The group has not been previously created and so must be made... */
    target = isols_matching_groupnum(engine->actors, groupnum);

    gp = isogrp_add_ls(engine->groups, target, groupnum);
    if(engine->groups == NULL){
        engine->groups = gp;
    }

    return(gp);
}

struct isols *isols_del_actor(struct isols *ls, struct isoactor *actor)
{
    if(ls == NULL){
        return NULL;
    }

    if(ls->actor->uid == actor->uid){
        struct isols *p = ls->next;
        free(ls);
        return p;
    }

    ls->next = isols_del_actor(ls->next, actor);
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
    struct isogrp *p;

#ifdef DEBUG_MODE
    fprintf(stderr, "Deleting actor %d\n", actor->uid);
#endif

    for(p = engine->groups; p != NULL; p = p->next){
        if(p->groupnum & actor->groups){
           p->ls = isols_del_actor(p->ls, actor);

#ifdef DEBUG_MODE
           if(p->ls == NULL){ 
               fprintf(stderr, "List has been completely deleted\n");
           }
#endif
       }
    }

    engine->actors = isols_del_actor(engine->actors, actor);
    free(actor);

    return;
}

void isoeng_actor_drop(struct isoeng *engine, struct isoactor *actor,
        unsigned int groupnum)
{
    struct isogrp *pg;

    for(pg = engine->groups; pg != NULL; pg = pg->next){
        if(actor->groups & pg->groupnum && pg->groupnum & groupnum){
            printf("dropping actor %d\n", pg->groupnum);
            pg->ls = isols_del_actor(pg->ls, actor);
        }
    }

    actor->groups ^= (actor->groups & groupnum);
    printf("now in group: %d", actor->groups);

    return;
}

void isoeng_actor_add(struct isoeng *engine, struct isoactor *actor,
        unsigned int groupnum)
{
    struct isogrp *pg;

    for(pg = engine->groups; pg != NULL; pg = pg->next){
        if(!(actor->groups & pg->groupnum) && pg->groupnum & groupnum){
            pg->ls = isols_add_actor(pg->ls, actor);
        }
    }

    actor->groups |= groupnum;

    return;
}
