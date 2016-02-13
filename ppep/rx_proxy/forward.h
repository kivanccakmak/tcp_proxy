#ifndef FORWARD_H
#define FORWARD_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "../network/network.h"
#include "pqueue.h"
#include "boss_server.h"

#define INIT_QUEUE_SIZE sizeof(char*) * 1000

#define SLEEP 0
#define SEND 1

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

    pthread_cond_t cond;
    pthread_mutex_t lock;

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
 * @brief forward queue initialization to
 * stream proxied data through agnostic and
 * destination. used by boss_server
 * module
 *
 * @param sockfd
 *
 * @return 
 */
fqueue_t* fqueue_init(int sockfd);

/**
 * @brief enabled by boss_server
 * module, queue module waits
 * nudgeing from receiver threads.
 *
 * @param args
 *
 * @return 
 */
void *wait2forward(void *args);

#endif
