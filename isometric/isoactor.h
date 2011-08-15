/* isoactor.h 
 * 16-04-11
 * Functions for creating/animating/destroying a character in a game 
 *
 */

#ifndef ISOACTOR_H
#define ISOACTOR_H

#include "isomap.h"
#define MAX_SPRITES 15 /* Maximum number of sprite frames that an actor
                          can have */


enum isoside{ /* Contains the sides of a tile that can be collided with */
    LEFT,
    RIGHT,
    TOP,
    BOTTOM 
};

struct map_handle_ls{ /* A list of map collision handlers */
    struct map_handle_ls *next;
    struct isomap *map;
    isobf_t tiles; /* Bit-wise OR of all tiles that this collision handler
                           should be fired for */
    void (*map_handler)(struct isoactor *actor, struct isomap *map);
                           /* Pointer to the 
                              collision handler
                              function */
};

struct actor_handle_ls{ /* A list of actor-actor collision handlers */
    struct actor_handle_ls *next;
    isobf_t groups; /* Groups that this handler applies to */
    void (*actor_handler)(struct isoactor *a1, struct isoactor *a2);
                                                /* Pointer to the actor
                                                   collision handler
                                                   function */
}; 

struct isoactor_overlap_l{ /* Struct for describing the linear overlap
                              of two actors */
    double a1_offset;    /* Offset of actor 1 from the start of the overlap */
    double a2_offset;    /* Offset of actor 2 from the start of the overlap */
    double overlap;      /* Size of the overlap */
};

struct isoactor_overlap{  /* Struct for describing the overlap of two actors */
    struct isoactor_overlap_l x; /* Linear overlap in x-direction */
    struct isoactor_overlap_l y; /* Linear overlap in y-direction */
};

struct isoactor{       /* A character in the game */
    unsigned int uid;           /* Unique identifier for actor */
    GLfloat x, y;               /* x, y coordinates of actor */
    GLfloat px, py;             /* x, y coordinates from previous frame */
    GLfloat gx, gy;             /* x, y coordinates of actor's interpolated
                                  position in fractional frame */

    double vx, vy;              /* x and y components of velocity */
    double ax, ay;              /* x and y components of acceleration */
                                   
    int w, h;                   /* Width and height of actor */
    int p2w, p2h;               /* Width and height of actor to nearest power
                                   of two (as required by openGL */

    unsigned int number_of_sprites; /* Total number of sprites associated with
                                       this actor */

    unsigned int show_sprite;   /* Number of the sprite that is to shown */

    GLuint textures[MAX_SPRITES]; /* Array of textures that are this actor's
                                     sprites */

    isobf_t groups;        /* Bit-field of the groups the
                                   actor is a member of */

    void (*i_handler)(struct isoactor *, \
            struct isoeng *); /* Iteration handler that gets called on
                                 each logic frame */

    struct map_handle_ls *map_handlers[MAX_MAPS];
                                /* Array of handle lists (one for each
                                   map), each list contains a list of 
                                   collision handlers detailing what should
                                   be done for collision with certain tiles
                                   on the map */ 

    int cw, ch;                 /* Width and height of collision sprites (in
                                   pixels) */

    int ctw, cth;              /* Width and height of collision tiles
                                  (in pixels) */

    int coll_point_x, coll_point_y; /* Point in collision sprite that
                                       has just been collided with */

    isobf_t *cfields[MAX_SPRITES];  /* Array of pointers to the
                                             arrays that contain the 
                                             bit-fields of the
                                             collision maps of the 
                                             various sprites */

    struct actor_handle_ls *actor_handlers[MAX_MAPS];
                                /* Array of lists of actor-actor collision
                                 * handlers detailing what should be done
                                 * for an actor-actor collision on a certain
                                 * map */
    isobf_t collision_groups[MAX_MAPS]; /* Bitwise OR of all groups actor
                                                is primed to detect collisons 
                                                with for the map */
};

/* Frees the resources allocated to the actor */
void isoactor_free(struct isoactor *actor);

/* Creates a new actor, of width 'w', height 'h', using the image file located
 * at 'sprite_filename' to source the sprite images.
 * A collision map is also created using tiles of dimensions 'ctw' x 'cth' that
 * are fitted to the images located at 'c_sprite_filename'. Each collision
 * sprite should be sized 'cw' x 'ch'.  The collision image
 * should be a black sillouette of the collision outline on a white
 * background. */ 
struct isoactor *isoactor_create(int w, int h, const char *sprite_filename,
                                 int ctw, int cth, int cw, int ch,
                                 const char *c_sprite_filename);

/* Sets a collision handler for the actor. The handler is such that it 
 * will be called everytime 'actor' steps on one of 'tiles' in 'map'.
 * The tiles are given as a string in the same way as the map gets
 * specified, and the same key as was used to specify 'map' is used
 * to look up the tiles */ 
void set_map_handler(struct isoactor *actor, struct isomap *map, 
        const char *tiles,
        void (*handler)(struct isoactor *,
                        struct isomap *));

/* Unsets the handler 'handle' that gets fired when 'actor' steps on
 * one of the 'tiles' in 'map' */
void uset_map_handler(struct isoactor *actor, struct isomap *map, 
        const char *tiles,
        void (*handler)(struct isoactor *,
                        struct isomap *));

/* Sets an actor-actor collision handler for the actor. The handler
 * is called every time the actor collides with an actor belonging to
 * one of the groups 'groups'. Handler is passed the two actors involved
 * in the collision */
void set_actor_handler(struct isoactor *actor, struct isomap *map,
        isobf_t groups,
        void (*handler)(struct isoactor *, struct isoactor *));

/* Unsets the actor handler */
void uset_actor_handler(struct isoactor *actor, struct isomap *map,
        isobf_t groups,
        void (*handler)(struct isoactor *, struct isoactor *));

/* Calculates overlap of collision rectangles of 'a1' and 'a2'. If
 * they don't overlap at all 0 is returned. If they do overlap then
 * the struct 'overlap' is filled with the appropriate information */
int isoactor_calc_overlap(struct isoactor *a1, struct isoactor *a2,
       struct isomap *map, struct isoactor_overlap *overlap);

/* Does a bitwise collision detection for actors 'a1' and 'a2'on
 * 'map', presuming that they overlap as described in 'overlap'.
 * Returns 1 if actors have collided */
int isoactor_bw_c_detect(struct isoactor *a1, struct isoactor *a2,
        struct isomap *map, struct isoactor_overlap *overlap);


/* Calculates the overlap of the actor 'a' and the map 'map' */
int isoactor_map_calc_overlap(struct isoactor *a, struct isomap *map,
        struct isoactor_overlap *overlap);

/* Does a bitwise collision detection for actor 'a' and tiles of
 * type 'index' on map 'map', presuming that they overlap as 
 * described in 'overlap'. Returns 1 if there has been a collision */
int isoactor_map_bw_c_detect(struct isoactor *a, struct isomap *map,
        unsigned int index, struct isoactor_overlap *overlap);

/* Paint the actor, on the map 'map'. 'frame' is the frame that the game is 
 * currently on */
int isoactor_paint(struct isoactor *actor, struct isomap *map, double frame);

/* Function that gets called on each logic frame */
void isoactor_iterate(struct isoactor *actor, struct isoeng *engine);


#endif
