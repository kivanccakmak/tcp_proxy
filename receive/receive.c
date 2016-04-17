#include "receive.h"

static void recv_loop(struct pollfd *pfd, int sockfd, 
        FILE *fp, FILE *logp);

static void get_packets(char *port, FILE *fp, FILE *logp);

static int sock_init(char *port);

static const int num_options = 3;

static void eval_config_item(char const *token,
        char const *value, struct arg_configer *arg_conf); 

#ifdef RECV
int main(int argc, char **argv) 
{
    const char *port, *output, *log_file;
    FILE *fp, *logp;

    if (argc == 4) {
        int c = 0, i, option_index = 0;
        struct arg_configer arg_conf;
        for(;;) {
            c = getopt_long(argc, argv, "",
                    long_options, &option_index);

            if (c == -1)
                break;

            if (c == '?' || c == ':')
                exit(1);

            for (i = 0; i < num_options; i++) {
                if (long_options[i].val == c) {
                    eval_config_item(long_options[i].name, optarg,
                            &arg_conf);
                }
            }
        }
        port = arg_conf.port;
        output = arg_conf.output;
        log_file = arg_conf.log_file;
    }

    #ifdef CONF_ENABLE
    if (argc == 1) {
        config_t cfg;
        config_setting_t *setting;
        char *config_file = "../network/network.conf";
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

    fp = fopen(output, "a");
    logp = fopen(log_file, "a");
    get_packets((char *)port, fp, logp);
    return 0;
}
#endif

/**
 * @brief sets socket file descriptor
 * to receive stream
 *
 * @param[in] port
 */
static int sock_init(char *port)
{
    int sockfd, rs_addr, yes = 1;
    
    struct addrinfo hints, *addr;
    addr = (struct addrinfo*) 
        malloc(sizeof(struct addrinfo*));

    // set hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rs_addr = getaddrinfo(NULL, port, &hints, &addr);
    if (rs_addr != 0) {
        fprintf(stderr,
                "getaddrinfo: %s\n", gai_strerror(rs_addr));
    }

    sockfd = socket(addr->ai_family,
            addr->ai_socktype, addr->ai_protocol);

    if (setsockopt(sockfd, SOL_SOCKET, 
                SO_REUSEADDR, &yes, sizeof(int)) != 0) {
        perror("setsockopt: ");
        exit(1);
    }

    if (bind(sockfd, addr->ai_addr, addr->ai_addrlen) == -1) {
        close(sockfd);
        perror("server: bind");
        exit(1);
    }

    if (addr == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    return sockfd;
}

/**
 * @brief receives data and writes onto
 * file
 *
 * @param[in] pfd
 * @param[in] sockfd
 * @param[in] fp
 * @param[in] logp
 */
static void recv_loop(struct pollfd *pfd, int sockfd, 
        FILE *fp, FILE *logp)
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
                    fprintf(logp, "%s", log_str);
                    fflush(fp);
                    fflush(logp);
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
        fprintf(logp, "%s", log_str);
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
 * @param[in] logp
 */
void get_packets(char *port, FILE *fp, FILE *logp) 
{ 
    int sockfd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct pollfd pfd;

    sin_size = sizeof(their_addr);
    sockfd = sock_init(port);

    printf("end destination listens \n");
    if (listen(sockfd, 1) != 0) {
        perror("listen: ");
        exit(1);
    }

    printf("end destination waits connection \n");
    sockfd = accept(sockfd, 
            (struct sockaddr*)&their_addr, &sin_size);
    printf("end destination accepted connection \n");

    pfd.fd = sockfd;
    pfd.events = POLLIN;

    recv_loop(&pfd, sockfd, fp, logp);
}

static void eval_config_item(char const *token,
        char const *value, struct arg_configer *arg_conf) {
    if (!strcmp(token, "port")) {
        strncpy(arg_conf->port, value, (int) sizeof(value)); 
        printf("arg_conf->port: %s\n", arg_conf->port);
        return;
    }

    if (!strcmp(token, "output")) {
        strncpy(arg_conf->output, value, (int) sizeof(value));
        printf("arg_conf->output: %s\n", arg_conf->output);
        return;
    }

    if (!strcmp(token, "log_file")) {
        strncpy(arg_conf->log_file, value, (int) sizeof(value));
        printf("arg_conf->log_file: %s\n", arg_conf->log_file);
        return;
    }
}
