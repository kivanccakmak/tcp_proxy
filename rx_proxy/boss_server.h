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
#include "time.h"

#include "forward.h"
#include "../network/network.h"
#include "link_receptor.h"
#include "pqueue.h"

#define CLOSE_CONN -1



/**
 * @brief buffers sequentially ordered 
 * data from receptor thread, 
 * passes towards agnostic end destination
 */
typedef struct fqueue{
    /**
     * @brief threads adds pointers 
     * of sequentially ordered raw 
     * data to this pointer array
     */
    char **buffer;

    /**
     * @brief number of bytes in **buf
     */
    int byte_count;

    /**
     * @brief capacity of **buf
     */
    int byte_capacity;

    /**
     * @brief to send data to
     * end-destination
     */
    int sockfd;
    
    /**
     * @brief SLEEP or SEND
     */
    int state;
    
    /**
     * @brief number of sent packets
     */
    int sent;

} fqueue_t;

/**
 * @brief receiver threads
 * would access pool to put
 * data inside priority queue.
 * Queue would access pool to
 * get forward data from
 * priority queue.
 */
typedef struct pool{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pqueue_t* pq;
    int avail_min_seq;
    int sent_min_seq;
}pool_t;

/**
 * @brief call-back arguments
 * for link receptor thread.
 */
typedef struct cb_rx_args{
    /**
     * @brief socket descriptor of new 
     * TCP connection in between link receptor 
     * and transmitter side of proxy
     */
    int sockfd;
    int poll_timeout;
    pool_t *pl;
} rx_args_t;

/**
 * @brief queue call back arguments.
 */
typedef struct queue_args{
    pool_t *pl;
    char *dest_port;
    char *dest_ip;
} queue_args_t;

void server_listen(char* server_port, pool_t* pool);

pool_t* pool_init(); 

int rcv_sock_init(char *server_port);

#endif
