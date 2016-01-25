#ifndef QUEUE_FUNCS_H
#define QUEUE_FUNCS_H

#include <stdio.h>
#include "pqueue.h"
#include "../network/network.h"

/**
 * @brief node in queue
 */
typedef struct node_t
{
    pqueue_pri_t pri;
    unsigned char* raw_packet;
    size_t pos;
} node_t;

/**
 * @brief compares priority  
 * of queue elements
 *
 * @param[in] next
 * @param[in] curr
 *
 * @return bool 
 */
int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr);

/**
 * @brief set priority 
 * of element in queue
 *
 * @param[out] a
 * @param[in] pri
 */
void set_pri(void *a, pqueue_pri_t pri);

/**
 * @brief get priority of 
 * element in queue
 *
 * @param[in] a
 *
 * @return int 
 */
pqueue_pri_t get_pri(void *a);

/**
 * @brief get position of
 * element in queue
 *
 * @param[in] a
 *
 * @return int 
 */
size_t get_pos(void *a);

/**
 * @brief set position 
 * of element in queue
 *
 * @param[out] a
 * @param[in] pos
 */
void set_pos(void *a, size_t pos);

#endif
