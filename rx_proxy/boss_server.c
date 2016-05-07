#include "boss_server.h"

struct arg_configer{
    char dest_ip[IP_CHAR_MAX];
    char dest_port[PORT_MAX_CHAR];
    char port[PORT_MAX_CHAR];
};

static struct option long_options[] = {
    {"recv_port", required_argument, NULL, 'A'},
    {"dest_port", required_argument, NULL, 'B'},
    {"dest_ip", required_argument, NULL, 'C'}
};

static void sigchld_handler(int s);

static struct sigaction sig_init();

static void accept_loop(int sockfd, pool_t* pl); 

static void eval_config_item(
                             char const          *token,
                             char const          *value, 
                             struct arg_configer *arg_conf
                            );

static FILE *log_fp; /* error logger fp */

#ifdef RX_PROXY
int main(int argc, char ** argv)
{
    const char *dest_ip, *dest_port, /*forward to dest_ip:dest_port */
          *recv_port;               /* recv_port: accept conns from */

    int ret;
    pool_t *pl;
    queue_args_t *que_args;
    pthread_t que_id, serv_id;
    que_args = (queue_args_t *) malloc(sizeof(queue_args_t));

    pl = pool_init();
    log_fp = fopen(RX_PROXY_LOG, "w+");
    if (log_fp == NULL) {
        perror("fopen: ");
        exit(1);
    }

    arg_val_t **arg_vals = init_arg_vals(
            (int) RX_PROXY_ARGV_NUM-1, long_options);

    if (argc == RX_PROXY_ARGV_NUM) { //running via command argvs
        ret = argv_reader(arg_vals,
                long_options, argv, (int) RX_PROXY_ARGV_NUM);
        LOG_ASSERT(log_fp, LL_ERROR, ret==0);
    } else if (argc == 1) {
        char *config_file = "../network.conf";
        config_t cfg;
        config_setting_t *setting;
        config_init(&cfg);
        if (!config_read_file(&cfg, config_file)) {
            printf("\n%s:%d - %s", config_error_file(&cfg),
                    config_error_line(&cfg), config_error_text(&cfg));
            config_destroy(&cfg);
            return -1;
        }
        setting = config_lookup(&cfg, "rx_proxy");
        if (setting != NULL) {
            ret = config_reader(arg_vals, setting, RX_PROXY_ARGV_NUM-1);
            LOG_ASSERT(log_fp, LL_ERROR, ret==0);
        }
    }

    dest_ip = get_argv((char *) "dest_ip", arg_vals, RX_PROXY_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, dest_ip!=NULL);
    dest_port = get_argv((char *) "dest_port", arg_vals, RX_PROXY_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, dest_port!=NULL);
    recv_port = get_argv((char *) "recv_port", arg_vals, RX_PROXY_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, recv_port!=NULL);

    printf("dest-> %s:%s, recv_port:%s\n", dest_ip, dest_port,
            recv_port);

    que_args->pl = pl;
    que_args->dest_ip = (char *) dest_ip;
    que_args->dest_port = (char *) dest_port;
    
    ret = pthread_create(&que_id, NULL, &wait2forward,
            (void*) que_args);
    LOG_ASSERT(log_fp, LL_ERROR, ret==0);
    sleep(3);
    
    server_start((char*) recv_port, pl);
    pthread_join(que_id, NULL);
    return 0;
}
#endif

/**
 * @brief initialize packet pool 
 * struct to be used by receiver threads.
 *
 * @return 
 */
