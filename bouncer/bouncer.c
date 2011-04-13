/* TODO make jen_group_free function, and make the freeing
 * of jen truely clear up after herself */
/* TODO improve the actor actor collision detect so that 
 * it functions for high speed collisions */
#include "../jaunty/jen.h"

#include "text_jactor.h"
#include "SDL.h"
#include <sys/types.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <math.h>

#define FPS 900.0 /* frames per second */
#define FRICTION (150.0 / (FPS * FPS)) /* magnitude of the friction force */
#define MAX_SPEED (25.0) /* maximum speed of the game (in meters per frame */
#define CUE_ROT_RAD 530 /* radius around which the cue rotates */

struct bcr_en{
    struct jen core;
    int shots;
    struct jplayer *player1;
    struct jplayer *player2;
};

void bcr_en_free(struct bcr_en *en)
{
    free(en->player1);
    free(en->player2);
    jen_free((struct jen *)en);
    return;
}

enum {
    SCREEN_W = 800,
    SCREEN_H = 500,
    TILE_W = 25,
    TILE_H = 25,
    MAP_W = 32,
    MAP_H = 18, 
    BALL_DIAM = 30,
    SBOARD_W = 32,
    SBOARD_H = 2,
    POTTED = 1
};

/* the groups of actors */
enum groups {
    CUE_BALL = 1,
    YELLOW_BALL = 1<<1,
    RED_BALL = 1<<2,
    CUE = 1<<3,
    EIGHT_BALL = 1<<4,
    BALLS = CUE_BALL | YELLOW_BALL | RED_BALL | EIGHT_BALL,
    OBJ_BALLS = YELLOW_BALL | RED_BALL | EIGHT_BALL,
    PLAYER1_TXT = 1<<5,
    PLAYER2_TXT = 1<<6,
    MESSAGE_TXT = 1<<7,
    SB_ITEMS = PLAYER1_TXT | PLAYER2_TXT | MESSAGE_TXT
};

/* the various game states that are possible */
enum game_states{
    BALLS_MOVING = 1,
    CUE_MOVING = 1<<1,
    CUE_POSITIONING = 1<<2,
    USER_INPUT = 1<<3,
    CUE_CHARGING = 1<<4,
    CUE_DISCHARGING = 1<<5,
    CUE_BALL_POTTED = 1<<6,
    RED_BALL_POTTED = 1<<7,
    YELLOW_BALL_POTTED = 1<<8,
    PEN_SHOT = 1<<9,
    FOUL_COMMITED = 1<<10,
    CHANGE_PLAY = 1<<11,
    CB_CLEAN = 1 << 12,
    PLAYING_FROM_BAULK = 1<<13,
    BREAK_SHOT = 1<<14,
    EIGHT_BALL_POTTED = 1<<15,
    OPEN = 1<<16
};

/* The various actor states that are possible */
enum {
    HIT_CUSHION = 1
} actor_states;

/* Some states that are only important during 
 * the break */
enum {
    FOUR_CUSHION_HITS = 1<<2
} break_constants;


unsigned int valid_break(struct jen *engine)
{
    struct jactor_ls *balls;
    int total_balls = 0;
    unsigned int return_value = 0;
    int cushion_hits = 0;

    if(engine->game_state & EIGHT_BALL_POTTED)
        return EIGHT_BALL_POTTED;


    if(engine->game_state & CUE_BALL_POTTED)
        return_value |= CUE_BALL_POTTED;


    balls = jen_get_groups(engine, RED_BALL);
    for(;balls != NULL; balls = balls->next){
        if(balls->actor->actor_state & HIT_CUSHION){
            cushion_hits++;
        }
        total_balls++;
    }

    if(total_balls < 7){
        return_value |= RED_BALL_POTTED;
    }

    total_balls = 0;

    balls = jen_get_groups(engine, YELLOW_BALL);
    for(;balls != NULL; balls = balls->next){
        if(balls->actor->actor_state & HIT_CUSHION){
           cushion_hits++;
        }
        total_balls++;
    }

    if(total_balls < 7){
        return_value |= YELLOW_BALL_POTTED;
    }


    balls = jen_get_groups(engine, EIGHT_BALL);
    for(;balls != NULL; balls = balls->next){
       if(balls->actor->actor_state & HIT_CUSHION){
           cushion_hits++;
       }
    }

    if(cushion_hits >= 4){
        return_value |= FOUR_CUSHION_HITS;
    }

    return return_value;

}

void update_scoreboard(struct bcr_en *engine)
{
    struct text_jactor *player_text;
    const char *text;

    if(engine->player1->colour == RED_BALL){
       text = "Player 1 (Red) | Player 2 (Yellow)";
    }else{
       text = "Player 1 (Yellow) | Player 2 (Red)";
    } 


    player_text = jen_get_groups((struct jen *)engine, PLAYER1_TXT)->actor;
    text_jactor_set_text((struct text_jactor *)player_text, text);

   return; 
}


