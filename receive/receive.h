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
#include "../commons/logger/logger.h"
#include "../commons/argv_reader/argv_read.h"

#define PATH_MAX 2048
#define PORT_MAX_CHAR 50

#define RECEIVE_ARGV_NUM 4

#define RECV_LOG "recv.log"

#endif
