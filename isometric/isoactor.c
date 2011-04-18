/* isoactor.c
 * 16-04-11
 * Functions for creating/animating/destroying a character in a game
 *
 */

#include "isoeng.h"
#include "isoactor.h"

void isoactor_free(struct isoactor *actor)
{
    int i;
#ifdef DEBUG_MODE
    fprintf(stderr, "Deleting actor uid: %d\n", actor->uid);
#endif

    for(i = 0; i < actor->number_of_sprites; i++)
        if(glIsTexture(actor->textures[i]))
            glDeleteTextures(1, &(actor->textures[i]));


    free(actor);

    return;
}

struct isoactor *isoactor_create(int w, int h, const char *sprite_filename)
{
    struct isoactor *actor;
    int max_size, xpad, ypad; 
    int images_wide, images_high, hindex, windex, index;
    Uint32 alpha;
    SDL_Surface *sprite, *image;
    SDL_Rect dst, src;


    if((actor = malloc(sizeof(*actor))) == NULL){
        fprintf(stderr, "Error allocating memory for actor.\n");
        exit(1);
    }

    actor->w = w;
    actor->h = h;

    actor->p2w = mkp2(w);
    actor->p2h = mkp2(h);

    actor->number_of_sprites = 0;

    /* Check sprite size doesn't exceed openGL's maximum texture size */
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);

    if(actor->p2w > max_size || actor->p2h > max_size){
        fprintf(stderr, "Image size (%d, %d) exceeds maximum texture size (%d)\n",
                actor->p2w, actor->p2h, max_size);
        return NULL;
    }

    /* Load the image file that will contain the sprites */
    image = IMG_Load(sprite_filename);
    if(!image){
        fprintf(stderr, "Error! Could not load %s\n", sprite_filename);
        return NULL;
    }

    /* Get the transparent colour from the newly loaded surface */
    alpha = SDL_MapRGBA(image->format, 0, 0, 0, 0);
    /* Make SDL copy the alpha channel of the image */
    SDL_SetAlpha(image, 0, SDL_ALPHA_OPAQUE);

    xpad = (actor->p2w - actor->w)/2;
    ypad = (actor->p2h - actor->h)/2;

    sprite = SDL_CreateRGBSurface(SDL_SWSURFACE, actor->p2w,
            actor->p2h, 32, RMASK, GMASK, BMASK, AMASK);
    if(!sprite){
        fprintf(stderr, "Error creating a surface for the sprites\n");
       // jactor_free(actor);
        return NULL;
    }


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
    images_wide = (int)(image->w/actor->w);
    images_high = (int)(image->h/actor->h);
    actor->number_of_sprites = images_wide * images_high;
    actor->show_sprite = 0;

    for(hindex = 0; hindex < images_high; hindex++){
        for(windex = 0; windex < images_wide; windex++){
            index = windex + hindex;

#ifdef DEBUG_MODE
            fprintf(stderr, "loading sprite index %d, ", index);
#endif
            SDL_FillRect(sprite, NULL, alpha);
            SDL_BlitSurface(image, &src, sprite, &dst);

            /* Create an openGL texture and bind the sprite's image to it */
            glGenTextures(1, &(actor->textures[index]));
            glBindTexture(GL_TEXTURE_2D, actor->textures[index]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite->w, sprite->h,
                    0, GL_RGBA, GL_UNSIGNED_BYTE, sprite->pixels);

            src.x += actor->w;
        }
        src.x = 0;
        src.y += actor->h;
    }

#ifdef DEBUG_MODE
            fprintf(stderr, "\n");
#endif


    SDL_FreeSurface(image);
    SDL_FreeSurface(sprite);

    return actor;
}

int isoactor_paint(struct isoactor *actor, struct isomap *map, double frame)
{
    double fframe = frame - floor(frame); /* fframe holds what fraction we
                                             through the current frame */
    int show_sprite = actor->show_sprite;

    /* Calculating the point where the actor should be drawn */
    actor->gx = fframe * actor->x + (1 - fframe) * actor->px;
    actor->gy = fframe * actor->y + (1 - fframe) * actor->py;

    if(actor->show_sprite > actor->number_of_sprites -1 ){
        show_sprite = actor->number_of_sprites - 1;
    }

    /* Transforming the point where the actor should be drawn 
     * to the map's perspective */
    map->coord_trans(map, &(actor->gx), &(actor->gy));



    /* Paint the actor's texture in the right place */

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);


    /* Load the actor's texture */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, actor->textures[actor->show_sprite]);

    /* translating the actor's matrix to the point where the 
     * the actor should be drawn */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(actor->gx , actor->gy , 0);


    /* Draw the actor */
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

    return 1;
}

