#include "SDL.h"
#include <SDL_image.h>
#include <stdlib.h>
#include <math.h>
#include "../jaunty/jutils.h"

enum{
    SCREEN_W = 1024,
    SCREEN_H = 768 
};

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;

   default:
        break;           /* shouldn't happen, but avoids warnings */
    } // switch
}

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    } // switch
}



int main(int argc, char **argv)
{
    int flags = SDL_HWSURFACE;
    SDL_Surface *screen, *image, *tmp;
    int bpp = 0;
    int carry_on = 1;
    Uint32 black;
    double theta = 0;

    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
    black = SDL_MapRGB(screen->format, 0, 0, 0);
    if(!screen){
        fprintf(stderr, "Failed to open screen!\n");
        return 1;
    }

    image = IMG_Load("cue.png");
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    if(!image){
        fprintf(stderr, "Could not load the image!\n");
        return 1;
    }
    
    image = SDL_DisplayFormat(image);

    SDL_BlitSurface(image, NULL, screen, NULL);

    tmp = SDL_ConvertSurface(screen, screen->format, flags);

    SDL_UpdateRect(screen, 0, 0, 0, 0);


    while(carry_on){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT)
                carry_on = 0;
            else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_a){
                    double a[2] = {sin(-theta), cos(-theta)};
                    int i, j;
                    Uint32 colour;
                    theta += M_PI / 160;

                    SDL_FillRect(tmp, NULL, black);

                    //colour = SDL_MapRGB(screen->format, 255, 0, 0);
                    if(SDL_MUSTLOCK(screen))
                        SDL_LockSurface(screen);

                    for(i = 0; i < SCREEN_W; i++){
                        for(j = 0; j < SCREEN_H; j++){
                            
                            int i_src, j_src;

                            i_src = bankround(((double)j
                                    - (double)SCREEN_H/2) * a[0] 
                                    + ((double)i
                                    - (double)SCREEN_W/2)* a[1] 
                                + (double)SCREEN_W / 2);
                            j_src = bankround(((double)i
                                       - (double)SCREEN_W/2) * -a[0]
                                    + ((double)j
                                       -(double)SCREEN_H /2) * a[1]
                                + (double)SCREEN_H /2);

                            if(i_src >= 0 && i_src < image->w &&
                                    j_src < image->h && j_src >= 0){
                                colour = getpixel(image, i_src, j_src);
                            }else{
                                colour = black;
                            }
                            putpixel(tmp, i, j, colour);
                        }
                    }


                    if(SDL_MUSTLOCK(screen))
                        SDL_UnlockSurface(screen);

                    SDL_BlitSurface(tmp, 0, screen, 0);
                    SDL_UpdateRect(screen, 0, 0, 0, 0);

                }
            }
        }

    }

    return 0;
}
