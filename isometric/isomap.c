/* isomap.c
 * 27-02-11
 * Functions for creating/loading/drawing/destroying an isometric map.
 *
 */

#include "isomap.h"
#include "isoeng.h"
void isomap_free(struct isomap *map)
{
    SDL_FreeSurface(map->tilepalette);
    glDeleteTextures(1, &(map->texname));
    free(map->map);
    free(map);
    return ;
}


static int isomap_set_cmap(struct isomap *map, const char *ck, const char *cm);

struct isomap *isomap_create(struct isoeng *engine,
        const SDL_Rect *map_rect,
        isobf_t groups, int w, int h, int tw,
        const char *filename, const char *k, const char *m,
        const char *ck, const char *cm)
{
    static unsigned int uid = 0;
    struct isomap *map;

    if(uid + 1 > MAX_MAPS){
        fprintf(stderr, "Game contains more maps than is allowed by the "
                "engine\n");
        return NULL;
    }

    map = malloc(sizeof(*map));
    if(!map)
        return NULL;

    map->engine = engine;

    map->uid = uid;
    uid++;

    map->groups = groups;

    map->w = w;
    map->h = h;

    map->tw = tw;
    map->th = ceil((float)tw / sqrt(3));

    map->ri[0][0] = 1 / sqrt(2);
    map->ri[0][1] = -1 / sqrt(2);
    map->ri[0][2] = map->h * map->tw/2;
    map->ri[1][0] = 1/ sqrt(6);
    map->ri[1][1] = 1/ sqrt(6);
    map->ri[1][2] = 0;

    map->ir[0][0] = 1 / sqrt(2);
    map->ir[0][1] = sqrt(6) / 2;
    map->ir[0][2] = -map->h * map->tw/2 / sqrt(2);
    map->ir[1][0] = - 1 / sqrt(2);
    map->ir[1][1] = sqrt(6) / 2;
    map->ir[1][2] = map->h * map->tw/2 / sqrt(2);


    memcpy(&(map->map_rect), map_rect, sizeof(map->map_rect));
    //map->itw = project_y(map->ir, map->tw/2, map->th/2);
    map->itw = map->tw / sqrt(2);

    map->rw = (project_x(map->ri, map->w, 0) - project_x(map->ri, 0, map->h))
         / sqrt(2) * (float)map->tw;
    map->rh = ((float)project_y(map->ri, map->w, map->h)
        * sqrt(3) / sqrt(2) * (float)map->th);

    map->map = malloc(sizeof(*(map->map)) * w * h);


    if(!map->map){
        isomap_free(map);
        return NULL;
    }

    map->tilepalette = IMG_Load(filename);
    if(!map->tilepalette){
        isomap_free(map);
        return NULL;
    }

    if(!isomap_from_string(map, k, m)){
        isomap_free(map);
        return NULL;
    }

    if(!isomap_set_cmap(map, ck, cm)){
        isomap_free(map);
        return NULL;
    }

    map->i_handler = NULL;

    map->ob_handler = isomap_ob_handler;

    return map;
}

static void isomap_load_cfields(struct isomap *map, int ctw, int cth,
        unsigned int index)
{
    int ba_h = ceil(map->rh / cth); /* Height of the bitmask array */
    int ba_w = ceil(map->rw / ctw); /* Width of the bitmask array */
    int i, j, x, y;

    float rx, ry; /*  Point in real-space */

    if(ba_w > MAP_BF_W * ISO_BFBW){
        fprintf(stderr, "Error! Collision map tiles are too wide "
                "for bitmask array. Please increase ctw.\n");
        exit(-1);
    }

    map->cfields[index] = malloc(MAP_BF_W * (ba_h
                * sizeof(*(map->cfields[index]))));
    if(!(map->cfields[index])){
        fprintf(stderr,
                "Error! Could not allocate memory for "
                "map collision bit-field\n");
        return;
    }

    memset(map->cfields[index], 0, MAP_BF_W * ba_h * sizeof(*(map->cfields[index])));

#ifdef DEBUG_MODE
                        fprintf(stderr, "Map collision bitmask "
                                "for index: %d:\n", index);
#endif
    for(i = 0; i < ba_h; i++){
        for(j = 0; j < ba_w; j++){
            /* Point on the screen we are looking at */
            x = (j + 0.5) * ctw;
            y = (i - 0.5) * cth;

            /* Project this point back onto map */
            rx = project_a_x(map->ir, (float)x, (float)y);
            ry = project_a_y(map->ir, (float)x, (float)y);

            /* Check if the point we're examining lies outside
             * the map */
            if(rx < 0 || rx > map->w * map->itw || ry < 0 ||
                    ry > map->h * map->itw){
                /* The index 0 is reserved for bitwise collisions
                 * where the actor has strayed over the edge of 
                 * the map */
                if(index == 0){
                    /* This line produces the binary number
                     * 00010000000 where there are j%l_int_bl 0's before
                     * the leading 1 */
                    map->cfields[index][MAP_BF_W * i + (int)floor(j/ISO_BFBW)]
                        |= (isobf_t)1 <<
                        ( ISO_BFBW - j%ISO_BFBW - 1);
                }
            }else{
                unsigned char (*map_ptr)[map->w] = (unsigned char(*)[map->w])map->c_map;
                /* If the tile underlying the sample point is 
                 * of the same type as the index then mark this
                 * part of the bitmask */
                if(map_ptr[(int)(ry/ map->itw)][(int)(rx/map->itw)] == index){

                    map->cfields[index][MAP_BF_W * i + (int)floor(j/ISO_BFBW)]
                        |= (isobf_t)1 << (ISO_BFBW - j%ISO_BFBW - 1);

                }
            }
        }
    }

#ifdef DEBUG_MODE
    fprintf(stderr, "Bit map for cmap index: %d\n", index);
    for(i = ba_h - 1; i >= 0; i--){
        int j;
        for(j = 0; j < MAP_BF_W; j++){
            printbitssimple(map->cfields[index][MAP_BF_W * i + j]);
        }
        fprintf(stderr, "\n");
    }
#endif

    return;
}

