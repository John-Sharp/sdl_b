/* An extension of the jactor structure, that contains extra components
 * to allow description of some pango-cairo text that exists on the 
 * actor's surface */

#ifndef TEXT_JACTOR_H
#define TEXT_JACTOR_H

#include "../jaunty/jen.h"
#include <cairo.h>
#include <pango/pangocairo.h>
#include <string.h>

#define TEXTLENGTH 70

struct text_jactor{

    struct jactor core; /* jactor attributes this struct 'inherits' */

    SDL_Surface *p2_surface; /* surface formatted for associating to a openGL
                               texture */

    char text[TEXTLENGTH]; /* Text that will be written on the background
                              when the paint function is called */

    cairo_surface_t *cairo_surface; /* cairo surface that's around
                                       'p2_surface' */

    cairo_t *cr; /* cairo brush used for painting the text */

    PangoLayout *layout; /* layout that describes the text */

    
    GLuint text_tex; /* Texture containing the text to be drawn on top
                        of the background */


    PangoFontDescription *font_description; /* variable that describes
                                               the font */
    
};

/* Free the resources allocated to 'actor' */
void text_jactor_free(struct text_jactor *actor);

/* Create text_jactor width 'w', height 'h', associated with 'map' and 
 * background image contained in the file named by 'bg_img_filename' */
struct text_jactor *text_jactor_create(int w, int h, struct jmap *map,
        const char *bg_img_filename);

/* Sets the text to 'text', returns 1 if successful, 0 if error */
void text_jactor_set_text(struct text_jactor *actor, const char *text);

/* Blits the 'actor' onto the screen */
void text_jactor_paint(struct text_jactor *actor, double frames_so_far);

#endif
