//functions for creating and manipulating an actor
#include <stdlib.h>
#include <math.h>
#include "SDL.h"
#include <SDL_image.h>
#include "jen.h"

void jactor_free(jactor *actor)
{
    if(glIsTexture(actor->tex_name))
        glDeleteTextures(1, &(actor->tex_name));
    free(actor);
}

jactor *jactor_create(int w, int h, jmap *map, const char *sprite_filename)
{
    SDL_Surface *temp;
    struct jactor *actor;

    /* load image */
    temp = IMG_Load(sprite_filename);

    if(!temp){
        fprintf(stderr, "Error! Could not load %s\n", sprite_filename);
        return NULL;
    } 
    
    actor = jactor_create_from_surface(w, h, map, temp);

    SDL_FreeSurface(temp);

    return actor;
}


jactor *jactor_create_from_surface(int w, int h, jmap *map, SDL_Surface *surf)
{
    SDL_Surface *sprite;
    SDL_Rect dst;
    jactor *actor;
    int max_size, xpad, ypad;
    static unsigned int uid = 0;
    Uint32 alpha;
    SDL_Rect src;
    int images_wide, images_high, hindex, windex, index;


    if((actor = malloc(sizeof(*actor))) == NULL){
        fprintf(stderr, "Could not allocate memory for sprite\n");
        return NULL; 
    }

    /* Set the actor's unique identifier */
    actor->uid = uid;
    uid++;

    /* Zero the group_num */
    actor->group_num = 0;

    /* Note the map that the actor resides in */
    actor->map = map;

    /* set initial values of actor's attributes */
    actor->w = w;
    actor->h = h;

    actor->v_x = 0;
    actor->v_y = 0;
    actor->x = 0;
    actor->y =0;
    actor->a_x = 0;
    actor->a_y = 0;

    actor->r = 0;
    actor->pr = 0;
    actor->v_r = 0;

    actor->theta = 0;
    actor->ptheta = 0;
    actor->omega = 0;

    actor->c_handler = NULL;
    actor->i_handler = NULL;

    actor->p2w = mkp2(w);
    actor->p2h = mkp2(h);

    actor->paint = jactor_paint;

    /* check it doesn't exceed the maximum texture size */
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);

    if ((actor->p2w > max_size) || (actor->p2h > max_size)) {
        fprintf(stderr, "Image size exceeds max texture size\n");
        jactor_free(actor);
        return NULL;
    }


    /* We need to copy image onto a surface whose width
     * and height are both a power of two
     * (as demanded by openGL) */
  
    xpad = (actor->p2w - actor->w)/2;
    ypad = (actor->p2h - actor->h)/2;

    sprite = SDL_CreateRGBSurface(SDL_SWSURFACE, actor->p2w,
            actor->p2h, 32, RMASK, GMASK, BMASK, AMASK);

 
    if(!sprite){
        fprintf(stderr, "Error creating a surface for the sprite\n");
        jactor_free(actor);
        return NULL;
    }

    alpha = SDL_MapRGBA(surf->format, 0, 0, 0, 0); 
    SDL_SetAlpha(surf, 0, SDL_ALPHA_OPAQUE);

    dst.x = xpad;
    dst.y = ypad;
    dst.w = actor->w;
    dst.h = actor->h;
    src.w = actor->w;
    src.h = actor->h;
    src.x = 0;
    src.y = 0;


    /* Calculate how many sprite images are contained on the
     * surface we have been given */
    images_wide = (int)(surf->w/actor->w);
    images_high = (int)(surf->h/actor->h);
    actor->number_of_sprites = images_wide * images_high;
    actor->show_sprite = 0;
    
    for(hindex = 0; hindex < images_high; hindex++){
        for(windex = 0; windex < images_wide; windex++){
            index = windex + hindex;
            fprintf(stderr, "index is %d\n", index);

            SDL_FillRect(sprite, NULL, alpha); 
            SDL_BlitSurface(surf, &src, sprite, &dst); 

            /* create an openGL texture and bind the sprite's image
             * to it */
            glGenTextures(1, &(actor->tex_name[index]));
            glBindTexture(GL_TEXTURE_2D, actor->tex_name[index]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite->w, sprite->h,
                    0, GL_RGBA, GL_UNSIGNED_BYTE, sprite->pixels);
            src.x += actor->w;
        }
        src.y += actor->h;
    }

    SDL_FreeSurface(sprite);

    return actor;
}

