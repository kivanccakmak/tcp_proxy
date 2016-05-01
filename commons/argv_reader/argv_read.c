#include "argv_read.h"

#ifdef ARGV_READ
int main(int argc, char *argv[])
{
    int i = 0, ret;

    struct option long_options[] = {
        {"port", required_argument, NULL, 'A'},
        {"output", required_argument, NULL, 'B'},
        {"log_file", required_argument, NULL, 'C'}
    };

    if (argc == 4) {
        arg_val_t **argv_vals = \
            (arg_val_t **) malloc(sizeof(arg_val_t*) * (argc - 1));

        for (i = 0; i < argc - 1; i++)
            argv_vals[i] = (arg_val_t *) malloc(sizeof(arg_val_t));

        ret = argv_reader(argv_vals,
                long_options, argv, argc);
        printf("ret: %d\n", ret);
        if (!ret) {
            char *port;
            port = get_argv((char *) "port", argv_vals, argc-1);
            printf("port: %s\n", port);
        }
    }

    return 0;
}
#endif

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
            return argv_vals[i]->val;
    }
    return NULL;
}
