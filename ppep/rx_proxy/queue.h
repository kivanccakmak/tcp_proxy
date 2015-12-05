#ifndef QUEUE_H
#define QUEUE_H

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

#define INIT_QUEUE_SIZE sizeof(char *) * 1000

#define NO_SEQ -1

/**
 * @brief 
 * buffers sequentially ordered data from 
 * receptor thread, passes towards agnostic
 * end destination
 */
typedef struct queue{
    /**
     * @brief 
     * threads adds pointers of sequentially
     * ordered raw data to this pointer array
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

    int index;

    /**
     * @brief seq_number of packet
     * lastly passed to 
     */
    int last_seq;

    /**
     * @brief 
     * file descriptor with end-destination
     */
    int sockfd;

} queue_t;


void *queue_wait(void *args);
queue_t* queue_init(char *dest_ip, char *dest_port);

#include "boss_server.h"
#include "network.h"

#endif
