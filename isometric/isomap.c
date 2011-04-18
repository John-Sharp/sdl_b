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

void iso_transform(struct isomap *map, double *x, double *y)
{
    double x_temp, y_temp;

    x_temp = project_x(map->ri, *x, *y);
    y_temp = project_y(map->ri, *x, *y);

    *x = x_temp + map->h * map->tw/2;
    *y = y_temp;

    return;
}

struct isomap *isomap_create(const SDL_Rect *map_rect,
        int w, int h, int tw, const char *filename,
        const char *k, const char *m)
{
    struct isomap *map;

    map = malloc(sizeof(*map));
    if(!map)
        return NULL;

    map->w = w;
    map->h = h;

    map->tw = tw;
    map->th = ceil((float)tw / sqrt(3));

    map->ri[0][0] = 1 / sqrt(2);
    map->ri[0][1] = -1 / sqrt(2);
    map->ri[1][0] = 1/ sqrt(6);
    map->ri[1][1] = 1/ sqrt(6);

    map->ir[0][0] = 1 / sqrt(2);
    map->ir[0][1] = 1 / sqrt(6);
    map->ir[1][0] = -1 / sqrt(2);
    map->ir[1][1] = 1 / sqrt(6);

    map->coord_trans = iso_transform;

    memcpy(&(map->map_rect), map_rect, sizeof(map->map_rect));

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

    return map;
}

int isomap_from_string(struct isomap *map, const char *k, const char *m)
{
    SDL_Surface *map_image;
    SDL_Rect dst, src;
    int j, i, z = 0;
    unsigned char (*map_ptr)[map->w] = (unsigned char(*)[map->w])map->map;
    unsigned char index;

    for(j = 0; j < map->h; j++)
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
            /* The vector going from the center of one tile to the 
             * center of the next tile in the map's x-direction
             * is given by (map->tw/2, map->th/2).
             * The vector going from the center of one tile to the
             * center of the next tile in the map's y-direction
             * is given by (-map->tw/2, map->th/2).
             * The tiles belong in the position 
             * with center i * mx + j * my.
             * In addition to this a common translation in the 
             * screen's x-direction of (map->tw/2 * (-1 + map->h), 0)
             * should be made to ensure that the left hand side of the
             * map is visible in the screen area */
            dst.x = map->tw/2 * (i - j - 1 + map->h);
            dst.y = map->th/2 * (i + j); 

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

void isomap_paint(struct isomap *map)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, map->map_rect.w, 0, map->map_rect.h,0.5, 0.0);
    glViewport(map->map_rect.x, map->map_rect.y, map->map_rect.w,
            map->map_rect.h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, map->texname);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(0, 0, 0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(0, (float)map->rh, 0);

    glTexCoord2f(1.0, 1.0);
    glVertex3f((float)map->rw, (float)map->rh, 0);

    glTexCoord2f(1.0, 0.0);
    glVertex3f((float)map->rw, 0 , 0);

    glEnd();
    

    glDisable(GL_TEXTURE_2D);

    return;
}



