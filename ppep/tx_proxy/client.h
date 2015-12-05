#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

#include "file_read.h"
#include "network.h"


/**
 * @brief 
 *
 * Call-back arguments of transmitter
 * thread.
 *
 */
typedef struct cb_tx_args{
    int sockfd;
    int seq_num;
    char buff[BLOCKSIZE];
} cb_tx_args_t;

#endif
