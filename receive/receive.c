#include "receive.h"

static void recv_loop(
                      struct pollfd *pfd, 
                      int           sockfd, 
                      FILE          *fp, 
                      FILE          *res_fp
                     );

static void get_packets(
                        char *port, 
                        FILE *fp, 
                        FILE *res_fp
                       );

static int sock_init(char *port);

static const int num_options = 3;

static void eval_config_item(
                             char const          *token,
                             char const          *value, 
                             struct arg_configer *arg_conf
                            ); 

static FILE *log_fp;

#ifdef RECV
int main(int argc, char **argv) 
{
    const char *port, *output, *log_file;
    FILE *fp, *res_fp;
    int i = 0, ret;

    log_fp = fopen(RECV_LOG, "w");
    if (log_fp == NULL) {
        perror("logfp: ");
        exit(1);
    }
    
    #ifdef ARGV_ENABLE
    if (argc == 4) {
        struct option long_options[] = {
            {"port", required_argument, NULL, 'A'},
            {"output", required_argument, NULL, 'B'},
            {"log_file", required_argument, NULL, 'C'}
        };

        arg_val_t **argv_vals = \
            (arg_val_t **) malloc(sizeof(arg_val_t*) * (argc - 1));

        for (i = 0; i < argc - 1; i++)
            argv_vals[i] = (arg_val_t *) malloc(sizeof(arg_val_t));

        ret = argv_reader(argv_vals,
                long_options, argv, argc);

        port = get_argv((char *) "port", argv_vals, argc-1);
        LOG_ASSERT(log_fp, LL_ERROR, port!=NULL);

        output = get_argv((char *) "output", argv_vals, argc-1);
        LOG_ASSERT(log_fp, LL_ERROR, output!=NULL);

        log_file = get_argv((char *) "log_file", argv_vals, argc-1);
        LOG_ASSERT(log_fp, LL_ERROR, log_file!=NULL);

    }
    #endif

    #ifdef CONF_ENABLE
    if (argc == 1) {
        config_t cfg;
        config_setting_t *setting;
        char *config_file = "../network.conf";
        config_init(&cfg);
        if (!config_read_file(&cfg, config_file)) {
            printf("\n%s:%d - %s", config_error_file(&cfg), 
                    config_error_line(&cfg), config_error_text(&cfg));
            config_destroy(&cfg);
            return -1;
        }
        setting = config_lookup(&cfg, "dest");
        if (setting != NULL) {
            if (config_setting_lookup_string(setting, "recv_port", &port)) {
                printf("\n recv_port: %s\n", port);
            } else {
                printf("receiving port is not configured\n");
                return -1;
            }
            if (config_setting_lookup_string(setting, "out", &output)) {
                printf("\n output: %s\n", output);
            } else {
                printf("output file is not configured \n");
                return -1;
            }
            if (config_setting_lookup_string(setting, "log", &log_file)) {
                printf("\n log file: %s\n", log_file);
            } else {
                printf("log file is not configured \n");
                return -1;
            }
        } else {
            printf("no configuration for receive\n");
            return -1;
        }
    }
    #endif

    fp = fopen(output, "w");
    LOG_ASSERT(log_fp, LL_ERROR, fp!=NULL);

    res_fp = fopen(log_file, "w");
    LOG_ASSERT(log_fp, LL_ERROR, res_fp!=NULL);

    get_packets((char *)port, fp, res_fp);
    return 0;
}
#endif

/**
 * @brief sets socket file descriptor
 * to receive stream
 *
 * @param[in] port
 */
static int sock_init(
                     char *port)
{
    int sockfd, ret, yes = 1;
    
    struct addrinfo hints, *addr;
    addr = (struct addrinfo*) 
        malloc(sizeof(struct addrinfo*));

    // set hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    ret = getaddrinfo(NULL, port, &hints, &addr);
    LOG_ASSERT(log_fp, LL_ERROR, ret==0);

    sockfd = socket(addr->ai_family,
            addr->ai_socktype, addr->ai_protocol);
    LOG_ASSERT(log_fp, LL_ERROR, sockfd!=-1);

    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
            &yes, sizeof(int));
    LOG_ASSERT(log_fp, LL_ERROR, ret==0);

    ret = bind(sockfd, addr->ai_addr, addr->ai_addrlen);
    LOG_ASSERT(log_fp, LL_ERROR, ret!=-1);

    LOG_ASSERT(log_fp, LL_ERROR, addr!=NULL);

    return sockfd;
}

