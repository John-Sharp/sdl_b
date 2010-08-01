//Functions for creating/loading/drawing/destroying quite a generic map.
#include "jmap.h"
#include "jen.h"
#include <math.h>
#include "SDL.h"
#include <SDL_image.h>
#include "jutils.h"

#define ensure_between(a, lower, higher) \
    (a < lower ? lower : (a > higher ? higher : a))

typedef enum{
    BG_IMG,
    COLLISION
}map_type;

static void jmap_from_string_internal(jmap *map, const char *k, const char *m,
        map_type mtype);


void jmap_free(jmap *map)
{
    SDL_FreeSurface(map->tilepalette);
    free(map->map);
    free(map);
    return;
}


jmap *jmap_create(int w, int h)
{
    jmap *map;

    map = malloc(sizeof(*map));
    if(!map)
        return NULL;

    map->w = w;
    map->h = h;

    map->map = malloc(sizeof(*(map->map)) * w * h);
    if(!map->map){
        jmap_free(map);
        return NULL;
    }

    map->c_map = malloc(sizeof(*(map->map)) * w * h);
    if(!map->c_map){
        jmap_free(map);
        return NULL;
    }

    return map;
}

int jmap_load_tilepalette(jmap *map, const char *filename, int tw, int th)
{
	SDL_Surface *tmp;
	map->tw = tw;
	map->th = th;
	tmp = IMG_Load(filename);
	if(!tmp)
	{
		fprintf(stderr, "Could not load '%s'!\n", filename);
		return -1;
	}
	map->tilepalette = SDL_DisplayFormat(tmp);
	if(!map->tilepalette)
	{
		fprintf(stderr, "Could not convert '%s'!\n", filename);
		return -1;
	}
	SDL_FreeSurface(tmp);
	return 0;
}

void jmap_from_string(jmap *map, const char *k, const char *m)
{
    jmap_from_string_internal(map, k, m, BG_IMG);
    return;
}

void jmap_c_map_from_string(jmap *map, const char *k, const char *m)
{
    jmap_from_string_internal(map, k, m, COLLISION);
    return;
}


//todo: need to add error checking
static void jmap_from_string_internal(jmap *map, const char *k, const char *m,
        map_type mtype)
{
    char c;
    int z, i, j;
    unsigned char index;
    unsigned char (*map_ptr)[map->w];

    switch(mtype){
        case BG_IMG:
            map_ptr = (unsigned char(*)[map->w])map->map;
            break;
        case COLLISION:
            map_ptr = (unsigned char(*)[map->w])map->c_map;
            break;
        default:
            return;
    }


    z = 0;
    
    for(j = 0; j < map->h; j++)
        for(i = 0; i < map->w; i++){
            c = m[z];
            index = (strchr(k, c) - k) / (sizeof(unsigned char));
            map_ptr[j][i] = index;
            z++;
        }
        

    return;
}


void jmap_paint(jmap *map, SDL_Surface *screen, SDL_Rect *part)
{
    int i, j;
    int i_ll, i_ul, j_ll, j_ul;
    int index;
    SDL_Rect src, dst;

    src.w = map->tw;
    src.h = map->th;

    dst.w = map->tw;
    dst.h = map->th;

    if(part == NULL){
        i_ll = 0;
        i_ul = map->w;
        j_ll =0;
        j_ul = map->h;
    }else{
        i_ll = floor((double)part->x / (double)map->tw);
        i_ul = ceil((double)(part->x + part->w) / (double)map->tw);
        j_ll = floor((double)part->y / (double)map->th);
        j_ul = ceil((double)(part->y + part->h) / (double)map->th);
    }

    for(j = j_ll; j < j_ul; j++)
        for(i = i_ll; i < i_ul; i++){
            dst.x = i * map->tw;
            dst.y = j * map->th;

            index = ((unsigned char(*)[map->w])map->map)[j][i];
            src.x = (index % src.w) * map->tw;
            src.y = (index / src.w) * map->th;

            SDL_BlitSurface(map->tilepalette, &src, screen, &dst); 
        }

    return;
}


jsides jmap_collision_detect(jmap *map, jactor *actor, jcoll *c_info,
        int tile_mask)
{
    jcoll spare_c_info;
    int i;

    if(c_info == NULL){
        c_info = &spare_c_info;
    }

    i = jmap_first_tile_touched(map, actor->px, actor->py,
            actor->x, actor->y,
            tile_mask);

    if(i==-1){
        return NONE;
    }

    map->map[i] = 1;

    c_info->side = jmap_tile_intersection_point(map,
            i, actor->px, actor->py, actor->x, actor->y,
            &(c_info->x), &(c_info->y));

    c_info->c_type = A_T;

    return c_info->side;
}

