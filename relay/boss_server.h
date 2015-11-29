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

#include "queue.h"
#include "reorder.h"
#include "network.h"
#include "link_receptor.h"

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

    struct packet_pool *pool;

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
