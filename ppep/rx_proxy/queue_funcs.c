#include <stdio.h>
#include <stdlib.h>

#include "pqueue.h"
#include "queue_funcs.h"

int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr)
{
    return (next < curr);
}

pqueue_pri_t get_pri(void *a)
{
    return ((node_t *) a)->pri;
}

size_t get_pos(void *a) 
{
    return ((node_t *) a)->pos;
}

void set_pos(void *a, size_t pos)
{
    ((node_t *) a)->pos = pos;
}

void set_pri(void *a, pqueue_pri_t pri)
{
    ((node_t *) a)->pri = pri;
}
