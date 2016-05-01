#ifndef ARGV_READ_H

#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <time.h>

#define OSMAN_CONF "osman.conf"

/**
 * @brief joint struct to use 
 * as output of both libconfig and
 * getopt libraries
 *
 */
typedef struct argv_val{
    char *key;
    char *val;
    struct arg_val *next;
}arg_val_t;


int argv_reader(arg_val_t     **argv_vals, 
                struct option *long_options, 
                char          **argv,
                int           argc
                ); 

char* get_argv(
               char ch[], 
               arg_val_t **argv_vals, 
               int argc
               );

#endif
