#ifndef ARGV_READ_H

#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define OSMAN_CONF "osman.conf"

/**
 * @brief joint struct to use 
 * as output of both libconfig and
 * getopt libraries
 *
 */
typedef struct argv_val{
    const char *key;
    const char *val;
    struct arg_val *next;
}arg_val_t;

arg_val_t** init_arg_vals(
                          int           argnum,
                          struct option *long_options
                         );

int argv_reader(
                arg_val_t     **argv_vals,
                struct option *long_options, 
                char          **argv,
                int           argc
               );

char* get_argv(
               char ch[], 
               arg_val_t **argv_vals, 
               int argc
              );

int config_reader(
                  arg_val_t        **arg_vals,
                  config_setting_t *setting,
                  int              argc
                 );

void print_argvs(
                 arg_val_t **arg_vals,
                 int argnum
                );
#endif
