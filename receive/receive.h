#ifndef RECEIVE_H
#define RECEIVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <stdbool.h>
#include <poll.h>
#include <libconfig.h>
#include <getopt.h>
#include "../commons/network/network.h"

#define PATH_MAX 2048
#define PORT_MAX_CHAR 50

struct arg_configer{
    char log_file[PATH_MAX];
    char output[PATH_MAX];
    char port[PORT_MAX_CHAR];
};

struct option long_options[] = {
    {"port", required_argument, NULL, 'A'},
    {"output", required_argument, NULL, 'B'},
    {"log_file", required_argument, NULL, 'C'}
};




#endif
