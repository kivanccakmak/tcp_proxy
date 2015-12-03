#ifndef TEST_QUEUE_H
#define TEST_QUEUE_H

#define MAX_PACKET 5 

/**
 * @brief msec, reorder test
 * runtime
 */
#define RUNTIME 5000

#define RECEIVER_OUT "output.txt"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <poll.h>
#include <math.h>
#include <time.h>

#include "../network.h"
#include "../queue.h"
#include "../boss_server.h"
#include "../reorder.h"
#include "../link_receptor.h"


/**
 * @brief call-back arguments
 * of debug receiver thread
 */
struct debug_receiver_args{
    char *dest_port;

    /**
     * @brief writes on file
     * then, test code would 
     * check whether output
     * matches with array
     */
    char *filename;
};

#endif

