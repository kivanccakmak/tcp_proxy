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
#include "../network/network.h"


/**
 * @brief receiver chain thread
 *
 * @param args
 *
 * @return 
 */
void *rx_chain(void *args);

#endif