void add_score(struct bcr_en *en, unsigned int to_team)
{
    if(en->core.current_player->colour == to_team
            && !(en->core.game_state & BREAK_SHOT)){
        fprintf(stderr, "Congrats, you potted a ball\n");
        en->core.game_state &= ~OPEN; 
        update_scoreboard(en);
        en->shots = 1;
    }else if(!(en->core.game_state & BREAK_SHOT)){
        fprintf(stderr, "Foul commited since wrong ball "
                "potted\n");
        en->core.game_state |= FOUL_COMMITED;
    }

    /*
    if(en->player1->colour == to_team )
        en->player1->score++;
    else
        en->player2->score++;
        */

    /*
    printf("Player 1 (%s): %d\n", (en->player1->colour == RED_BALL ? "Red" 
                : "Yellow"), en->player1->score);
    printf("Player 2 (%s): %d\n", (en->player2->colour == RED_BALL ? "Red" 
                : "Yellow"), en->player2->score);
                */

}



/* Function that should be called every time a foul is commited */
void foul_commited(jen *engine)
{
    engine->game_state |= FOUL_COMMITED;
    return;
}

void load_level(jmap *bg_map, int level)
{
    const char *k;
    const char *m;
    const char *ck;
    const char *cm;

    if(level == 0){
        k = "...a";

        m = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

        ck = "";
        cm = "";
    }else if(level == 1){
        k = "abcdefghi";

        m = "fgbbbbbbbbbbbbbfgbbbbbbbbbbbbbfg"
            "haaaaaaaaaaaaaaaaaaaaaaaaaaaaaai"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaac"
            "faaaaaaaaaaaaaaaaaaaaaaaaaaaaaag"
            "hidddddddddddddhidddddddddddddhi";

        ck = "abcd";

        cm = "bbaaaaaaaaaaaaabbaaaaaaaaaaaaabb"
             "bdddddddddddddddcccccccccccccccb"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "adddddddddddddddccccccccccccccca"
             "bdddddddddddddddcccccccccccccccb"
             "bbaaaaaaaaaaaaabbaaaaaaaaaaaaabb";

    }

    jmap_from_string(bg_map, k, m);

    jmap_c_map_from_string(bg_map, ck, cm);

    return;
}

/* Set the cue moving towards the cue ball */
void set_cue_motion(jen *engine)
{
    jactor_ls *cue_ls;
    jactor_ls *cue_ball_ls;

    /* Put code to move the cue to the cue ball here */
    engine->game_state |= CUE_MOVING;
    engine->game_state |= CB_CLEAN;

    cue_ls = jen_get_groups(engine, CUE);
    cue_ball_ls = jen_get_groups(engine, CUE_BALL);
    cue_ls->actor->x = cue_ball_ls->actor->x; 
    cue_ls->actor->px = cue_ball_ls->actor->x; 
    cue_ls->actor->y = cue_ball_ls->actor->y; 
    cue_ls->actor->py = cue_ball_ls->actor->y;


    return;
}

void dot_iterator(jactor *a, jen *engine)
{
    
    if(magnitude(a->v_x, a->v_y) <= FRICTION)
        a->a_x = a->a_y = a->v_x = a->v_y = 0;

    return;
}

/* Collision handler that is used when the cue-ball is being
 * positioned prior to a baulk shot */
int cue_baulk_collider(jactor *a, jcoll *c_info, jen *en)
{
    if(c_info->c_type == A_T){
        if(c_info->side & BOTTOM){
            a->v_y = 0;
            a->x = a->px = c_info->x;
            a->y = a->py = c_info->y + 0.001;
        }else if(c_info->side & LEFT){
            a->v_x = 0;
            a->x = a->px = c_info->x - 0.001;
            a->y = a->py = c_info->y;
        }else if(c_info->side & TOP){
            a->v_y = 0;
            a->x = a->px = c_info->x;
            a->y = a->py = c_info->y - 0.001;
        }else if(c_info->side & RIGHT){
            a->x = a->px = c_info->x + 0.001;
            a->y = a->py = c_info->y;
        }
    }

    return 0;
}