void jactor_iterate(jactor *actor, jen *engine)
{

    /* if it is set call the iteration handler */
    if(actor->i_handler != NULL){
        actor->i_handler(actor, engine);
    }

    actor->px = actor->x;
    actor->py = actor->y;

    actor->pr = actor->r;
    actor->ptheta = actor->theta;

    actor->x += actor->v_x; 
    actor->y += actor->v_y;

    actor->r += actor->v_r;
    actor->theta += actor->omega;

    actor->v_x += actor->a_x;
    actor->v_y += actor->a_y;
    return;
}

void jactor_paint(jactor *actor, double frames_so_far)
{
    double fframe = frames_so_far - floor(frames_so_far);
    int show_sprite = actor->show_sprite;

    if(actor->show_sprite > actor->number_of_sprites - 1){
        show_sprite = actor->number_of_sprites - 1;
    }

    /* calculating the point where the actor should be drawn */
    actor->gx = actor->px * (1-fframe) + fframe * actor->x;
    actor->gy = actor->py * (1-fframe) + fframe * actor->y;

    /* Moving to correct for fact that actor's px, py, x, y are
     * all relative to the actor's map, not to the main screen */
  //  actor->gx += actor->map->map_rect.x;
   // actor->gy += actor->map->map_rect.y;

    actor->gr = actor->pr * (1-fframe) + fframe * actor->r;
    actor->gtheta = actor->ptheta * (1-fframe) + fframe * actor->theta; 


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, actor->map->map_rect.w,
            actor->map->map_rect.h, 0, 0.5, 0.0);
    glViewport(actor->map->map_rect.x, actor->map->map_rect.y,
            actor->map->map_rect.w, actor->map->map_rect.h);



	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

    /* translating the actor's matrix to the point where the 
     * the actor should be drawn */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(actor->gx , actor->gy , 0);

    /* Rotating the actor as described by theta and r */
    glRotatef(-actor->gtheta, 0.0, 0.0, 1.0);
    glTranslatef(0.0, actor->gr, 0);

    /* loading the actor's texture */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, actor->tex_name[actor->show_sprite]);

    /* drawing the actor */
    
    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-(float)actor->p2w/2.0, -(float)actor->p2h/2.0, 0.0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-(float)actor->p2w/2.0, (float)actor->p2h/2.0, 0.0);

    glTexCoord2f(1.0, 1.0);
    glVertex3f((float)actor->p2w/2.0, (float)actor->p2h/2.0, 0.0);

    glTexCoord2f(1.0, 0.0);
    glVertex3f((float)actor->p2w/2.0, -(float)actor->p2h/2.0, 0.0);

    glEnd();
    

    glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
    return;
}


/* TODO: make the detection work by searching the entire path traced out
 * by the actors from the previous frame to the next frame */
int jactor_collision_detect(jactor *actor1, jactor *actor2, jcoll *c_info)
{
    double separation;
    /* the angle between the line joining the centres of the two 
     * colliding actors and the x-axis */
    double theta;

    separation = magnitude(actor2->y - actor1->y, actor2->x - actor1->x);

    if(separation - (actor2->w / 2.) < actor1->w / 2. ){
        theta = x_pos_angle((actor2->x - actor1->x) ,(actor2->y - actor1->y));
        c_info->x = actor1->x + actor1->w/2. * cos(theta);
        c_info->y = actor1->y + actor1->w/2. * sin(theta);
        c_info->actor = actor2;
        c_info->c_type = A_A;

        return 1;
    }

    return 0;
}