static int isomap_set_cmap(struct isomap *map, const char *ck, const char *cm)
{
    map->c_map = malloc(sizeof(*(map->c_map)) * strlen(cm));
    map->c_key = malloc(sizeof(*(map->c_key)) * (strlen(ck) + 1));

    if(map->c_map == NULL || map->c_key == NULL){
        return 0;
    }

    strcpy(map->c_key, ck);

    int i, j;
    int z = 0;
    int index;
    unsigned char (*map_ptr)[map->w] = (unsigned char(*)[map->w])map->c_map;

    for(j = map->h - 1; j >= 0; j--) 
        for(i = 0; i < map->w; i++){
            index = (strchr(ck, cm[z]) - ck) / (sizeof(unsigned char)) + 1;
            map_ptr[j][i] = index;
            z++;
        }

    index = strlen(ck);
    for(i = 0; i < index; i++){
        isomap_load_cfields(map, map->engine->ctw, map->engine->cth, i);
    }
    

    return 1;
}



int isomap_from_string(struct isomap *map, const char *k, const char *m)
{
    SDL_Surface *map_image;
    SDL_Rect dst, src;
    int j, i, z = 0;
    unsigned char (*map_ptr)[map->w] = (unsigned char(*)[map->w])map->map;
    unsigned char index;

    for(j = map->h - 1; j >= 0; j--) 
        for(i = 0; i < map->w; i++){
            index = (strchr(k, m[z]) - k) / (sizeof(unsigned char));
            map_ptr[j][i] = index;
            z++;
        }

    map_image = SDL_CreateRGBSurface(SDL_SWSURFACE, map->rw, map->rh, 32,
            RMASK, GMASK, BMASK, AMASK);

    if(!map_image){
        return 0;
    }

    src.w = map->tw;
    src.h = map->th;

    for(j = 0; j < map->h; j++)
        for(i = 0; i < map->w; i++){
            /* An SDL_Surface has the y-axis pointing down.
             * The vector going from the center of one tile to the 
             * center of the next tile in the map surface's x-direction
             * is given by mx = (map->tw/2, -map->th/2).
             * The vector going from the center of one tile to the
             * center of the next tile in the map surfaces's y-direction
             * is given by my = (-map->tw/2, -map->th/2).
             * The tiles belong in the position 
             * with center i * mx + j * my.
             * In addition to this a common translation in the 
             * screen's x-direction of (map->tw/2 * (-1 + map->h), 0)
             * should be made to ensure that the left hand side of the
             * map is visible in the screen area.
             * Finally, a translation of (0, map->th / 2 * (-1 + map->h
             *  -1 + map-w) is neded to ensure that the top of the map
             *  is visible in the screen area. */
            dst.x = map->tw/2 * (i - j - 1 + map->h);
            dst.y = -map->th/2 * (i + j + 2 - map->h
                    - map->w);

#ifdef DEBUG_MODE
            fprintf(stderr, "Center of tile: %d %d ", dst.x, dst.y);
#endif

            index = ((unsigned char(*)[map->w])map->map)[j][i];
#ifdef DEBUG_MODE
            fprintf(stderr, "Index is: %d\n", index);
#endif
            src.x = (index % (map->tilepalette->w / map->tw)) * map->tw;
            src.y = (index / (map->tilepalette->w / map->tw)) * map->th;


            SDL_BlitSurface(map->tilepalette, &src, map_image, &dst);
        }
            
 
    /* create an openGL texture and bind the sprite's image
     * to it */
    glGenTextures(1, &(map->texname));
    glBindTexture(GL_TEXTURE_2D, map->texname);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map_image->w,
            map_image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
            map_image->pixels);

    SDL_FreeSurface(map_image);

    return 1;
}