int dot_collider(jactor *a, jcoll *c_info, jen *en)
{
    /* angle of motion relative to the x-axis */
    double phi;

    if(c_info->c_type == A_T){
        if(c_info->c_key_num == 1){
                if(c_info->side & TOP){
                    a->x = c_info->x;
                    a->px = c_info->x;
                    a->py = c_info->y;
                    a->y = c_info->y - 1;
                    a->v_y = -a->v_y;
                }else if(c_info->side & BOTTOM){
                    a->x = c_info->x;
                    a->px = c_info->x;
                    a->py = c_info->y;
                    a->y = c_info->y + 1;
                    a->v_y = -a->v_y;
                }
                if(c_info->side & LEFT){
                    a->px = c_info->x;
                    a->py = c_info->y;
                    a->x = c_info->x-1;
                    a->y = c_info->y;
                    a->v_x = -a->v_x;
                }else if(c_info->side & RIGHT){
                    a->px = c_info->x;
                    a->py = c_info->y;
                    a->x = c_info->x + 1;
                    a->y = c_info->y;
                    a->v_x = -a->v_x;
                }

                phi = x_pos_angle(a->v_x, a->v_y);

                a->a_x = - FRICTION * cos(phi);
                a->a_y = - FRICTION * sin(phi);
                
                if(c_info->c_key_num == 1){
                    a->actor_state |= HIT_CUSHION;
                }
        /* Following code is executed if the ball goes into a hole */
        }else if(c_info->c_key_num == (1 << 1)){ 
            if(a->group_num == CUE_BALL){
                /* Need to reset the game here */
                en->game_state |= CUE_BALL_POTTED;
                en->game_state |= FOUL_COMMITED;
                fprintf(stderr, "You have potted the cue ball\n");

                return POTTED;
            }else if(a->group_num == RED_BALL){
                add_score((struct bcr_en *)en, RED_BALL);
            }else if(a->group_num == YELLOW_BALL){
                add_score((struct bcr_en *)en, YELLOW_BALL);
            }else if(a->group_num == EIGHT_BALL){
                fprintf(stderr, "8 ball gone\n");
                en->game_state |= EIGHT_BALL_POTTED;
            }
            //en->current_player->colour = c_info->actor->group_num;
            jen_del_jactor(en, a);
            return POTTED;
        }

    }else{

        /* Code that should execute upon the first collision of the cue-ball
         * with another ball */
        if(en->game_state & CB_CLEAN){

            if(en->current_player->colour != 0){
                /* Check the colour of the ball that was collided with */
                if(c_info->actor->group_num != en->current_player->colour){
                    fprintf(stderr, "Foul! Collision with a ball not belonging "
                            "to player.\n");
                    en->game_state |= FOUL_COMMITED;
                }
                /* If there has been a collision that means that the cue-ball
                 * is no longer 'clean' */
                en->game_state &= ~CB_CLEAN;
            }

        }

        double m1 = a->m, m2 = c_info->actor->m;
        /* angle between line joining centres of two colliding
         * actors and the x-axis */
        double theta = x_pos_angle((c_info->actor->x - a->x),
                (c_info->actor->y - a->y));

        double cos_theta = cos(theta);
        double sin_theta = sin(theta);

        /* vector between centre of 2nd actor and collision point */
        double a2cp_x = c_info->x - c_info->actor->x;
        double a2cp_y = c_info->y - c_info->actor->y;

        double dm = c_info->actor->w/2. - magnitude(a2cp_x, a2cp_y);

        a->x -= dm/2. * cos_theta;
        a->y -= dm/2. * sin_theta;
        c_info->actor->x += dm/2. * cos_theta;
        c_info->actor->y += dm/2. * sin_theta;


        /* velocity resolved parallel to the line of collision, prior
         * to collision (element [0]) and a variable for storing the 
         * velocity after the collision (element [1]) */
        double v1p[2] = {a->v_x * cos_theta + a->v_y * sin_theta, 0};
        double v2p[2] = {c_info->actor->v_x * cos_theta + 
            c_info->actor->v_y * sin_theta, 0};

        /* velocity resolved transverse to the line of collision */
        double v1t = -a->v_x * sin_theta + a->v_y * cos_theta;
        double v2t = -c_info->actor->v_x * sin_theta + 
            c_info->actor->v_y * cos_theta;


        v1p[1] = v1p[0] * (m1 - m2)/(m1 + m2)
            + v2p[0] * 2 * m2 / (m1 + m2);

        v2p[1] = v2p[0] * (m2 - m1)/(m1 + m2)
            + v1p[0] * 2 * m1 / (m1 + m2);

        /* transforming back into the original set of x and y
         * axes */
        a->v_x = v1p[1] * cos_theta + v1t * -sin_theta;
        a->v_y = -v1p[1] * -sin_theta + v1t * cos_theta;

        c_info->actor->v_x = v2p[1] * cos_theta + v2t * -sin_theta;
        c_info->actor->v_y = -v2p[1] * -sin_theta + v2t * cos_theta;

        
        phi = x_pos_angle(a->v_x, a->v_y);

        if(isnan(phi)){
            a->a_x = 0;
            a->a_y = 0;
        }else{

            a->a_x = - FRICTION * cos(phi);
            a->a_y = - FRICTION * sin(phi);
        }

        phi = x_pos_angle(c_info->actor->v_x, c_info->actor->v_y);

        if(isnan(phi)){
            c_info->actor->a_x = 0;
            c_info->actor->a_y = 0;
        }else{
            c_info->actor->a_x = - FRICTION * cos(phi);
            c_info->actor->a_y = - FRICTION * sin(phi);
        }


    }

    return 0;
}

void cue_iterator(jactor *a, jen *engine)
{
    if(engine->game_state & CUE_DISCHARGING){
        if(a->r + a->v_r < 525){
            jactor *cue_ball;
            double theta_r = a->theta * M_PI / 180;

            cue_ball = jen_get_groups(engine, CUE_BALL)->actor;
            cue_ball->v_x = a->v_r * sin(theta_r);
            cue_ball->v_y = a->v_r * cos(theta_r);

            cue_ball->a_x = FRICTION * sin(theta_r);
            cue_ball->a_y = FRICTION * cos(theta_r);

            engine->game_state &= ~CUE_DISCHARGING;
            engine->game_state |= BALLS_MOVING;

            a->r = CUE_ROT_RAD;
            a->v_r = 0;

        }
    }

    return;
}


/* Positions ball number 'ball_number' of the type 'ball_type' */
void position_ball(struct jactor *a, struct jmap *map,
        enum groups ball_type, int ball_number)
{
    int table_length = map->w * map->tw;
    int table_height = map->h * map->th;

    int black_ball_start_x =  3 * table_length / 4.; /*  x-coordinate of black 
                                                      *  ball's starting
                                                      *  position */

