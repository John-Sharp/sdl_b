//functions for creating and manipulating an actor
#include <stdlib.h>
#include <math.h>
#include "SDL.h"
#include <SDL_image.h>
#include "jactor.h"
#include "jen.h"
#include "jutils.h"

void jactor_free(jactor *actor)
{
    glDeleteTextures(1, &(actor->tex_name));
    free(actor);
}

jactor *jactor_create(int w, int h, const char *sprite_filename, 
                      void (*c_handler)(jactor *, jcoll *))
{
    SDL_Surface *temp, *sprite;
    SDL_Rect dst;
    jactor *actor;
    int max_size, xpad, ypad;
    Uint32 alpha;


    if((actor = malloc(sizeof(*actor))) == NULL){
        fprintf(stderr, "Could not allocate memory for sprite\n");
        return NULL; 
    }


    /* set initial values of actor's attributes */
    actor->w = w;
    actor->h = h;

    actor->v_x = 0;
    actor->v_y = 0;
    actor->x = 0;
    actor->y =0;
    actor->a_x = 0;
    actor->a_y = 0;

    actor->c_handler = c_handler;

    /* load image */
    temp = IMG_Load(sprite_filename);

    if(!temp){
        fprintf(stderr, "Error! Could not load %s\n", sprite_filename);
        jactor_free(actor);
        return NULL;
    } 


    /* check it doesn't exceed the maximum texture size */
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);

    if ((temp->w > max_size) || (temp->h > max_size)) {
        fprintf(stderr, "Image size exceeds max texture size\n");
        SDL_FreeSurface(temp);
        jactor_free(actor);
        return NULL;
    }

    /* copy image onto a surface whose width and height are 
     * both a power of two (as demanded by openGL) */
    actor->p2w = mkp2(w);
    actor->p2h = mkp2(h);

    xpad = (actor->p2w - actor->w)/2;
    ypad = (actor->p2h - actor->h)/2;

    sprite = SDL_CreateRGBSurface(SDL_SWSURFACE, actor->p2w,
            actor->p2h, 32, RMASK, GMASK, BMASK, AMASK);

    alpha = SDL_MapRGBA(temp->format, 0, 0, 0, 0); 
    SDL_FillRect(sprite, NULL, alpha); 
    
    if(!sprite){
        fprintf(stderr, "Error creating a surface for the sprite\n");
        SDL_FreeSurface(temp);
        jactor_free(actor);
        return NULL;
    }

    dst.x = xpad;
    dst.y = ypad;
    dst.w = actor->w;
    dst.h = actor->h;

    SDL_SetAlpha(temp, 0, SDL_ALPHA_OPAQUE);
    SDL_BlitSurface(temp, 0, sprite, &dst); 
    SDL_FreeSurface(temp);

    /* create an openGL texture and bind the sprite's image
     * to it */
    glGenTextures(1, &(actor->tex_name));
    glBindTexture(GL_TEXTURE_2D, actor->tex_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite->w, sprite->h,
            0, GL_RGBA, GL_UNSIGNED_BYTE, sprite->pixels);

    SDL_FreeSurface(sprite);

    return actor;
}

void jactor_iterate(jactor *actor)
{
    actor->px = actor->x;
    actor->py = actor->y;

    actor->v_x += actor->a_x;
    actor->v_y += actor->a_y;

    actor->x += actor->v_x; 
    actor->y += actor->v_y;

    return;
}

void jactor_paint(jactor *actor, double frames_so_far)
{
    double fframe = frames_so_far - floor(frames_so_far);


    /* calculating the point where the actor should be drawn */
    actor->gx  = actor->px * (1-fframe) + fframe * actor->x;
    actor->gy  = actor->py * (1-fframe) + fframe * actor->y;

	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

    /* translating the actor's matrix to the point where the 
     * the actor should be drawn */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(actor->gx , actor->gy , 0);

    /* loading the actor's texture */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, actor->tex_name);

    /* drawing the actor */
    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-(float)actor->p2w/2.0, -(float)actor->p2h/2.0, 0.0);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-(float)actor->p2w/2.0, (float)actor->p2h/2.0, 0.0);

    glTexCoord2f(1.0, 0.0);
    glVertex3f((float)actor->p2w/2.0, (float)actor->p2h/2.0, 0.0);

    glTexCoord2f(1.0, 1.0);
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

    separation = magnitude(actor1->y - actor2->y, actor1->x - actor2->x);

    if(separation < actor1->w || separation < actor2->w){
        theta = atan((actor2->y - actor1->y)/(actor2->x - actor1->y));
        c_info->x = actor1->x + actor1->w * cos(theta);
        c_info->y = actor1->y + actor1->w * sin(theta);
        c_info->actor = actor2;
        c_info->c_type = A_A;
        return 1;
    }

    return 0;
}
