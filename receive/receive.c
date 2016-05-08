#include "receive.h"

static struct option long_options[] = {
    {"recv_port", required_argument, NULL, 'A'},
    {"out", required_argument, NULL, 'B'},
    {"log", required_argument, NULL, 'C'}
};

static void recv_loop(
                      struct pollfd *pfd, 
                      int           sockfd, 
                      FILE          *wrt_fp,
                      FILE          *res_fp
                     );

static void get_packets(
                        char *port, 
                        FILE *fp, 
                        FILE *res_fp
                       );

static int sock_init(char *port);

static FILE *log_fp; /* error logger fp */

#ifdef RECV
int main(int argc, char **argv) 
{
    const char *port, *output, *log_file;  /* expected argvs             */
    FILE *wrt_fp, *res_fp;                /*  wrt_fp: data writer fp     */
    int i = 0, ret;                      /*   res_fp: data amount logger */

    log_fp = fopen(RECV_LOG, "w");
    if (log_fp == NULL) {
        perror("logfp: ");
        exit(1);
    }

    arg_val_t **arg_vals = init_arg_vals(
            (int) RECEIVE_ARGV_NUM-1, long_options);

    if (argc == RECEIVE_ARGV_NUM) { // running via command argvs
        ret = argv_reader(arg_vals,
                long_options, argv, (int) RECEIVE_ARGV_NUM);
        LOG_ASSERT(log_fp, LL_ERROR, ret==0);
    } else if (argc == 1) {        // running via config file
        char *config_file = "../network.conf";
        config_t cfg;
        config_init(&cfg);
        config_setting_t *setting;
        if (!config_read_file(&cfg, config_file)) {
            printf("\n%s:%d - %s", config_error_file(&cfg),
                    config_error_line(&cfg), config_error_text(&cfg));
            config_destroy(&cfg);
            return -1;
        }
        setting = config_lookup(&cfg, "dest");
        if (setting != NULL) {
            ret = config_reader(arg_vals, setting, RECEIVE_ARGV_NUM-1);
            LOG_ASSERT(log_fp, LL_ERROR, ret==0);
        } else {
            printf("no configuration for receive\n");
            return -1;
        }
    } else { printf("wrong usage\n"); return -1; }

    port = get_argv((char *) "recv_port", arg_vals, RECEIVE_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, port!=NULL);
    output = get_argv((char *) "out", arg_vals, RECEIVE_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, output!=NULL);
    log_file = get_argv((char *) "log", arg_vals, RECEIVE_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, log_file!=NULL);

    wrt_fp = fopen(output, "w");
    LOG_ASSERT(log_fp, LL_ERROR, wrt_fp!=NULL);
    res_fp = fopen(log_file, "w");
    LOG_ASSERT(log_fp, LL_ERROR, res_fp!=NULL);

    printf("port:%s, output:%s, log_file:%s\n", port, output, log_file);
    get_packets((char *)port, wrt_fp, res_fp);
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
                      FILE          *wrt_fp,
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
                    fprintf(wrt_fp, "%s", buffer);
                    fprintf(res_fp, "%s", log_str);
                    fflush(wrt_fp);
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
                fprintf(wrt_fp, "%c", buffer[i]);
                fflush(wrt_fp);
            }
        }
    }
    fclose(wrt_fp);
    fclose(res_fp);
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
                 FILE *wrt_fp,
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

    recv_loop(&pfd, sockfd, wrt_fp, res_fp);
}