pool_t* pool_init() 
{
    pqueue_t *pq;
    pool_t *pl;
    pthread_condattr_t attr;

    // initialize priority queue
    pq = pqueue_init(10, cmp_pri, get_pri, set_pri,
            get_pos, set_pos);

    pl = (pool_t *) malloc(sizeof(pool_t));
    pl->pq = (pqueue_t*) malloc(sizeof(pqueue_t));
    pl->pq = pq;
    pl->avail_min_seq = 0;
    pl->sent_min_seq = 0;

    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);

    pthread_cond_init(&pl->cond, &attr);

    pthread_mutex_init(&pl->lock, NULL);
    pl->pq = pq;
    
    return pl;
}

 /**
  * @brief initializes server socket
  * and enters connection accepting
  * loop
  *
  * @param[in] server_port
  * @param[in] pl
  */
void server_start(char* server_port, pool_t *pl)
{
    int sockfd, ret, yes = 1;
    struct sigaction sig_sa;
    struct addrinfo hints, *addr;

    // set server socket
    addr = (struct addrinfo*) malloc(sizeof(struct addrinfo));
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    ret = getaddrinfo(NULL, server_port,
            &hints, &addr);
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

    printf("-listen()\n");
    ret = listen(sockfd, BACKLOG);
    LOG_ASSERT(log_fp, LL_ERROR, ret!=-1);

    sig_sa = sig_init();
    accept_loop(sockfd, pl);
}

/**
 * @brief Listens from ip_addr:port and 
 * initiates new thread whenever 
 * new TCP connection established
 * from transmitter side of proxy.
 *
 * @param[in] sockfd
 * @param[in] pl
 */
static void accept_loop(int sockfd, pool_t* pl)
{
    int ret, newfd;
    char tx_ip[INET6_ADDRSTRLEN];
    uint32_t tx_port;
    struct sockaddr_storage their_addr;
    rx_args_t *rx_args = NULL;
    socklen_t sin_size;
    sin_size = sizeof(their_addr);

    while (true) {
        newfd = accept(sockfd, (struct sockaddr*)&their_addr,
                &sin_size);
        LOG_ASSERT(log_fp, LL_ERROR, newfd!=-1);

        pthread_t t_id;
        inet_ntop(their_addr.ss_family,
                get_in_ipaddr((struct sockaddr*)&their_addr),
                tx_ip, sizeof(tx_ip));
        LOG_ASSERT(log_fp, LL_ERROR, tx_ip!=NULL);

        tx_port = get_in_portnum((struct sockaddr*)&their_addr);
        printf("connection %s : %d \n", tx_ip, tx_port);
        rx_args = (rx_args_t*) malloc(sizeof(rx_args_t));
        rx_args->sockfd = newfd;
        rx_args->pl = pl;
        rx_args->poll_timeout = 100;
        ret = pthread_create(&t_id, NULL, &rx_chain, rx_args);
        LOG_ASSERT(log_fp, LL_ERROR, ret==0);
    }
}

/**
 * @brief sets checks argument variable key and 
 * sets argument var config struct
 *
 * @param token[in]
 * @param value[in]
 * @param arg_conf[out]
 */
static void eval_config_item(
                             char const *token,
                             char const *value, 
                             struct arg_configer *arg_conf
                            )
{
    if (!strcmp(token, "port")) {
        strcpy(arg_conf->port, value); 
        printf("arg_conf->port: %s\n", arg_conf->port);
        return;
    }

    if (!strcmp(token, "dest_port")) {
        strcpy(arg_conf->dest_port, value);
        printf("arg_conf->output: %s\n", arg_conf->dest_port);
        return;
    }

    if (!strcmp(token, "dest_ip")) {
        strcpy(arg_conf->dest_ip, value);
        printf("arg_conf->log_file: %s\n", arg_conf->dest_ip);
        return;
    }
}

/**
 * @brief  Initialize sigaction to wait
 * incoming connections.
 *
 * @return 
 */
static struct sigaction sig_init()
{
    int ret;
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    ret = sigaction(SIGCHLD, &sa, NULL);
    LOG_ASSERT(log_fp, LL_ERROR, ret==0);

    return sa;
}

/**
 * @brief 
 *
 * wait for a process to change
 * state
 *
 * @param s
 */
static void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}
