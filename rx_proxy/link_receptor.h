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

#include "boss_server.h"
#include "libpqueue/pqueue.h"
#include "../commons/network/network.h"

void *rx_chain(void *args);

#endif