    int black_ball_start_y = table_height / 2.; /* y-coordinate of black *
                                                 * ball's starting position */


    if(ball_type == CUE_BALL){
        a->px = a->x = table_length / 4.; 
        a->py = a->y = table_height / 2. - 60;
    }else if(ball_type == EIGHT_BALL){
        a->px = a->x = black_ball_start_x; 
        a->py = a->y = black_ball_start_y;
    }else if(ball_type == YELLOW_BALL){

        if(ball_number == 1){
            a->px = a->x = black_ball_start_x - BALL_DIAM / 1.1;  
            a->py = a->y = black_ball_start_y + BALL_DIAM / 2.;
        }else if(ball_number == 2){
            a->px = a->x = black_ball_start_x;  
            a->py = a->y = black_ball_start_y - BALL_DIAM;
        }else if(ball_number == 3){
            a->px = a->x = black_ball_start_x + BALL_DIAM / 1.1;  
            a->py = a->y = black_ball_start_y + 1.5 * BALL_DIAM;
        }else if(ball_number == 4){
            a->px = a->x = black_ball_start_x + BALL_DIAM / 1.1;  
            a->py = a->y = black_ball_start_y - 0.5 * BALL_DIAM;
        }else if(ball_number == 5){
            a->px = a->x = black_ball_start_x + 2 * BALL_DIAM / 1.1;   
            a->py = a->y = black_ball_start_y + 1 * BALL_DIAM;
        }else if(ball_number == 6){
            a->px = a->x = black_ball_start_x + 2 * BALL_DIAM / 1.1;   
            a->py = a->y = black_ball_start_y;
        }else if(ball_number == 7){
            a->px = a->x = black_ball_start_x + 2 * BALL_DIAM / 1.1;   
            a->py = a->y = black_ball_start_y - 2 * BALL_DIAM;
        }
    }else if(ball_type == RED_BALL){

        if(ball_number == 1){
            a->px = a->x = black_ball_start_x - BALL_DIAM / 1.1;  
            a->py = a->y = black_ball_start_y - BALL_DIAM / 2.;
        }else if(ball_number == 2){
            a->px = a->x = black_ball_start_x - 2 * BALL_DIAM / 1.1;  
            a->py = a->y = black_ball_start_y;
        }else if(ball_number == 3){
            a->px = a->x = black_ball_start_x;  
            a->py = a->y = black_ball_start_y + BALL_DIAM;
        }else if(ball_number == 4){
            a->px = a->x = black_ball_start_x + BALL_DIAM / 1.1; 
            a->py = a->y = black_ball_start_y + 0.5 * BALL_DIAM;
        }else if(ball_number == 5){
            a->px = a->x = black_ball_start_x + BALL_DIAM / 1.1; 
            a->py = a->y = black_ball_start_y - 1.5 * BALL_DIAM;
        }else if(ball_number == 6){
            a->px = a->x = black_ball_start_x + 2 * BALL_DIAM / 1.1; 
            a->py = a->y = black_ball_start_y + 2 * BALL_DIAM;
        }else if(ball_number == 7){
            a->px = a->x = black_ball_start_x + 2 * BALL_DIAM / 1.1; 
            a->py = a->y = black_ball_start_y - BALL_DIAM;        
        }

    }

    return;
}


void rerack(struct jen *engine, struct jmap *map)
{
    struct jactor *actor;
    struct jactor_ls *a_list;
    int total = 0;

    if(!jen_get_groups(engine, CUE_BALL)){
        actor = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/cue_ball.png");
        actor->c_handler = dot_collider;
        actor->i_handler = dot_iterator;
        actor->m = 5;
        jen_add_jactor(engine, actor, CUE_BALL);
    }
    actor = jen_get_groups(engine, CUE_BALL)->actor;
    position_ball(actor, map, CUE_BALL, 1);


    if(!jen_get_groups(engine, EIGHT_BALL)){
        actor = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/black_ball.png");
        actor->c_handler = dot_collider;
        actor->i_handler = dot_iterator;
        actor->m = 5;
        jen_add_jactor(engine, actor, EIGHT_BALL);
    }
    actor = jen_get_groups(engine, EIGHT_BALL)->actor;
    position_ball(actor, map, EIGHT_BALL, 1);



    for(a_list = jen_get_groups(engine, YELLOW_BALL);
            a_list != NULL; a_list = a_list->next){
        total++;
        actor = a_list->actor;
        position_ball(actor, map, YELLOW_BALL, total);
    }


    while(total < 7){
        actor = jactor_create(BALL_DIAM, BALL_DIAM, map,
                "images/yellow_ball.png");
        actor->c_handler = dot_collider;
        actor->i_handler = dot_iterator;
        actor->m = 5;
        jen_add_jactor(engine, actor, YELLOW_BALL);
        total++;
        position_ball(actor, map, YELLOW_BALL, total);
    }

    total = 0;
    for(a_list = jen_get_groups(engine, RED_BALL);
            a_list != NULL; a_list = a_list->next){
        total++;
        actor = a_list->actor;
        position_ball(actor, map, RED_BALL, total);
    }

    while(total < 7){
        actor = jactor_create(BALL_DIAM, BALL_DIAM, map,
                "images/red_ball.png");
        actor->c_handler = dot_collider;
        actor->i_handler = dot_iterator;
        actor->m = 5;
        jen_add_jactor(engine, actor, RED_BALL);
        total++;
        position_ball(actor, map, RED_BALL, total);
    }

    return;
}


