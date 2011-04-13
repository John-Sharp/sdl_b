#include "jplayer.h"
#include <stdlib.h>

unsigned int max_uid = 0;


void jplayer_free(struct jplayer *player)
{
    free(player);

    return;
}

struct jplayer *jplayer_create(unsigned int colour)
{
    struct jplayer *player;

    if(!(player = malloc(sizeof(*player)))){
        return NULL;
    }

    player->colour = colour;
    player->score = 0;
    player->uid = max_uid;
    max_uid++;

    return player;
}

