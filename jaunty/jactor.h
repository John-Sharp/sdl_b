//functions for creating and manipulating an actor
#ifndef JACTOR_H
#define JACTOR_H

#define JACTOR_MAX_SPRITES 15

typedef struct jactor jactor;
typedef struct jen jen;

/* flag for storing whether collision was actor-actor or actor-tile */
typedef enum{
    A_A = 0,
    A_T = 1
} jctype;



typedef enum {
    NONE = 0,
    TOP = 0x1,
    TOPRIGHT = 0x11,
    RIGHT = 0x10,
    BOTTOMRIGHT = 0x110,
    BOTTOM = 0x100,
    BOTTOMLEFT = 0x1100,
    LEFT = 0x1000,
    TOPLEFT = 0x1001,
    MIDDLE = 0x10000
} jsides;


typedef struct jcoll
{
    double x, y; /* x, y coordinates of the collision */
    jctype c_type; /* flag to show whether collision was actor-actor or 
                      actor-tile */
    unsigned char c_index; /* Index of the tile that was collided with */
    unsigned char c_key_num; /* Number of the tile collided with on the 
                                collision map */
    jsides side; /* Side of the tile that was collided with */
    jactor *actor; /* the actor that was collided with */

} jcoll;
 
/* TODO add iteration function handler that gets called prior to each
 * iteration for each actor, as arguments for just take the actor */

struct jactor {
    /* width and height of actor in pixels */
    int w;
    int h;

    /* map that the actor resides in */
    jmap *map;

    /* Unique identifier of the actor */
    unsigned int uid;

    /* Number of the group that the actor is a member of */
    unsigned char group_num;

    /* State of the actor */
    unsigned int actor_state;

    /* width and height of the actual texture, which is required to be 
     * a power of 2 by openGL */
    int p2w, p2h;

    //the mass of the actor
    double m;

    //previous logic frame's position
    double px, py, pr, ptheta;

    //interpolated position
    double gx, gy, gr, gtheta;

    //position, velocity and acceleration of 
    //actor
    double x, y, v_x, v_y, a_x, a_y;

    /* Radius about which the actor rotates,
     * angle of rotation of the actor, from the
     * positive x axis */
    double r, theta;

    /* Radial velocity of the actor,
     * angular velocity of the actor */
    double v_r, omega;

    //collision handling function
    int (*c_handler)(jactor *, jcoll *, jen *);

    /* function that gets called upon each iteration */
    void (*i_handler)(jactor *, jen *);

    /* function that gets called when the actor is to be painted */
    void (*paint)(jactor *, double);


    /* Name of textures associated with this actor */
    GLuint tex_name[JACTOR_MAX_SPRITES];

    /* Total number of sprites */
    unsigned int number_of_sprites;

    /* Index number of the sprite to be shown */
    unsigned int show_sprite;

};

//frees all resources allocated to 'actor'
void jactor_free(jactor *actor);

/* Creates an actor at ('w', 'h'), residing in map 'map', and with the 
 * sprite image stored in the file names 'sprite_filename' */
jactor *jactor_create(int w, int h, jmap *map, const char *sprite_filename);

/* Creates an actor, but rather than loading an image file for the sprite 
 * image, uses an SDL_Surface for the image */
jactor *jactor_create_from_surface(int w, int h, jmap *map, SDL_Surface * surf);


//computes the new position of 'actor', after a logical frame has passed 
void jactor_iterate(jactor *actor, jen *engine);

//blits the 'actor' onto the 'screen'
void jactor_paint(jactor *actor, double frames_so_far);

/* returns 1 if actor1 is colliding with actor2 and fills the jcoll struct 
 * pointed to with information about the colission that has taken place */
int jactor_collision_detect(jactor *actor1, jactor *actor2, jcoll *c_info);


#endif  