void cast_actors(jen *engine, jmap *map)
{
    jactor *dot;
    jactor *cue;
    jactor *actor;

    /* set up 'cue_ball' actor */
    actor = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/cue_ball.png");
    actor->c_handler = cue_baulk_collider;
    actor->i_handler = dot_iterator;
    position_ball(actor, map, CUE_BALL, 1);
    actor->m = 5;
    jen_add_jactor(engine, actor, CUE_BALL);

    /* set up 'black' ball */
    actor = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/black_ball.png");
    actor->c_handler = dot_collider;
    actor->i_handler = dot_iterator;
    position_ball(actor, map, EIGHT_BALL, 1);
    actor->m = 5;
    jen_add_jactor(engine, actor, EIGHT_BALL);

    /* set up 'yellow' balls */
    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/yellow_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, YELLOW_BALL, 1);
    dot->m = 5;
    jen_add_jactor(engine, dot, YELLOW_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/yellow_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, YELLOW_BALL, 2);
    dot->m = 5;
    jen_add_jactor(engine, dot, YELLOW_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/yellow_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, YELLOW_BALL, 3);
    dot->m = 5;
    jen_add_jactor(engine, dot, YELLOW_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/yellow_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, YELLOW_BALL, 4);
    dot->m = 5;
    jen_add_jactor(engine, dot, YELLOW_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/yellow_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, YELLOW_BALL, 5);
    dot->m = 5;
    jen_add_jactor(engine, dot, YELLOW_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/yellow_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, YELLOW_BALL, 6);
    dot->m = 5;
    jen_add_jactor(engine, dot, YELLOW_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/yellow_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, YELLOW_BALL, 7);
    dot->m = 5;
    jen_add_jactor(engine, dot, YELLOW_BALL);



    /* set up 'red' balls */
    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/red_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, RED_BALL, 1);
    dot->m = 5;
    jen_add_jactor(engine, dot, RED_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/red_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, RED_BALL, 2);
    dot->m = 5;
    jen_add_jactor(engine, dot, RED_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/red_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, RED_BALL, 3);
    dot->m = 5;
    jen_add_jactor(engine, dot, RED_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/red_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, RED_BALL, 4);
    dot->m = 5;
    jen_add_jactor(engine, dot, RED_BALL);

    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/red_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, RED_BALL, 5);
    dot->m = 5;
    jen_add_jactor(engine, dot, RED_BALL);
    
    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/red_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, RED_BALL, 6);
    dot->m = 5;
    jen_add_jactor(engine, dot, RED_BALL);
    
    dot = jactor_create(BALL_DIAM, BALL_DIAM, map, "images/red_ball.png");
    dot->c_handler = dot_collider;
    dot->i_handler = dot_iterator;
    position_ball(dot, map, RED_BALL, 7);
    dot->m = 5;
    jen_add_jactor(engine, dot, RED_BALL);


    /* Set up the cue */
    cue = jactor_create(25, 1000, map, "cue.png");
    cue->i_handler = cue_iterator;
    cue->v_x =0 / FPS;
    cue->v_y =0 / FPS;
    cue->r = CUE_ROT_RAD;
    cue->pr = cue->r;
    cue->theta = cue->ptheta = -90;
    cue->omega = 0;
    jen_add_jactor(engine, cue, CUE);

    return;
}

void make_sb(struct jen *engine, struct jmap *map)
{
    /* Set up some text */
    struct text_jactor *text;
    text = text_jactor_create(400, 50, map, "images/text_bg.png");
    text->core.px = text->core.x = 200;
    text->core.py = text->core.y = 25;

    text_jactor_set_text(text, "Player 1 (?) | Player 2 (?)");
    jen_add_jactor(engine, (struct jactor *)text, PLAYER1_TXT); 
    text->core.show_sprite = 1;

    return;
}

/* Swap the player who is the current player */
void change_player(struct bcr_en *engine)
{

    if(engine->core.current_player->uid == engine->player1->uid){
        engine->core.current_player = engine->player2;
        fprintf(stderr, "Player2 (%s) is now in control\n",
                (engine->core.game_state & OPEN) ? "???" : 
                 (engine->core.current_player->colour == RED_BALL ? 
                  "Red" : "Yellow"));
    }else{
        engine->core.current_player = engine->player1;
        fprintf(stderr, "Player1 (%s) is now in control\n",
                (engine->core.game_state & OPEN) ? "???" : 
                 (engine->core.current_player->colour == RED_BALL ?
                  "Red" : "Yellow"));

    }

    return;
}

#define other_player(engine, player) (engine->player1->uid == player->uid ? \
        engine->player2 : engine->player1)

