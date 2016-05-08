/*
 * Copyright (C) 2016  Kıvanç Çakmak <kivanccakmak@gmail.com>
 * Author: Kıvanç Çakmak <kivanccakmak@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


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
