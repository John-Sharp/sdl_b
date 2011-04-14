#include "isoeng.h"
#include <stdio.h>

void printgrp(struct isogrp *grp)
{
    struct isols *lp;

    printf("Members of group: %d\n", grp->grp_num);

    for(lp = grp->list; lp != NULL; lp = lp->next){
        printf("Actor %d\n", lp->actor->uid);
    }
    printf("\n");
}

int main(void)
{
    struct isoeng *engine;
    struct isoactor *actor, *actor2;
    struct isogrp *grp;

    int testgrp = 1, testgrp2 = 1<<1;

    engine = isoeng_create();
    actor = isoeng_new_actor(engine, testgrp);
    actor = isoeng_new_actor(engine, testgrp);
    actor = isoeng_new_actor(engine, testgrp2);
    actor2 = isoeng_new_actor(engine, testgrp | testgrp2);
    actor = isoeng_new_actor(engine, 1 << 4);


    grp = isoeng_get_group(engine, testgrp);

    grp = isoeng_get_group(engine, testgrp | 1 << 4);
    grp = isoeng_get_group(engine, testgrp);




    printgrp(grp);

    isoeng_del_actor(engine, actor);
    printgrp(grp);


    isoeng_free(engine);

    return 0;
}