int nominate_colour(struct bcr_en *engine, int types_potted){
    SDL_Event selection;
    struct jplayer *current_player = engine->core.current_player;

    fprintf(stderr, "Please nominate the colour you are going for\n");
    while(1){
        SDL_WaitEvent(&selection);
        if(selection.type == SDL_QUIT){
            return -1;
        }
        if(selection.type == SDL_KEYDOWN){
            if(selection.key.keysym.sym == SDLK_r){
                fprintf(stderr, "Red selected\n");
                current_player->colour = RED_BALL;
                other_player(engine, current_player)->colour = YELLOW_BALL;

                if(types_potted & RED_BALL_POTTED){
                    fprintf(stderr, "Game no longer open");
                    engine->core.game_state &= ~OPEN;
                    update_scoreboard(engine);
                }
                return RED_BALL;
            }else if(selection.key.keysym.sym == SDLK_y){
                fprintf(stderr, "Yellow selected\n");
                current_player->colour = YELLOW_BALL;
                other_player(engine, current_player)->colour = RED_BALL;

                if(types_potted & YELLOW_BALL_POTTED){
                    fprintf(stderr, "Game no longer open");
                    engine->core.game_state &= ~OPEN;
                    update_scoreboard(engine);
                }
                return YELLOW_BALL;
            }
        }
    }

    return -1;
}

