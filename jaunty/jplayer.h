/* Struct for storing details about the players */

struct jplayer{
    /* Record of the score of the player */
    int score;
    /* The colour of the team the player is on */
    unsigned int colour;
    /* Unique identifying number of player */
    unsigned int uid;
};

/* Frees the resources allocated to the player */
void jplayer_free(struct jplayer *player);

/* Creates a player of the colour 'colour' */
struct jplayer *jplayer_create(unsigned int colour);
