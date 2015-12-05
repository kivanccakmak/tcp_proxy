#ifndef LINK_RECEPTOR_H
#define LINK_RECEPTOR_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>

#include "queue.h"
#include "boss_server.h"
#include "network.h"
#include "reorder.h"

void *rx_chain(void *args);

void push2pool(char *raw_packet, 
        struct packet_pool* pool);
#endif