int jmap_first_tile_touched(jmap *map, double x1, double y1,
        double x2, double y2, int tile_mask)
{
    double m, c;
    double f_x, f_xplus1;
    double *y_lower = &f_x, *y_upper = &f_xplus1;
    int dy = 1;
    int dx = 1;
    int i, j;



    unsigned char (*c_map)[map->w] = (unsigned char(*)[map->w])map->c_map;

    int tile_x1 = floor(x1/(double)map->tw);
    int tile_y1 = floor(y1/(double)map->th);
    int tile_x2 = floor(x2/(double)map->tw);
    int tile_y2 = floor(y2/(double)map->th);

    tile_x1 = ensure_between(tile_x1, 0, map->w - 1);
    tile_y1 = ensure_between(tile_y1, 0, map->h - 1);
    tile_x2 = ensure_between(tile_x2, 0, map->w - 1);
    tile_y2 = ensure_between(tile_y2, 0, map->h - 1);

    if(x1 > x2){
        dx *= -1;
    }

    if(y1 > y2)
        dy *= -1;
        
    //gradient and intercept are calculated relative to the grid formed
    //by the background tiles
    calc_m_c(x1/(double)map->tw, y1/(double)map->th,
            x2/(double)map->tw, y2/(double)map->th, &m, &c);
 
    //when the gradient is infinite, all tiles along
    //a vertical line are checked to see if they match 'tile_mask'
    if(m == HUGE_VAL || -m == HUGE_VAL ){
        for(j = tile_y1; j != tile_y2 + dy; j+=dy){
            if(1 << c_map[j][tile_x1] & tile_mask)
                return j * map->w + tile_x1;
        }
        return -1;
    }

    if(m < 0){
            y_lower = &f_xplus1;
            y_upper = &f_x;
    }

    for(i = tile_x1; i != tile_x2 + dx; i+=dx){
        f_x = m * (double)i + c;
        f_xplus1 = m * (double)(i + 1) + c;
        for(j = tile_y1; j != tile_y2 + dy; j+=dy){
            /* addition/substraction of 0.00001 is necessary because of 
             * rounding errors that can occur in the previous steps */
            if(*y_lower <= (double)(j + 1.00001)
                    && *y_upper >= (double)j - 0.00001){
                if(1 << c_map[j][i] & tile_mask){
                    return j * map->w + i;
                }
            }
        }
    }

    return -1;
}

jsides jmap_tile_intersection_point(jmap *map, unsigned char tile_index,       
        double x1, double y1, double x2, double y2, 
        double *x, double *y)
{
    /* Calculate coordinates of the verticies of the tile */                 
    double tile_xmin = (tile_index % map->w) * map->tw;
    double tile_xmax = (tile_xmin + map->tw);
    double tile_ymin = (tile_index / map->w) * map->th;
    double tile_ymax = (tile_ymin + map->th);

    double x_intersect[2], y_intersect[2];
    double displacement[2];
    int i = 0;
    jsides collision_side[2] = {0, 0};


    //check for intersection with top of tile
    if(lines_intersect(x1, y1, x2, y2, tile_xmin, tile_ymin, tile_xmax,
                tile_ymin, &x_intersect[i], &y_intersect[i]) == 1){
        collision_side[i] = TOP;

        if(x_intersect[i] == tile_xmin)
            collision_side[i] |= LEFT;
        else if(x_intersect[i] == tile_xmax)
            collision_side[i] |= RIGHT;
        i++;
    }

    //check for intersection with bottom
    
    if(lines_intersect(x1, y1, x2, y2, tile_xmin, tile_ymax, tile_xmax,
                tile_ymax, &x_intersect[i], &y_intersect[i]) == 1){
        collision_side[i] = BOTTOM;

        if(x_intersect[i] == tile_xmin)
            collision_side[i] |= LEFT;
        else if(x_intersect[i] == tile_xmax)
            collision_side[i] |= RIGHT;
        i++;
    }
    

    //check for intersection with left
    if(!(collision_side[0]&LEFT) && i<2){
          
        if(lines_intersect(x1, y1, x2, y2, tile_xmin, tile_ymin, tile_xmin,
                    tile_ymax, &x_intersect[i], &y_intersect[i]) == 1){
            collision_side[i] = LEFT;
            i++;
        }
    }

    //intersection with right
    if(!(collision_side[0]&RIGHT) && i<2){
        if(lines_intersect(x1, y1, x2, y2, tile_xmax, tile_ymin, tile_xmax,
                    tile_ymax, &x_intersect[i], &y_intersect[i]) == 1){
            collision_side[i] = RIGHT;
            i++;
        }
    }

    if(i==0){
        fprintf(stderr, "\nLine does not intersect tile, %d\n", i);
        return 0;
    }else if(i==1){
        /* If the tile is only intersected at one point return the 
         * information about this point */
        *x = x_intersect[0];
        *y = y_intersect[0];
        return collision_side[0];
    }

    /*If the tile is intersected twice then return information about
     * the collision that happened closest to (x1, y1) */

    displacement[0] = sqrt(pow(x1 - x_intersect[0], 2) +
            pow(y1 - y_intersect[0], 2));
    displacement[1] = sqrt(pow(x1 - x_intersect[1], 2) +
            pow(y1 - y_intersect[1], 2));
    
    if(displacement[0] < displacement[1]){
        *x = x_intersect[0];
        *y = y_intersect[0];
        return collision_side[0];
    }else{
        *x = x_intersect[1];
        *y = y_intersect[1];
        return collision_side[1];
    }
}

