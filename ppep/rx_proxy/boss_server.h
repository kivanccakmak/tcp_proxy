#ifndef BOSS_SERVER_H
#define BOSS_SERVER_H

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>

#include "reorder.h"
#include "../network/network.h"
#include "link_receptor.h"
#include "pqueue.h"
#include "queue_funcs.h"
#include "forward.h"

/**
 * @brief receiver threads
 * would access pool to put
 * data inside priority queue.
 * Queue would access pool to
 * get forward data from
 * priority queue
 */
typedef struct pool{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pqueue_t* pq;
}pool_t;

/**
 * @brief call-back arguments
 * for link receptor thread.
 */
typedef struct cb_rx_args{

    /**
     * @brief socket descriptor of TCP connection
     * in between link receptor and transmitter
     * side of proxy
     */
    int sockfd;
    int poll_timeout;
    pool_t *pl;
    queue_t *queue;

} cb_rx_args_t;


/**
 * @brief call-back arguments
 * for reorder thread.
 */
typedef struct cb_reord_args{
    queue_t *queue;
    struct packet_pool *pool;
} cb_reord_args_t;

#define CLOSE_CONN -1

#endif
