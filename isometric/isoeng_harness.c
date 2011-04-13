#include "isoeng.h"
#include <stdio.h>

void printlist(struct isols *list)
{
    struct isols *lp;
    printf("trying to print list\n");

    for(lp = list; lp != NULL; lp = lp->next){
        printf("Actor %d\n", lp->actor->uid);
    }
    printf("\n");
}

int main(void)
{
    struct isoeng *engine;
    struct isoactor *actor;
    struct isols *list;
    int testgrp = 1, testgrp2 = 1<<1;

    engine = isoeng_create();
    actor = isoeng_new_actor(engine, testgrp);
    actor = isoeng_new_actor(engine, testgrp);
    actor = isoeng_new_actor(engine, testgrp2);
    actor = isoeng_new_actor(engine, testgrp | testgrp2);
    actor = isoeng_new_actor(engine, 1 << 4);

    list = isoeng_get_group(engine, testgrp | testgrp2);
    list = isoeng_get_group(engine, testgrp | 1 << 4);
    printlist(list);

    isoeng_free(engine);

    return 0;
}

