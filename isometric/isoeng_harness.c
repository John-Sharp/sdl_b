#include "isoeng.h"
#include <stdio.h>

void printgrp(struct isogrp *grp)
{
    struct isols *lp;

    printf("\nMembers of group: %d\n", grp->groupnum);

    for(lp = grp->ls; lp != NULL; lp = lp->next){
        printf("Actor %d\n", lp->actor->uid);
    }
    printf("\n");
}

int main(void)
{
    struct isoeng *engine;
    struct isoactor *actor[5];
    struct isogrp *grp;

    int testgrp = 1, testgrp2 = 1<<1;

    engine = isoeng_create();
    actor[0] = isoeng_new_actor(engine, testgrp);
    actor[1] = isoeng_new_actor(engine, testgrp);
    actor[2] = isoeng_new_actor(engine, testgrp2);
    actor[3] = isoeng_new_actor(engine, testgrp | testgrp2);
    actor[4] = isoeng_new_actor(engine, 1 << 4);


    grp = isoeng_get_group(engine, testgrp);

    grp = isoeng_get_group(engine, testgrp | 1 << 4);

    grp = isoeng_get_group(engine, testgrp);

    isoeng_actor_drop(engine, actor[0], testgrp);
    isoeng_actor_drop(engine, actor[2], testgrp);
    isoeng_actor_add(engine, actor[2], testgrp);

    printgrp(grp);

    isoeng_del_actor(engine, actor[0]);
//    isoeng_del_actor(engine, actor[1]);
    isoeng_del_actor(engine, actor[4]);

    actor[4] = isoeng_new_actor(engine, testgrp);
    actor[4] = isoeng_new_actor(engine, testgrp);
//    isoeng_del_actor(engine, actor[3]);

    printgrp(grp);


    isoeng_free(engine);

    return 0;
}

