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

    for(i = 0; i < MAX_MAPS; i++){
        struct map_handle_ls *hp, *hq;

        for(hp = actor->map_handlers[i]; hp != NULL; ){

            hq = hp;
            hp = hp->next;
            free(hq);
        }
    }

    free(actor);

    return;
}

struct isoactor *isoactor_create(int w, int h, const char *sprite_filename)
{
    struct isoactor *actor;
    Uint32 colourkey;
    int max_size, xpad, ypad; 
    int images_wide, images_high, hindex, windex, index;
    SDL_Surface *sprite, *image;
    SDL_Rect dst, src;


    if((actor = malloc(sizeof(*actor))) == NULL){
        fprintf(stderr, "Error allocating memory for actor.\n");
        exit(1);
    }

    actor->x = actor->y = actor->px = actor->py = 0;
    actor->vx = actor->vy = actor->ax = actor->ay = 0;

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

    /* Make SDL copy the alpha channel of the image */
    SDL_SetAlpha(image, 0, SDL_ALPHA_OPAQUE);
    colourkey = SDL_MapRGBA(image->format, 0xff, 0x00, 0xff, 0);

    xpad = (actor->p2w - actor->w)/2;
    ypad = (actor->p2h - actor->h)/2;

    sprite = SDL_CreateRGBSurface(SDL_SWSURFACE, actor->p2w,
            actor->p2h, 32, RMASK, GMASK, BMASK, AMASK);
    if(!sprite){
        fprintf(stderr, "Error creating a surface for the sprites\n");
        isoactor_free(actor);
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
            SDL_FillRect(sprite, NULL, colourkey);
            SDL_SetColorKey(image, SDL_SRCCOLORKEY, colourkey);
            SDL_BlitSurface(image, &src, sprite, &dst);

            /* Create an openGL texture and bind the sprite's image to it */
            glGenTextures(1, &(actor->textures[index]));
            glBindTexture(GL_TEXTURE_2D, actor->textures[index]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, actor->p2w, actor->p2h,
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

    int i;
    for(i = 0; i < MAX_MAPS; i++){
        actor->map_handlers[i] = NULL;
    }


    actor->i_handler = NULL;

    return actor;
}

static struct map_handle_ls *map_handler_add(struct map_handle_ls *ls,
        struct isomap *map, const char *tiles,
        void (*map_handler)(struct isoactor *, struct isomap *,
                        unsigned int tile, enum isoside))
{
    struct map_handle_ls *hp;

    hp = malloc(sizeof(*hp));

    if(hp == NULL){
        fprintf(stderr, "Unable to allocate memory for a "
                "list handler node.\n");
        exit(1);
    }
    hp->map = map;

    int i = 0;
    for(i = 0; i < strlen(tiles); i++){
        int index = 0;
        index = ((strchr(map->c_key, tiles[i]) - map->c_key) / sizeof(unsigned char));
        hp->tiles |= 1 << index;
    }

    hp->map_handler = map_handler;
    hp->next = ls;

    return hp;
}

static struct map_handle_ls *map_handler_del(struct map_handle_ls *ls,
                                         struct isomap *map,
                                         unsigned int tiles,
                        void (*map_handler)(struct isoactor *,
                            struct isomap *, unsigned int, enum isoside))
{
    if(ls == NULL){
        return NULL;
    }

    if(ls->map->uid == map->uid && ls->map_handler == map_handler){

        /* Make the handler insensitive to the correct tiles */
        ls->tiles &= ~tiles;

        if(tiles != 0) /* This handler still applies to some tiles */
            return ls;

        /* This handler applies to no more tiles and can be deleted */
        struct map_handle_ls *p = ls->next;
        free(ls);
        return p;
    }

    ls->next = map_handler_del(ls->next, map, tiles, map_handler);
    return ls;
}


void set_map_handler(struct isoactor *actor, struct isomap *map, 
        const char *tiles,
        void (*handler)(struct isoactor *,
                        struct isomap *,
                        unsigned int, enum isoside))
{

    actor->map_handlers[map->uid] = 
        map_handler_add(actor->map_handlers[map->uid], map, tiles,
            handler);

    return;
}

void uset_map_handler(struct isoactor *actor, struct isomap *map, 
        const char *tiles,
        void (*handler)(struct isoactor *,
                        struct isomap *,
                        unsigned int, enum isoside))
{
    unsigned int tilemask = 0;

    int i = 0;
    for(i = 0; i < strlen(tiles); i++){
        int index = 0;
        index = ((strchr(map->c_key, tiles[i]) - map->c_key) / sizeof(unsigned char));
        tilemask |= 1 << index;
    }


    actor->map_handlers[map->uid] = 
        map_handler_del(actor->map_handlers[map->uid], map, tilemask,
            handler);

    return;
}

int isoactor_paint(struct isoactor *actor, struct isomap *map, double frame)
{
    double fframe = frame - floor(frame);  /* fframe holds what fraction we
                                              through the current frame */
    int show_sprite = actor->show_sprite;
    double x_temp;

    /* Calculating the point where the actor should be drawn */
    actor->gx = fframe * actor->x + (1 - fframe) * actor->px;
    actor->gy = fframe * actor->y + (1 - fframe) * actor->py;

    if(actor->show_sprite > actor->number_of_sprites -1 ){
        show_sprite = actor->number_of_sprites - 1;
    }

    /* Transforming the point where the actor should be drawn 
     * to the map's perspective */
    x_temp = project_a_x(map->ri, actor->gx, actor->gy);
    actor->gy = project_a_y(map->ri, actor->gx, actor->gy);
    actor->gx = x_temp;


    /* Paint the actor's texture in the right place */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 2*actor->p2w, 0, 2*actor->p2h, -1, 1);



    /* Load the actor's texture */
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* translating the actor's matrix to the point where the 
     * the actor should be drawn */
    glTranslatef(actor->gx, actor->gy, 0);
    glBindTexture(GL_TEXTURE_2D, actor->textures[actor->show_sprite]);

    /* Draw the actor */

    GLint xpad, ypad;
    xpad = (actor->p2w - actor->w) / 2.;
    ypad = (actor->p2h - actor->h) / 2.;


    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);

    glTexCoord2d(xpad, ypad);
    glVertex2i(-actor->w/2, actor->h/2);

    glTexCoord2d(xpad, ypad + actor->h);
    glVertex2i(-actor->w/2, -actor->h/2);

    glTexCoord2d(xpad + actor->w, ypad + actor->h);
    glVertex2i(actor->w/2, -actor->h/2);

    glTexCoord2d(xpad + actor->w, ypad);
    glVertex2i(actor->w/2, actor->h/2);


    glEnd();

    glDisable(GL_BLEND);

    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    return 1;
}

void isoactor_iterate(struct isoactor *actor, struct isoeng *engine)
{
    if(actor->i_handler != NULL){
        actor->i_handler(actor, engine);
    }

    actor->px = actor->x;
    actor->py = actor->y;

    actor->x += actor->vx;
    actor->y += actor->vy;

    actor->vx += actor->ax;
    actor->vy += actor->ay;

    return;
}

