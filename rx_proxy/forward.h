#ifndef FORWARD_H
#define FORWARD_H

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
#include <stdbool.h>
#include "time.h"

#include "../network/network.h"
#include "pqueue.h"
#include "boss_server.h"

#define INIT_QUEUE_SIZE sizeof(char*) * 1000

#define SLEEP 0
#define SEND 1

#define WAIT_TIME 2

void *wait2forward(void *args);

#endif