int main(int argc, char **argv)
{
    int flags = SDL_OPENGL;
    SDL_Surface *screen, *buffer;
    jmap *bg_map, *scoreboard;
    int bpp = 0;
    int last_tick;
    int start_time, end_time;
    double frames_so_far;
    int rendered_frames = 0, logic_frames = 0;
    int carry_on = 1;
    struct bcr_en *engine;
    jactor *cue, *cue_ball;
    jactor_ls *ball_ls, *sb_ls;


    if(SDL_Init(SDL_INIT_VIDEO) == -1){
		fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
        return -1;
    }
    atexit(SDL_Quit);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	

    //set up screen
    screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
    if(!screen){
        fprintf(stderr, "Failed to open screen!\n");
        return 1;
    }


    /* openGL initialisation */
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDisable(GL_DEPTH);  /* This is a 2d program, no need for depth test */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, MAP_W * TILE_W,
            MAP_H * TILE_H, 0, 0.5, 0.0);
    glViewport(0, 0, MAP_W * TILE_W, MAP_H * TILE_H);



    buffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
				screen->w, screen->h,
				screen->format->BitsPerPixel,
				screen->format->Rmask,
				screen->format->Gmask,
				screen->format->Bmask,
				screen->format->Amask);

    /* set up the engine */
    /* First set up the generic jaunty engine */
    if(!(engine = (struct bcr_en *)jen_create())){
        fprintf(stderr, "Unable to allocate resources to engine\n");
        return 1;
    }
    /* Then realloc to provide room for extensions
     * included in struct bcr_en */
    if(!(engine = realloc(engine, sizeof(*engine)))){
        fprintf(stderr, "Unable to allocate resources to engine\n");
        return 1;
    }


    engine->core.game_state = 0;


    /* Create the players */
    engine->player1 = jplayer_create(RED_BALL);
    engine->player2 = jplayer_create(YELLOW_BALL);
    engine->core.current_player = engine->player1;
    engine->shots = 1;


    /* set up background map */
    if(!(bg_map = jmap_create(0, 0, MAP_W, MAP_H))){
        fprintf(stderr, "Error could not open jmap!\n");
        return 1;
    }
    if(jmap_load_tilepalette(bg_map, "snooker.png", TILE_W, TILE_H) == -1){
        fprintf(stderr, "Error could not load the tile palette!\n");
        return 1;
    }

    /* set up the scoreboard's map (it uses the same tile palette as the
     * background map) */
    if(!(scoreboard = jmap_create(0, SCREEN_H - 2 * TILE_H , SBOARD_W, SBOARD_H))){
        fprintf(stderr, "Error could not open scoreboard!\n");
    }
    if(jmap_load_tilepalette(scoreboard, "testmap.png", TILE_W, TILE_H) == -1){
        fprintf(stderr, "Error could not load the tile palette!\n");
        return 1;
    }

    load_level(bg_map, 1);
    load_level(scoreboard, 0);

    jmap_paint(bg_map);
    jmap_paint(scoreboard);

    cast_actors((struct jen *)engine, bg_map);
    make_sb((struct jen *)engine, scoreboard);
    cue = jen_get_groups((struct jen *)engine, CUE)->actor;
    cue_ball = jen_get_groups((struct jen *)engine, CUE_BALL)->actor;
    ball_ls = jen_get_groups((struct jen *)engine, BALLS);
    sb_ls = jen_get_groups((struct jen *)engine, PLAYER1_TXT);

    engine->core.game_state |= PLAYING_FROM_BAULK | BREAK_SHOT | OPEN;

    rendered_frames = 0;

    start_time = last_tick = SDL_GetTicks();
    
    /* Main game loop */
    while(carry_on){
        int elapsed_whole_frames;
        struct changed_states changed_states;
        int tick;
        double frames, dt;
        SDL_Event event;
        jactor_ls *actor_ls;
        //SDL_Delay(10);


        changed_states = jen_state_change_detect((struct jen *)engine);
        if(changed_states.on || changed_states.off){

            // Code executed every time the balls stop moving
            if(changed_states.off & BALLS_MOVING){

                if(engine->shots <= 0 || engine->core.game_state & FOUL_COMMITED){

                    if(engine->core.game_state & FOUL_COMMITED){
                        fprintf(stderr, "Foul committed so other player gets "
                                " two visits.\n");
                        engine->core.game_state &= ~FOUL_COMMITED;
                        engine->shots = 2;
                    }else{
                        engine->shots = 1;
                    }
                    change_player(engine);
                }

                fprintf(stderr, "Player to play: %d\n",
                        engine->core.current_player->uid + 1);
                engine->shots--;
            }




            if(changed_states.on & PLAYING_FROM_BAULK){
                fprintf(stderr, "Position the cue-ball within the "
                        "baulk area with the cursor keys.\n"
                        "Press enter when you are ready to shoot\n");
                cue_ball->c_handler = cue_baulk_collider;
            }else if(changed_states.off & BREAK_SHOT){
                /* Check for valid break here */
                unsigned int break_number;
                break_number = valid_break((struct jen *)engine);
                if(break_number == EIGHT_BALL_POTTED){
                    fprintf(stderr, "rerack and same player breaks\n");
                    rerack((struct jen *)engine, bg_map);

                }else if(!(break_number & (RED_BALL_POTTED |
                            YELLOW_BALL_POTTED | FOUR_CUSHION_HITS))){
                    fprintf(stderr, "Unfair break: rerack, two visits\n");
                    rerack((struct jen *)engine, bg_map);
                    change_player(engine);
                    engine->shots = 2;
                    engine->core.game_state = PLAYING_FROM_BAULK | BREAK_SHOT | OPEN;

                }else if(break_number == CUE_BALL_POTTED){
                    fprintf(stderr, " Cue ball potted: turn goes to opponent\n");
                    change_player(engine);
                    engine->core.game_state = PLAYING_FROM_BAULK;
                }else{
                    fprintf(stderr, "ask player to nominate colour "
                            ", if this is a colour that has been potted "
                            "then they are on that colour, else they must "
                            "pot that colour on the next shot or the game is "
                            "still open\n");
                    if(nominate_colour(engine, break_number & (RED_BALL_POTTED |
                                YELLOW_BALL_POTTED)) == -1){
                        carry_on = 0;
                    }
                    

                }
            }else if(changed_states.off & BALLS_MOVING){

                if(engine->core.game_state & OPEN){
                    fprintf(stderr, "Game is still open\n");
                    fprintf(stderr, "Please nominate a colour to shoot at\n");
                    if(nominate_colour(engine, 0) == -1){
                        carry_on = 0;
                    }
                }

            }
        }

              
        /* Event loop */
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                carry_on = 0;
            }else if(engine->core.game_state & USER_INPUT){
                if(event.type == SDL_KEYDOWN){
                    if(event.key.keysym.sym == SDLK_LEFT){
                        cue->omega = -40 / FPS;
                        engine->core.game_state &= ~USER_INPUT;
                        engine->core.game_state |= CUE_POSITIONING;
                    }else if(event.key.keysym.sym == SDLK_RIGHT){
                        cue->omega = 40 / FPS;
                        engine->core.game_state &= ~USER_INPUT;
                        engine->core.game_state |= CUE_POSITIONING;
                    }else if(event.key.keysym.sym == SDLK_DOWN){
                        cue->v_r = 40 / FPS;
                        engine->core.game_state &= ~USER_INPUT;
                        engine->core.game_state |= CUE_CHARGING;
                    }
                }
            }else if(engine->core.game_state & CUE_POSITIONING){
                if(event.type == SDL_KEYUP){
                    if(event.key.keysym.sym == SDLK_LEFT
                            && cue->omega < 0){
                        cue->omega = 0;
                        jactor_iterate(cue, (struct jen *)engine);
                        engine->core.game_state &= ~CUE_POSITIONING;
                        engine->core.game_state |= USER_INPUT;
                    }else if(event.key.keysym.sym == SDLK_RIGHT
                            && cue->omega > 0){
                        cue->omega = 0;
                        jactor_iterate(cue, (struct jen *)engine);
                        engine->core.game_state &= ~CUE_POSITIONING;
                        engine->core.game_state |= USER_INPUT;
                    }
                }
            }else if(engine->core.game_state & CUE_CHARGING){
                if(event.type == SDL_KEYUP){
                    if(event.key.keysym.sym == SDLK_DOWN){
                        if((40 * (cue->r - CUE_ROT_RAD)) / FPS < MAX_SPEED){
                            cue->v_r = -(40 * (cue->r - CUE_ROT_RAD)) / FPS;
                        }else
                            cue->v_r = -MAX_SPEED;
                        engine->core.game_state &= ~CUE_CHARGING;
                        engine->core.game_state |= CUE_DISCHARGING;
                    }
                }
            }else if(engine->core.game_state & PLAYING_FROM_BAULK){
                if(event.type == SDL_KEYDOWN){
                    if(event.key.keysym.sym == SDLK_UP){
                        cue_ball->v_y = -20 / FPS;
                    }else if(event.key.keysym.sym == SDLK_RIGHT){
                        cue_ball->v_x = 20 / FPS;
                    }else if(event.key.keysym.sym == SDLK_DOWN){
                        cue_ball->v_y = 20 / FPS;
                    }else if(event.key.keysym.sym == SDLK_LEFT){
                        cue_ball->v_x = -20 / FPS;
                    }else if(event.key.keysym.sym == SDLK_RETURN){
                        engine->core.game_state &= ~PLAYING_FROM_BAULK;
                        set_cue_motion((struct jen *)engine);
                        cue_ball->c_handler = dot_collider;
                    }
                }else if(event.type == SDL_KEYUP){
                        cue_ball->v_y = 0;
                        cue_ball->v_x = 0;
                }
            }
        }


        glClear(GL_COLOR_BUFFER_BIT);
            
                //exit(0);


        tick = SDL_GetTicks();
        dt = (tick - last_tick) * 0.001;
        frames = dt * FPS;
        elapsed_whole_frames = floor(frames_so_far + frames) - 
            floor(frames_so_far);

        
        
        while(elapsed_whole_frames--){
            logic_frames++;
            if(engine->core.game_state & BALLS_MOVING){
                
                jcoll c_info;
                int i = 0;

            

                engine->core.game_state &= ~BALLS_MOVING;
                for(actor_ls = ball_ls;
                        actor_ls != NULL; actor_ls = actor_ls->next){ 
                    i++;
                    jactor *a = actor_ls->actor;
                    jactor_ls *p;
                    
                    jactor_iterate(a, (struct jen *)engine);
                    if(jmap_collision_detect(bg_map, a, &c_info, 3)){
                        /* This code is executed when a ball goes into
                         * a hole */
                        if(a->c_handler(a, &c_info, (struct jen *)engine) == POTTED){
                            if(engine->core.game_state & CUE_BALL_POTTED){
                                ball_ls = jen_get_groups((struct jen *)engine, OBJ_BALLS); 
                            }else{
                                actor_ls = jen_get_groups((struct jen *)engine, (CUE_BALL | BALLS));
                            }
                            continue;
                        }
                    }
                    for(p = actor_ls->next; p != NULL; p = p->next){
                        if(jactor_collision_detect(a, p->actor, &c_info)){
                            a->c_handler(a, &c_info, (struct jen *)engine);
                        }
                    }

                    if(a->v_x != 0.0 || a->v_y != 0.0){
                        engine->core.game_state |= BALLS_MOVING;
                    }
                } 

                /* Code that should be executed when the balls are no 
                 * longer moving */
                if(!(engine->core.game_state & BALLS_MOVING)){

                    engine->core.game_state &= ~BREAK_SHOT;

                    if(engine->core.game_state & CUE_BALL_POTTED){
                        /* Put the cue ball back in its place */
                        jactor *cb;

                        cb = jen_get_groups((struct jen *)engine, CUE_BALL)->actor;
                        cb->px = 100;
                        cb->py = 100;
                        cb->x = 100;
                        cb->y = 100;
                        cb->v_x = 0;
                        cb->v_y = 0;

                        ball_ls = jen_get_groups((struct jen *)engine, BALLS);
                        engine->core.game_state &= ~CUE_BALL_POTTED;
                        set_cue_motion((struct jen *)engine);
                        

                    }else{
                        if((engine->core.game_state & CB_CLEAN)){
                            engine->core.game_state |= FOUL_COMMITED;
                        }
                        set_cue_motion((struct jen *)engine);
                    }
                }

            }else if(engine->core.game_state & PLAYING_FROM_BAULK){
                jcoll c_info;
                if(jmap_collision_detect(bg_map, cue_ball, &c_info, 7)){
                    cue_ball->c_handler(cue_ball, &c_info, (struct jen *)engine);
                }
                jactor_iterate(cue_ball, (struct jen *)engine);
            }else if(engine->core.game_state & CUE_MOVING){
                engine->core.game_state &= ~CUE_MOVING;
     
                engine->core.game_state |= USER_INPUT;
            }else if(engine->core.game_state & USER_INPUT){
                /* Put code to respond to user input here */
        
            }else if(engine->core.game_state & (CUE_POSITIONING | CUE_CHARGING
                        | CUE_DISCHARGING)){
                jactor_iterate(cue, (struct jen *)engine);
            }
        }
        

        
        frames_so_far += frames - elapsed_whole_frames;

        

        /* overwrite with the background what was written last time */
        jmap_paint(bg_map);
        jmap_paint(scoreboard);
        
        /* draw all the balls onto the screen */
        for(actor_ls = ball_ls;
         actor_ls != NULL; actor_ls = actor_ls->next){
            jactor_paint(actor_ls->actor, frames_so_far);
        }

        /* draw the scoreboard elements */
        for(actor_ls = sb_ls;
                actor_ls != NULL; actor_ls = actor_ls->next){
            actor_ls->actor->paint(actor_ls->actor, 0);
        }

        /* Show the cue if the game state is such that the cue should 
         * be shown */
        if(engine->core.game_state & (CUE_MOVING | USER_INPUT | CUE_CHARGING |
                    CUE_POSITIONING | CUE_DISCHARGING)){
            jactor_paint(cue, frames_so_far);
        }


        rendered_frames++;
        last_tick = tick;
        
        SDL_GL_SwapBuffers();


    }
 
    end_time = SDL_GetTicks();

    printf("\nLogic frames: %.2f\n", logic_frames * 1000.0 / (end_time - 
               start_time)); 

    printf("\nRendered frames: %.2f\n", rendered_frames * 1000.0 / (end_time - 
               start_time));

    /* free resources */
    jmap_free(bg_map);
    bcr_en_free(engine);

    return 0;
}