/**
 * @brief receives data and writes onto
 * file
 *
 * @param[in] pfd
 * @param[in] sockfd
 * @param[in] fp
 * @param[in] res_fp
 */
static void recv_loop(
                      struct pollfd *pfd, 
                      int           sockfd, 
                      FILE          *fp, 
                      FILE          *res_fp
                     )
{
    int numbytes = 0, recv_count = 0,
        total = 0, i = 0;
    char buffer[BLOCKSIZE];
    char log_str[100];
    clock_t t1, t2;
    float t_diff;

    t1 = clock();

    // receive until connection closed
    while (true) {
        if (poll(pfd, 1, 100) > 0) {
            recv_count = 0;
            if (pfd->revents == POLLIN) {
                memset(buffer, '\0', BLOCKSIZE);
                while (recv_count < BLOCKSIZE) {
                    numbytes = recv(sockfd, buffer+recv_count,
                            BLOCKSIZE-recv_count, 0);
                    if (numbytes > 0) {
                        recv_count += numbytes;
                        total += numbytes;
                    } else if (numbytes == 0)
                        goto CLOSE_CONN;
                }
                if (recv_count > 0) {
                    t2 = clock();
                    t_diff = ((float) (t2 - t1) / 1000000) * 1000;
                    sprintf(log_str, "diff: %f bytes: %d\n",
                            t_diff, total);
                    fprintf(fp, "%s", buffer);
                    fprintf(res_fp, "%s", log_str);
                    fflush(fp);
                    fflush(res_fp);
                }
            }
        }
    }
CLOSE_CONN:
    if (recv_count > 0) {
        total += recv_count;
        t2 = clock();
        t_diff = ((float) (t2 - t1) / 1000000) * 1000;
        sprintf(log_str, "diff: %f bytes: %d\n",
                t_diff, total);
        fprintf(res_fp, "%s", log_str);
        for (i = 0; i < recv_count; i++) {
            if (buffer[i] != EOF) {
                fprintf(fp, "%c", buffer[i]);
                fflush(fp);
            }
        }
    }
    fclose(fp);
}

/**
 * @brief set receiving socket file 
 * descriptor and enters to recv_loop
 *
 * @param[in] port
 * @param[in] fp
 * @param[in] res_fp
 */
void get_packets(
                 char *port, 
                 FILE *fp, 
                 FILE *res_fp
                ) 
{ 
    int sockfd, ret;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct pollfd pfd;

    sin_size = sizeof(their_addr);
    sockfd = sock_init(port);

    ret = listen(sockfd, 1);
    LOG_ASSERT(log_fp, LL_ERROR, ret==0);

    printf("end destination waits connection \n");
    sockfd = accept(sockfd, 
            (struct sockaddr*)&their_addr, &sin_size);
    LOG_ASSERT(log_fp, LL_ERROR, sockfd!=-1);
    printf("end destination accepted connection \n");

    pfd.fd = sockfd;
    pfd.events = POLLIN;

    recv_loop(&pfd, sockfd, fp, res_fp);
}


/**
 * @brief 
 *
 * @param token
 * @param value
 * @param 
 */
static void eval_config_item(
                             char const          *token,
                             char const          *value, 
                             struct arg_configer *arg_conf
                            ) 
{
    if (!strcmp(token, "port")) {
        strcpy(arg_conf->port, value); 
        printf("arg_conf->port: %s\n", arg_conf->port);
        return;
    }

    if (!strcmp(token, "output")) {
        strcpy(arg_conf->output, value);
        printf("arg_conf->output: %s\n", arg_conf->output);
        return;
    }

    if (!strcmp(token, "log_file")) {
        strcpy(arg_conf->log_file, value);
        printf("arg_conf->log_file: %s\n", arg_conf->log_file);
        return;
    }
}
