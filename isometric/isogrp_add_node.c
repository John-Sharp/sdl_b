static int isogrp_add_node(struct isogrp *grp, int grp_num, struct isoactor *actor)
{
    struct isogrp *next;
    struct isogrp *newnode;

    if(grp == NULL){
        next = NULL;
        newnode = grp;
    }else{
        next = grp->next;
        newnode = grp->next;
    }

    /* Allocate group stub */
    if((grp->next = malloc(sizeof(*(grp)))) == NULL){
        fprintf(stderr, "Error allocating node for group %d\n", grp_num);
        exit(1);
    }

    grp->next->next = next;
    grp->next = grp_num;

    /* Add list to group stub */
    if((grp->next->list = malloc(sizeof(*(grp->next->list)))) == NULL){
        fprintf(stderr, "Error allocating list for group %d\n", grp_num);
        exit(1);
    }

    grp->next->list->next = NULL;

    /*  Add actor to list */
    grp->next->list->actor = actor;

#ifdef DEBUG_MODE
    fprintf(stderr, "Put actor in group: %d (new stub)\n", next_group);
#endif

    return 1;
}



int isols_add_node(struct isols *ls, struct isoactor *actor)
{
    struct isols *next;
    next = ls->next;

    if((ls->next = malloc(sizeof(*(lq->next)))) == NULL){
                    fprintf(stderr, "Error allocating list node\n");
                    exit(1);
                }

    lq->next->next = next;
    lq->next->actor = actor;
   return 1;
} 

