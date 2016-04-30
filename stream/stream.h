#ifndef STREAM_H
#define STREAM_H

#include <stdlib.h>
#include <stdio.h>
#include <libconfig.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#include "file_read/file_read.h"
#include "../network/network.h"

#define PATH_MAX 2048
#define IP_CHAR_MAX 512 
#define PORT_MAX_CHAR 50

struct arg_configer{
    char ip_addr[IP_CHAR_MAX];
    char port[PORT_MAX_CHAR];
    char fname[PATH_MAX];
};

struct option long_options[] = {
    {"ip_addr", required_argument, NULL, 'A'},
    {"port", required_argument, NULL, 'B'},
    {"fname", required_argument, NULL, 'C'}
};

#endif
