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


#include "argv_read.h"

static void example_usage();

#ifdef ARGV_READ
int main(int argc, char *argv[])
{
    int i = 0, ret;
    char *key1, *key2;

    struct option long_options[] = {
        {"key1", required_argument, NULL, 'A'},
        {"key2", required_argument, NULL, 'B'},
    };

    if (argc == 3) {
        arg_val_t **argv_vals = \
            (arg_val_t **) malloc(sizeof(arg_val_t*) * (argc - 1));

        for (i = 0; i < argc - 1; i++)
            argv_vals[i] = (arg_val_t *) malloc(sizeof(arg_val_t));

        ret = argv_reader(argv_vals,
                long_options, argv, argc);

        if (!ret) {
            key1 = get_argv((char *) "key1", argv_vals, argc-1);
            key2 = get_argv((char *) "key2", argv_vals, argc-1);

            if (key1 != NULL)
                printf("key1: %s\n", key1);
            else
                printf("key1 is empty\n");

            if (key2 != NULL)
                printf("key2: %s\n", key2);
            else
                printf("key2 is empty\n");

        }
    } else {
        example_usage();
    }

    return 0;
}
#endif

static void example_usage() 
{
    printf("== example usage == \n");
    printf("./argv_read --key1=value1 --key2=value2\n"); 
}


/**
 * @brief
 *
 * @param argnum
 * @param
 *
 * @return
 */
arg_val_t** init_arg_vals(
                          int           argnum,
                          struct option *long_options
                         )
{
    int i = 0;
    arg_val_t **arg_vals =\
            (arg_val_t **) malloc(sizeof(arg_val_t*) * argnum);

    for (i = 0; i < argnum; i++) {
        arg_vals[i] = (arg_val_t *) malloc(sizeof(arg_val_t));
        arg_vals[i]->key = (const char *) long_options[i].name;
    }

    return arg_vals;
}


/**
 * @brief
 *
 * @param argv_vals
 * @param long_options
 * @param argv
 * @param argc 
 *
 * @return 0 success, -1 error 
 */
int argv_reader(
                arg_val_t     **argv_vals, 
                struct option *long_options, 
                char          **argv,
                int           argc
               ) 
{
    int c = 0, option_index = 0;
    int i = 0, count = 0;
    for (;;) {
        c = getopt_long(argc, argv, "", long_options,
                &option_index);

        for (i = 0; i < argc; i++) {
            if (long_options[i].val == c){ 
                argv_vals[count]->key = (char *) long_options[i].name;
                argv_vals[count]->val = (char *) optarg;
                count++;
            }
        }

        if (c == -1)
            break;

        if (c == '?' || c == ':')
            return -1;
    }
    return 0;
} 


/**
 * @brief
 *
 * @param arg_vals
 * @param setting
 * @param
 *
 * @return
 */
int config_reader(
                  arg_val_t         **arg_vals,
                  config_setting_t  *setting,
                  int               argc
                 )
{
    int i = 0, ret;

    if (setting == NULL)
        return -1;

    for (i = 0; i < argc; i++) {
        ret = config_setting_lookup_string(setting,
                (const char*) arg_vals[i]->key, &arg_vals[i]->val);

        if (!ret)
            return -1;
    }

    return 0;
}

/**
 * @brief 
 *
 * @param ch[]
 * @param argv_vals
 * @param argc 
 *
 * @return 
 */
char* get_argv(
               char ch[], 
               arg_val_t **argv_vals, 
               int argc
              ) 
{
    int i = 0;
    for (i = 0; i < argc; i++) {
        if (!strcmp(argv_vals[i]->key, (char*) ch))
            return (char*) argv_vals[i]->val;
    }
    return NULL;
}

void print_argvs(
                 arg_val_t **arg_vals,
                 int argnum
                )
{
    int i = 0;

    for (i = 0; i < argnum; i++) {
        printf("arg_vals[%d]->key: %s\n", i, arg_vals[i]->key);
        printf("arg_vals[%d]->val: %s\n", i, arg_vals[i]->val);
    }

}
