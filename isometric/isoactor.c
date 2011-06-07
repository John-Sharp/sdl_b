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

    for(i = 0; ; i++){
        if(actor->cfields[i] == NULL){
            break;
        }
        free(actor->cfields[i]);
    }

    for(i = 0; i < MAX_MAPS; i++){
        struct map_handle_ls *hp, *hq;
        struct actor_handle_ls *ahp, *ahq;

        for(hp = actor->map_handlers[i]; hp != NULL; ){

            hq = hp;
            hp = hp->next;
            free(hq);
        }

        for(ahp = actor->actor_handlers[i]; ahp != NULL; ){

            ahq = ahp;
            ahp = ahp->next;
            free(ahq);
        }
    }

    free(actor);

    return;
}

static void isoactor_load_cfields(struct isoactor *actor, int ctw,
        int cth, const char *c_sprite_filename)
{
    SDL_Surface *image;
    int images_wide, images_high;
    int hindex, windex, index;
    int ba_h = ceil(actor->ch / cth); /* Height of the bitmask array */
    int ba_w = ceil(actor->cw / ctw); /* Width of the bitmask array */
    int (*ba)[ba_w]; /* Bitmask array */
    Uint32 maskcolour;

    if(ba_w > sizeof(*(actor->cfields[0])) * CHAR_BIT){
        fprintf(stderr, "Error! Collision tilemap is too wide "
                "for bitmask variable. \n");
        return;
    }

    /* Load the image file that contains the collision sprites */
    image = IMG_Load(c_sprite_filename);
    if(!image){
        fprintf(stderr, "Error! Could not load collision sprite: %s\n",
                c_sprite_filename);
        return;
    }

    maskcolour = SDL_MapRGB(image->format, 0, 0, 0);

    /* Calculate how many sprite images are contained on the
     * surface we have been given */
    images_wide = (int)(image->w/actor->cw);
    images_high = (int)(image->h/actor->ch);

    ba = malloc(ba_h * ba_w * sizeof(*ba));
    if(!ba){
        fprintf(stderr, "Error! Could not malloc bitmask array\n");
        SDL_FreeSurface(image);
        return;
    }


    for(hindex = 0; hindex < images_high; hindex++){
        for(windex = 0; windex < images_wide; windex++){
            index = hindex * images_wide + windex;

            memset(ba, 0, sizeof(ba) * ba_h * ba_w);
#ifdef DEBUG_MODE
            fprintf(stderr, "loading collision sprite index %d\n\n", index);
#endif
            SDL_LockSurface(image);

            int i, j;
            for(i = 0; i < actor->ch; i++){
                for(j = 0; j < actor->cw; j++){
                    if(get_pixel(image, j + windex * actor->cw,
                                i + hindex * actor->ch) == maskcolour){
                        ba[i / cth][j / ctw] += 1;
                    }
                }
            }

            SDL_UnlockSurface(image);

            actor->cfields[index] = malloc(ba_h *
                    sizeof(*(actor->cfields[index])));
            if(!(actor->cfields[index])){
                fprintf(stderr,
                 "Error! Could not allocate memory for collision bit-field\n");
                return;
            }
            memset(actor->cfields[index], 0, ba_h *
                    sizeof(*(actor->cfields[index])));

            for(i = 0; i < ba_h; i++){
                for(j = 0; j < ba_w; j++){
                    if(ba[i][j] > ctw * cth / 2.){
                        /* This line produces the binary number
                         * 00010000000 where there are j 0's before
                         * the leading 1 */
                        actor->cfields[index][i] |= 1UL << 
                            (CHAR_BIT * sizeof(*(actor->cfields[index])) - j - 1);
#ifdef DEBUG_MODE
                        fprintf(stderr, "1");
#endif

                    }else{
#ifdef DEBUG_MODE
                        fprintf(stderr, "0");
#endif
                    }
                }
#ifdef DEBUG_MODE
                fprintf(stderr, "\n");
#endif

            }
        }
    }

#ifdef DEBUG_MODE
    fprintf(stderr, "\n");
#endif

    free(ba);

    return;
}

