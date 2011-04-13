#include "text_jactor.h"

void text_jactor_free(struct text_jactor *actor)
{
    SDL_FreeSurface(actor->p2_surface);
    
    g_object_unref(actor->layout);
    pango_font_description_free(actor->font_description);

    jactor_free((struct jactor*)actor);
}

struct text_jactor *text_jactor_create(int w, int h, struct jmap *map,
        const char *bg_img_filename)
{
    struct text_jactor *actor;
    SDL_Surface *bg_temp;
    Uint32 alpha;

    actor = (struct text_jactor*)jactor_create(w, h, map, bg_img_filename);
    actor = realloc(actor, sizeof(*actor));

    actor->core.paint = text_jactor_paint;

    bg_temp = IMG_Load(bg_img_filename);

    actor->p2_surface = SDL_CreateRGBSurface(
            SDL_SWSURFACE,
            actor->core.p2w,
            actor->core.p2h,
            32,
            RMASK, GMASK, BMASK, AMASK);

    if(!actor->p2_surface){
        fprintf(stderr, "Error creating a surface for the sprite\n");
        jactor_free((struct jactor*)actor);
        return NULL;
    }

    SDL_SetAlpha(actor->p2_surface, 0, SDL_ALPHA_OPAQUE);

    if(SDL_MUSTLOCK(actor->p2_surface))
        SDL_LockSurface(actor->p2_surface);

    actor->cairo_surface = cairo_image_surface_create_for_data(
            actor->p2_surface->pixels,
            CAIRO_FORMAT_RGB24,
            actor->p2_surface->w,
            actor->p2_surface->h,
            actor->p2_surface->pitch);

    actor->cr = cairo_create(actor->cairo_surface);

    actor->font_description = pango_font_description_new();
    pango_font_description_set_family(actor->font_description, "serif");
    pango_font_description_set_weight(actor->font_description,
            PANGO_WEIGHT_BOLD);
    pango_font_description_set_absolute_size(
            actor->font_description,
            20 * PANGO_SCALE);

    actor->layout = pango_cairo_create_layout(actor->cr);
    pango_layout_set_font_description(actor->layout, actor->font_description);
    pango_layout_set_width(actor->layout, w * PANGO_SCALE); 
    pango_layout_set_height(actor->layout, h * PANGO_SCALE); 

    if(SDL_MUSTLOCK(actor->p2_surface))
        SDL_UnlockSurface(actor->p2_surface);

    actor->text[0] = '\0';

    /* create an openGL texture and bind the sprite's image
     * to it */
    glGenTextures(1, &(actor->text_tex));
    glBindTexture(GL_TEXTURE_2D, actor->text_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, actor->p2_surface->w,
            actor->p2_surface->h,
            0, GL_RGBA, GL_UNSIGNED_BYTE, actor->p2_surface->pixels);


    return actor;
}

void text_jactor_set_text(struct text_jactor *actor, const char *text)
{
    int offset_x, offset_y;

    offset_x = (actor->core.p2w - actor->core.w)/2;
    offset_y = (actor->core.p2h - actor->core.h)/2;


    strncpy(actor->text, text, TEXTLENGTH - 1);


    if(SDL_MUSTLOCK(actor->p2_surface))
        SDL_LockSurface(actor->p2_surface);

    pango_layout_set_text(actor->layout, actor->text, -1);
    cairo_move_to(actor->cr, offset_x,
           offset_y);
    pango_cairo_show_layout(actor->cr, actor->layout);

    if(SDL_MUSTLOCK(actor->p2_surface))
        SDL_UnlockSurface(actor->p2_surface);

    glBindTexture(GL_TEXTURE_2D, actor->text_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, actor->p2_surface->w,
            actor->p2_surface->h,
            0, GL_RGBA, GL_UNSIGNED_BYTE, actor->p2_surface->pixels);

    return;
}

void text_jactor_paint(struct text_jactor *actor, double frames_so_far)
{
    jactor_paint(&(actor->core), frames_so_far);

    
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, actor->text_tex);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-(float)actor->core.p2w/2.0,
               -(float)actor->core.p2h/2.0, 0.0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-(float)actor->core.p2w/2.0,
                (float)actor->core.p2h/2.0, 0.0);

    glTexCoord2f(1.0, 1.0);
    glVertex3f((float)actor->core.p2w/2.0,
               (float)actor->core.p2h/2.0, 0.0);

    glTexCoord2f(1.0, 0.0);
    glVertex3f((float)actor->core.p2w/2.0,
              -(float)actor->core.p2h/2.0,
               0.0);

    glEnd();
    

    glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
   
    return;
}



