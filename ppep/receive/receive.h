#ifndef RECEIVE_H
#define RECEIVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include "../network/network.h"

void get_packets(char *port, FILE *fp); 

#endif