void isomap_paint(struct isomap *map, struct isoeng *engine, double frame)
{
    struct isogrp *map_actors = isoeng_get_group(engine, map->groups);
    struct isols *pg;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, map->map_rect.w, 0, map->map_rect.h, 0.5, 0.0);
    glViewport(map->map_rect.x, map->map_rect.y, map->map_rect.w,
            map->map_rect.h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, map->texname);

    glBegin(GL_QUADS);

    glTexCoord2d(0.0, 0.0);
    glVertex2i(0, map->rh);

    glTexCoord2d(0.0, 1.0);
    glVertex2i(0, 0);

    glTexCoord2d(1.0, 1.0);
    glVertex2i(map->rw, 0);

    glTexCoord2d(1.0, 0.0);
    glVertex2i(map->rw, map->rh);

    glEnd();

    


    /* Paint all the actors associated with this map */
    for(pg = map_actors->ls; pg != NULL; pg = pg->next){
        isoactor_paint(pg->actor, map, frame);
    }

    glDisable(GL_TEXTURE_2D);
    return;
}

void isomap_iterate(struct isomap *map, struct isoeng *engine)
{
    struct isogrp *map_actors = isoeng_get_group(engine, map->groups);
    struct isols *pg, *qg;
    struct isoactor_overlap overlap;

    for(pg = map_actors->ls; pg != NULL; pg = pg->next){

        /* Checking if actor has strayed over the edge of the map,
         * in which case the over-boundary handler should be called */
        if(pg->actor->x / map->itw > map->w ||
           pg->actor->y / map->itw > map->h ||
           pg->actor->x / map->itw < 0 ||
           pg->actor->y / map->itw < 0){
 #ifdef DEBUG_MODE
            fprintf(stderr, "Actor has strayed over boundary\n");
#endif
            map->ob_handler(map, pg->actor);
        }
          
        /* Get the rectangle that the map is in on the map */    
        isoactor_map_calc_overlap(pg->actor, map,
                &overlap);
        /* Check for collision of the actor's collision bitmap
         * with the collision bitmap of the outskirts of the map */
        if(isoactor_map_bw_c_detect(pg->actor, map,
                0, &overlap)){
#ifdef DEBUG_MODE
            fprintf(stderr, "Actor has strayed outside of the map.\n");
#endif
            map->ob_handler(map, pg->actor);
        }


        struct map_handle_ls *ph;
        int i;
        for(ph = pg->actor->map_handlers[map->uid];
                ph != NULL; ph = ph->next){
            for(i = 1; i <= strlen(map->c_key); i++){
                if(ph->tiles & (isobf_t)1 << (i-1)){

                    if(isoactor_map_bw_c_detect(pg->actor, map,
                                i, &overlap)){
#ifdef DEBUG_MODE
                        fprintf(stderr, "Collision "
                                "with tile of type: %d\n", i);
#endif

                        ph->map_handler(pg->actor, map);
                    }
                }
            }
        }
    }

    if(map->i_handler != NULL){
        map->i_handler(map, engine);
    }

    /* Checking for actor-actor collision on this map */
    for(pg = map_actors->ls; pg != NULL; pg = pg->next){
        struct isoactor *a1 = pg->actor;
        for(qg = pg->next; qg != NULL; qg = qg->next){
            struct isoactor *a2 = qg->actor;

            /* Check if actors are primed to detect collision with
             * one another */
            if(a1->collision_groups[map->uid] & a2->groups ||
                    a2->collision_groups[map->uid] & a1->groups){

                /* Check if the actors' simple rectangle overlap
                 * exists */
                if(isoactor_calc_overlap(a1, a2, map, &overlap)){

                    /* Check if actors' are bitwise colliding */
                    if(isoactor_bw_c_detect(a1, a2, map, &overlap)){
                        struct actor_handle_ls *pahl;

                        /* Call appropriate collision handlers */

                        if(a1->collision_groups[map->uid] & a2->groups){

                            for(pahl = a1->actor_handlers[map->uid];
                                    pahl != NULL; pahl = pahl->next){
                                if(pahl->groups & a2->groups)
                                    pahl->actor_handler(a1, a2);
                            }
                        }


                        if(a2->collision_groups[map->uid] & a1->groups){
                            for(pahl = a1->actor_handlers[map->uid];
                                    pahl != NULL; pahl = pahl->next){
                                if(pahl->groups & a1->groups)
                                    pahl->actor_handler(a2, a1);
                            }
                        }
                    }
                }
            }
        }
    }
}

void isomap_ob_handler(struct isomap *map, struct isoactor *actor)
{
    actor->x = actor->px;
    actor->y = actor->py;
    actor->vx = 0;
    actor->vy = 0;

    return;
}

