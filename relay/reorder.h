#ifndef REORDER_H
#define REORDER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "network.h"
#include "queue.h"

#define NODE_SIZE sizeof(list_node_t *)
#define INIT_POOL_SIZE 1000
#define INIT_BUF_SIZE 1000

#define NO_SEQUENCE -1

/**
 * @brief 
 * sequentially non-ordered
 * packets would diffuse in packet_pool.
 *
 */
struct packet_pool {

    /**
     * @brief 
     * sequence number of packet that
     * lastly passed to queue
     */
    int queue_seq;

    /**
     * @brief 
     * capacity of packet pool in terms
     * of packet counter
     */
    unsigned int capacity;

    /**
     * @brief 
     * number of packets in pool
     */
    unsigned int count;

    /**
     * @brief 
     * link_receptor would pass packet pointers
     * to sequential nodes pointer array
     */
    char **sequential_nodes;

    pthread_mutex_t lock;
    pthread_cond_t cond;

};

//==============
// FUNCTIONS
//==============

struct packet_pool* packet_pool_init();

void *nudge_queue(void *args);

#endif
