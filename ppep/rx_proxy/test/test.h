#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include "../../receive/receive.h"
#include "../boss_server.h"

#define MAX_PACKET 50

struct rx_params{
    FILE *fp;
    char *port;
};

struct pxy_params{
    int recv_sock;
    pool_t* pl;
};

struct stream_params{
    char *rx_pxy_ip;
    char *rx_pxy_port;
};

#endif