struct isoactor *isoactor_create(int w, int h, const char *sprite_filename,
                                 int ctw, int cth, int cw, int ch,
                                 const char *c_sprite_filename)
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
        fprintf(stderr, "Image size (%d, %d) exceeds "
                "maximum texture size (%d)\n",
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
            index = windex + hindex * images_wide;

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

    actor->cw = cw;
    actor->ch = ch;

    actor->cth = cth;
    actor->ctw = ctw;

    int i;
    for(i = 0; i < MAX_SPRITES; i++){
        actor->cfields[i] = NULL;
    }

    isoactor_load_cfields(actor, ctw, cth, c_sprite_filename);
    

    SDL_FreeSurface(image);
    SDL_FreeSurface(sprite);

    for(i = 0; i < MAX_MAPS; i++){
        actor->map_handlers[i] = NULL;

        actor->actor_handlers[i] = NULL;
        actor->collision_groups[i] = 0;
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
    hp->tiles = 0;

    int i = 0;
    for(i = 0; i < strlen(tiles); i++){
        int index = 0;
        index = ((strchr(map->c_key, tiles[i]) - map->c_key)
                / sizeof(unsigned char));
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
        index = ((strchr(map->c_key, tiles[i]) - map->c_key)
                / sizeof(unsigned char));
        tilemask |= 1 << index;
    }


    actor->map_handlers[map->uid] = 
        map_handler_del(actor->map_handlers[map->uid], map, tilemask,
            handler);

    return;
}


static struct actor_handle_ls *actor_handler_add(struct actor_handle_ls *ls,
        unsigned int groups, void (*actor_handler)(struct isoactor *,
            struct isoactor *))
{
    struct actor_handle_ls *hp;

    hp = malloc(sizeof(*hp));

    if(hp == NULL){
        fprintf(stderr, "Unable to allocate memory for a "
                "list handler node.\n");
        exit(1);
    }

    hp->groups = groups;
    hp->actor_handler = actor_handler;

    hp->next = ls;

    return hp;
}

static struct actor_handle_ls *actor_handler_del(struct actor_handle_ls *ls,
        unsigned int groups, void (*actor_handler)(struct isoactor *,
            struct isoactor *))
{
    if(ls == NULL){
        return NULL;
    }

    if(groups & ls->groups && ls->actor_handler == actor_handler){

        ls->groups &= ~groups;

        if(groups != 0)
            return ls;

        struct actor_handle_ls *p = ls->next;
        free(ls);
        return p;
    }

    ls->next = actor_handler_del(ls->next, groups, actor_handler);
    return ls;
}

void set_actor_handler(struct isoactor *actor, struct isomap *map,
        unsigned int groups,
        void (*handler)(struct isoactor *, struct isoactor *))
{
    actor->actor_handlers[map->uid] = 
        actor_handler_add(actor->actor_handlers[map->uid], groups,
                handler);

    actor->collision_groups[map->uid] |= groups;

    return;
}

void uset_actor_handler(struct isoactor *actor, struct isomap *map,
        unsigned int groups,
        void (*handler)(struct isoactor *, struct isoactor *))
{
    struct actor_handle_ls *hp;

    actor->actor_handlers[map->uid] =
        actor_handler_del(actor->actor_handlers[map->uid], groups,
                handler);

    actor->groups = 0;
    for(hp = actor->actor_handlers[map->uid];
            hp != NULL; hp = hp->next){
        actor->groups |= hp->groups;
    }

    return;
}

static int isoactor_calc_overlap_l(double a1, double a2, double b1, double b2,
        struct isoactor_overlap_l *overlap)
{
    if(a1 < b1){
        if(a2 < b1){
            return 0;
        }
        overlap->a1_offset = b1 - a1;
        overlap->a2_offset = 0;

        if(a2 < b2){
            overlap->overlap = a2 - b1;
        }else{
            overlap->overlap = b2 - b1;
        }

        //return 1;
    }else{
        if(b2 < a1){
            return 0;
        }

        overlap->a1_offset = 0;
        overlap->a2_offset = a1 - b1;

        if(b2 < a2){
            overlap->overlap = b2 - a1;
        }else{
            overlap->overlap = a2 - a1;
        }

        //return 1;
    }



    return 1;
}

int isoactor_calc_overlap(struct isoactor *a1, struct isoactor *a2,
        struct isomap *map, struct isoactor_overlap *overlap)
{
    double a1x = project_a_x(map->ri, a1->x, a1->y);
    double a1y = project_a_y(map->ri, a1->x, a1->y);
    double a2x = project_a_x(map->ri, a2->x, a2->y);
    double a2y = project_a_y(map->ri, a2->x, a2->y);

    if(!isoactor_calc_overlap_l(a1x - (double)(a1->cw/2),
                a1x + (double)(a1->cw)/2,
                a2x - (double)(a2->cw)/2,
                a2x + (double)(a2->cw)/2, &(overlap->x))){
        return 0;
    }

    if(!isoactor_calc_overlap_l(a1y - (double)(a1->ch)/2,
                a1y + (double)(a1->ch)/2, a2y - (double)(a2->ch)/2,
                a2y + (double)(a2->ch)/2, &(overlap->y))){
        return 0;
    }

    return 1;
}

int isoactor_bw_c_detect(struct isoactor *a1, struct isoactor *a2,
        struct isomap *map, struct isoactor_overlap *overlap)
{
    int i;
    int ctw = a1->ctw;
    int cth = a1->cth;

    for(i = 0; i < floor(overlap->y.overlap / cth); i++){
        unsigned long bf1 = a1->cfields[map->uid]
            [i + (int)floor((overlap->y.a1_offset/cth))];
        unsigned long bf2 = a2->cfields[map->uid]
            [i + (int)floor((overlap->y.a2_offset/cth))];

        bf1 = (bf1 << ((int)(overlap->x.a1_offset/ctw))) /* Shift the bit-field
                                                          so the left-most 
                                                          figure corresponds 
                                                          to the start of the
                                                          overlap */

            & ((~0) << (CHAR_BIT * sizeof(unsigned long)
                        - (int)(overlap->x.overlap/ctw))); /* Trim the bitfield
                                                              so that it as
                                                              wide as the 
                                                              overlap */

        bf2 = (bf2 << ((int)(overlap->x.a2_offset/ctw)))
            & ((~0) << (CHAR_BIT * sizeof(unsigned long)
                        - (int)(overlap->x.overlap/ctw)));

        if(bf1 & bf2){
            return 1;
        }
    }

    return 0;
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

