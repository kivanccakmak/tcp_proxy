#include "split.h"

#define QUEUE_NUM 0 

struct arg_configer{
    char rx_proxy_ip[IP_CHAR_MAX];
    char rx_proxy_port[PORT_MAX_CHAR];
    char local_port[PORT_MAX_CHAR];
};

static struct option long_options[] = {
    {"local_port", required_argument, NULL, 'A'},
    {"rx_proxy_port", required_argument, NULL, 'B'},
    {"rx_proxy_ip", required_argument, NULL, 'C'}
};

static int nfqueue_get_syn(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa,
        void *data);

static void* queuer_loop(void __attribute__((unused)) *unused);

static void* pass_payload(void *args);

static struct listen_args* set_listen_args(
        char *dest_ip, char *dest_port, char *local_part);

static int set_tx_sock(char *dest_ip, char *dest_port); 

static int add2buff(proxy_buff *buff, char *raw_buf, int recv_count); 

static struct split_args* set_split_args(char *local_port);

static void* get_payload(void *args); 

static struct cb_cntrl_args* set_controller_args(char *dest_ip, 
    char *dest_port, proxy_buff *buff); 

static void set_link(char *dest_ip, char *dest_port,
        struct link *tcp_link);

static void *run_controller(void *args);

static void *tx_chain(void *args); 

static void split_loop(int sockfd, proxy_buff *buff);

static void eval_config_item(char const *token,
        char const *value, struct arg_configer *arg_conf);

static const int num_options = 3;

#ifdef TX_PROXY
int main(int argc, char *argv[])
{
    int ret;
    struct split_args *split = NULL;
    struct cb_cntrl_args *cntrl_args = NULL;
    pthread_t queuer_id, getter_id, controller_id;
    
    // local_port: get hijacked data from this port
    // dest_ip, dest_port: send hijacked 
    // data to there(rx_proxy)
    const char *dest_ip, *dest_port, *local_port;

    // argv getter from terminal
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
                    eval_config_item(long_options[i].name,
                            optarg, &arg_conf);
                }
            }
        }
        local_port = arg_conf.local_port;
        dest_ip = arg_conf.rx_proxy_ip;
        dest_port = arg_conf.rx_proxy_port;
    }
    
    // no argv variable, parameters set from config file
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
        setting = config_lookup(&cfg, "tx_proxy");
        if (setting != NULL) {
            if (config_setting_lookup_string(setting, "local_port", &local_port)) {
                printf("\n local_port: %s\n", local_port);
            } else {
                printf("local port of tx_proxy is not configured \n");
                return -1;
            }
            if (config_setting_lookup_string(setting, "rx_proxy_ip", &dest_ip)) {
                printf("\n dest_ip: %s\n", dest_ip);
            } else {
                printf("ip address of rx_proxy is not configured\n");
                return -1;
            }
            if (config_setting_lookup_string(setting, "rx_proxy_port", &dest_port)) {
                printf("\n dest_port: %s\n", dest_port);
            } else {
                printf("port of rx_proxy is not configured\n");
                return -1;
            }
        }
    }
    #endif


    split = set_split_args((char *)local_port);
    cntrl_args = set_controller_args((char *)dest_ip,
            (char *)dest_port, split->buff);

    ret = pthread_create(&queuer_id, NULL, &queuer_loop, NULL);
    if (ret) {
        printf("Failed to create queuer thread! [RET = %d]", ret);
        exit(0);
    } else {
        printf("queuer thread initiated\n");
    }

    ret = pthread_create(&getter_id, NULL, &get_payload,
            (void *) split);
    if (ret) {
        printf("Failed to initiate payload getter. [RET = %d]", ret);
        exit(0);
    } else {
        printf("payload thread initiated\n");
    }

    ret = pthread_create(&controller_id, NULL, &run_controller,
            (void *) cntrl_args);
    if (ret) {
        printf("Failed to initiate controller thread. [RET = %d]", ret);
        exit(0);
    } else {
        printf("controller thread initiated\n");
    }

    pthread_join(controller_id, NULL);
    pthread_join(queuer_id, NULL);
    pthread_join(getter_id, NULL);

    return 0;
}
#endif


/**
 * @brief TCP connection controller function,
 * which opens and closes TCP connections.
 *
 * @param[in] args
 *
 * @return 
 */
static void *run_controller(void *args)
{
    int i = 0;
    struct cb_cntrl_args *cntrl_args = NULL;
    struct link **tcp_links = NULL;
    struct link *temp_link = NULL;
    struct cb_tx_args **tx_args = NULL;
    char *dest_ip = (char *) malloc(sizeof(char*));;
    char *dest_port = (char *) malloc(sizeof(char*));

    cntrl_args = (struct cb_cntrl_args*)
        malloc(sizeof(struct cb_cntrl_args*));
    cntrl_args = (struct cb_cntrl_args*) args;
    pthread_t thr_ids[cntrl_args->conn_number];

    tcp_links = (struct link**) 
        malloc(sizeof(struct link*) * cntrl_args->conn_number);
    tx_args = (struct cb_tx_args**)
        malloc(sizeof(struct cb_tx_args*) * cntrl_args->conn_number);

    for (i = 0; i < cntrl_args->conn_number; i++) {
        tcp_links[i] = (struct link*) malloc(sizeof(struct link));
        tx_args[i] = (struct cb_tx_args*) malloc(sizeof(struct cb_tx_args));
    }

    /*printf("set pointer arrays\n");*/
    temp_link = (struct link*) malloc(sizeof(struct link*)); 
    dest_ip = cntrl_args->dest_ip;
    dest_port = cntrl_args->dest_port;
    printf("%s:%s\n", dest_ip,dest_port);

    // controlller just opens multiple connections, not
    // manages number of them etc.
    printf("entering loop...\n");
    for (i = 0; i < cntrl_args->conn_number; i++) {
        set_link(dest_ip, dest_port, tcp_links[i]);
        tx_args[i]->buff = cntrl_args->buff;
        tx_args[i]->tx_link = tcp_links[i];
        sleep(1);
        pthread_create(&(thr_ids[i]), NULL, 
                &tx_chain, (void*) tx_args[i]);
    }

    for (i = 0; i < cntrl_args->conn_number; i++)
        pthread_join(thr_ids[i], NULL);

    return NULL;
}


/**
 * @brief Sinlge TCP connection thread
 * that gets raw data buffer and passes 
 * towards end destination.
 *
 * @param args
 *
 * @return 
 */
static void *tx_chain(void *args) 
{
    struct cb_tx_args *tx_args = NULL;
    struct link *tx_link = NULL;
    encaps_packet_t packet;
    proxy_buff *buff;
    int diff = 0, ind = 0, sent_count = 0, 
        numbytes = 0, total = 0, sockfd;

    tx_args = (struct cb_tx_args*)  malloc(sizeof(struct cb_tx_args));
    buff = (proxy_buff*) malloc(sizeof(proxy_buff));
    tx_link = (struct link*) malloc(sizeof(struct link));
    tx_args = (struct cb_tx_args*) args;
    buff = tx_args->buff;
    tx_link = tx_args->tx_link;
    sockfd = tx_link->fd;

    while (true) {
        pthread_mutex_lock(&buff->lock);
        diff = buff->set_ind - buff->get_ind;
        if (diff >= 0){ 
            ind = buff->get_ind;
            memcpy(packet.raw_packet, buff->buffer[ind], BLOCKSIZE);
            packet.seq = (unsigned short) buff->get_ind;
            buff->get_ind++;
            pthread_mutex_unlock(&buff->lock);
            sent_count = 0;
            while (sent_count < (int) PACKET_SIZE) {
                numbytes = send(sockfd, 
                    &((unsigned char*) &packet)[sent_count],
                        (size_t) PACKET_SIZE-sent_count, 0);
                if (numbytes > 0) {
                    sent_count += numbytes;
                    total += numbytes;
                } else if(numbytes == 0)
                    goto CONN_CLOSE;
            }
        } else {
            if (buff->fin_flag == true) {
                pthread_mutex_unlock(&buff->lock);
                goto CONN_CLOSE;
            }
            pthread_mutex_unlock(&buff->lock);
            sleep(1);
        }  
        pthread_mutex_lock(&tx_link->lock);
        if (tx_link->state == PASSIVE) {
            pthread_mutex_unlock(&tx_link->lock);
            goto CONN_CLOSE;
        }
        pthread_mutex_unlock(&tx_link->lock);
    }
CONN_CLOSE:
    printf("closing tx_chain socket\n");
    close(sockfd);
}

/**
 * @brief sets network parameters of link struct, 
 * which would be passed to tx_chain() thread
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 * @param[out] tcp_link
 */
static void set_link(char *dest_ip, char *dest_port,
        struct link *tcp_link) 
{
    struct sockaddr_in server;
    int sockfd;
    socklen_t len = sizeof(server);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_addr.s_addr = inet_addr(dest_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(dest_port));

    printf("tcp link connecting ...\n");
    if (connect(sockfd, (struct sockaddr*)&server,
                sizeof(server)) > 0) {
        perror("connection failed. Error");
        exit(0);
    } else {
        tcp_link->fd = sockfd;
    }

    printf("connected ... \n");
    if (getsockname(sockfd, (struct sockaddr*)&server,
                &len) == -1) {
        perror("getsockname");
        exit(0);
    } else {
        tcp_link->src_port = ntohs(server.sin_port);
    }

    tcp_link->state = ACTIVE;
    pthread_mutex_init(&tcp_link->lock, NULL);
    pthread_cond_init(&tcp_link->cond, NULL);
}

/**
 * @brief set call-back arguments 
 * of controller thread.
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 * @param[in] buff
 *
 * @return 
 */
static struct cb_cntrl_args* set_controller_args(char *dest_ip,
        char *dest_port, proxy_buff *buff) 
{
    struct cb_cntrl_args *cntrl_args = NULL;

    cntrl_args = (struct cb_cntrl_args*)
        malloc(sizeof(struct cb_cntrl_args));

    cntrl_args->dest_ip = dest_ip;
    cntrl_args->dest_port = dest_port;
    cntrl_args->buff = buff;
    cntrl_args->conn_number = CONN_NUMBER;
    return cntrl_args;
}

/**
 * @brief 
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 * @param[in] local_port
 *
 * @return 
 */
static struct split_args* set_split_args(char *local_port)
{
    int i = 0;
    struct split_args *split = 
        (struct split_args*) malloc(sizeof(struct split_args));

    split->local_port = local_port;

    proxy_buff *buff = 
        (proxy_buff*) malloc(sizeof(proxy_buff));

    buff->rx_byte = 0;
    buff->tx_byte = 0;
    buff->set_ind = -1;
    buff->get_ind = 0;
    buff->fin_flag = false;
    buff->capacity = INITIAL_CAPACITY;
    buff->buffer = (char **) 
        malloc(sizeof(char*) * buff->capacity);
    pthread_mutex_init(&buff->lock, NULL);
    for (i = 0; i < INITIAL_CAPACITY; i++) {
        buff->buffer[i] = (char *) malloc(BLOCKSIZE * sizeof(char));
    }

    split->buff = buff;
    return split;
}

static void *queuer_loop(void __attribute__((unused)) *unused)
{
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    struct nfnl_handle *nh;

    int fd, i, ret;
    char buf[QUEUER_BUF_SIZE];
    ssize_t rc;

    h = nfq_open();
    if (!h) {
        printf("Failed to open NFQ handler! \n");
        exit(0);
    }

    printf("unbinding existing nf_queue handler "
            "for AF_INET (if any) \n");
    nfq_unbind_pf(h, AF_INET);

    printf("binding nfnetlink_queue as "
            "nf_queue handler for AF_INET\n");
    ret = nfq_bind_pf(h, AF_INET);
    if (ret < 0) {
        printf("Failed to bind NFQ handler! [RET = %d] \n", ret);
        exit(0);
    }

    printf("binding this socket to queue '%d'\n", QUEUE_NUM);
    qh = nfq_create_queue(h, QUEUE_NUM, &nfqueue_get_syn, NULL);
    if (!qh) {
        printf("Failed to create nfnetlink queue!\n");
        exit(0);
    }

    printf("setting copy_packet mode\n");
    ret = nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);
    if (ret < 0) {
        printf("Failed to setup NFQ packet_copy mode! [RET = %d]\n", ret);
        exit(0);
    }

    nh = nfq_nfnlh(h);
    fd = nfnl_fd(nh);

    printf("in loop...\n");
    while ((rc = recv(fd, buf, QUEUER_BUF_SIZE, 0)) >= 0) {
        printf("received packet [sz = %d]\n", (int) rc);
        nfq_handle_packet(h, buf, rc);
        for (i = 0; i < (int) rc;  i++) {
            printf("%c", (char) buf[i]);
        }
    }

    printf("Exiting [rc=%d]\n", (int) rc);
    nfq_close(h);
    pthread_exit(NULL);
}


/**
 * @brief initialize socket file descriptor
 * to get data from netfilter, then calls
 * split_loop().
 *
 * @param args
 *
 * @return 
 */
static void *get_payload(void *args) 
{
    int yes = 1, rs_addr;
    struct addrinfo hints;
    struct addrinfo *addr = NULL;
    struct split_args *split = NULL;
    proxy_buff *buff = NULL;
    char *local_port = NULL;
    int sockfd;

    // get call back arguments
    split = (struct split_args*) 
        malloc(sizeof(struct split_args*));
    buff = (proxy_buff *) malloc(sizeof(proxy_buff*));
    split = (struct split_args*) args;
    local_port = split->local_port;
    buff = split->buff;
    // set local socket fd to get data from nfqueue 
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addr = (struct addrinfo*) 
        malloc(sizeof(struct addrinfo*));
    rs_addr = getaddrinfo(NULL, local_port, &hints, &addr);

    if (addr == NULL) {
        fprintf(stderr, "server: failed to bind \n");
        exit(1);
    }  
    sockfd = socket(addr->ai_family, addr->ai_socktype,
            addr->ai_protocol);
    if (sockfd == -1) {
        perror("socket:");
        exit(0);
    }
    if (setsockopt(sockfd, SOL_SOCKET,
                SO_REUSEADDR, &yes, sizeof(int)) != 0) {
        perror("setsockopt: ");
        exit(0);
    }
    if (bind(sockfd, addr->ai_addr, 
                addr->ai_addrlen) == -1) {
        close(sockfd);
        perror("server: bind");
    }

    split_loop(sockfd, buff);
    return NULL;
}

/**
 * @brief gets hijacked data from netfilter,  
 * and calls add2buff().
 *
 * @param sockfd to get data from nfqueue.
 * @param buff buffer of transmit proxy,
 * to sync with forwarder threads.
 */
static void split_loop(int sockfd, proxy_buff *buff) 
{
    int listen_val, recv_count = 0, 
        numbytes = 0, total = 0, diff = 0;
    struct sockaddr_storage their_addr;
    struct pollfd pfd;
    socklen_t sin_size = sizeof(their_addr);
    char src_ip[INET6_ADDRSTRLEN], *raw_buf = NULL;
    uint32_t src_port;

    listen_val = listen(sockfd, 1);
    sockfd = accept(sockfd, 
            (struct sockaddr*)&their_addr, &sin_size);
    inet_ntop(their_addr.ss_family, 
            get_in_ipaddr((struct sockaddr*)&their_addr),
            src_ip, sizeof(src_ip));
    src_port = get_in_portnum((struct sockaddr*)&their_addr);
    printf("connection established with %s:%d\n",
            src_ip, (int) src_port);

    pfd.fd = sockfd;
    pfd.events = POLLIN | POLLERR;
    while (true) {
        if (poll(&pfd, 1, 100) > 0) {
            recv_count = 0;
            if (pfd.revents == POLLERR){
                perror("perr: ");
                goto CONN_CLOSE;
            } else if (pfd.revents == POLLIN) {
                raw_buf = (char *) malloc(BLOCKSIZE);
                while (recv_count < (int) BLOCKSIZE) {
                    numbytes = recv(sockfd, raw_buf+recv_count, 
                            BLOCKSIZE-recv_count, 0);
                    if (numbytes > 0) {
                        recv_count += numbytes;
                        total += numbytes;
                    } else
                        goto CONN_CLOSE;
                }
                if (recv_count > 0) {
                    diff = add2buff(buff, raw_buf, recv_count);
                }
           }
        } 
    }
CONN_CLOSE:
    if (recv_count > 0) {
        diff = add2buff(buff, raw_buf, recv_count);
        total += diff;
    }
    printf("no more interception, bytes: %d\n", total);
    pthread_mutex_lock(&buff->lock);
    buff->fin_flag = true;
    pthread_mutex_unlock(&buff->lock);
}

/**
 * @brief adds proxied payload to local 
 * buffer and updates count variables 
 * of proxy_buff 
 *
 * @param[out] buff
 * @param[in] raw_buf
 * @param[in] recv_count
 *
 * return 
 */
static int add2buff(proxy_buff *buff, char *raw_buf,
        int recv_count) 
{
    bool extend = false;
    int remained = 0, i = 0, pre_count = 0;

    // fill remaining as NULL, if residue
    if (recv_count > 0 && recv_count < BLOCKSIZE) {
        int count = 0;
        for (count = recv_count; count < BLOCKSIZE; count++){
            raw_buf[count] = '\0';
        }
    }

    pthread_mutex_lock(&buff->lock);

    // check capacity overflow
    remained = buff->capacity - buff->set_ind;
    extend = (remained) < (2 * (int) sizeof(char*));
    if (extend) {
        pre_count = buff->capacity;
        buff->capacity = buff->capacity * 2;
        buff->buffer = (char **) realloc(buff->buffer,
                sizeof(char*) * buff->capacity);
        for (i = pre_count; i < buff->capacity; i++) {
            buff->buffer[i] = (char *) 
                malloc(sizeof(char) * BLOCKSIZE);
        }

    }
    buff->set_ind++;
    buff->buffer[buff->set_ind] = raw_buf;
    buff->rx_byte += BLOCKSIZE;

    pthread_mutex_unlock(&buff->lock);
    return buff->set_ind - buff->get_ind;
}

/**
 * @brief set socket file descriptor
 * to forward data through receiver proxy.
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 *
 * @return 
 */
static int set_tx_sock(char *dest_ip, char *dest_port) 
{
    int sockfd;
    int conn_res;

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(dest_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(dest_port));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    conn_res = connect(sockfd, (struct sockaddr*)&server,
            sizeof(server));

    if (conn_res < 0) {
        perror("connection failed. Error");
        exit(0);
    }

    return sockfd;

}

/**
 * @brief  
 *
 * @param qh
 * @param nfmsg
 * @param nfa
 * @param data
 *
 * @return 
 */
static int nfqueue_get_syn(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data) 
{
    printf("in get syn\n");
    unsigned char *buffer;
    struct ipv4_packet *ip4;
    int id = 0, ret;
    struct nfqnl_msg_packet_hdr *ph;

    printf("**********************\n");
    printf("in nfqueue_get_syn\n");
    printf("**********************\n");

    ph = nfq_get_msg_packet_hdr(nfa);
    if (!ph) {
        perror("unable to get packet header");
    }

    id = ntohl(ph->packet_id);
    ret = nfq_get_payload(nfa, &buffer);

    ip4 = (struct ipv4_packet *) buffer;

    ret = nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL); 

    if (ret < 0) {
        printf("nfq_set_verdict to NF_ACCEPT failed\n");
    } else {
        printf("accepted \n");
    }

    return 0;
}

static void eval_config_item(char const *token,
        char const *value, struct arg_configer *arg_conf)
{
    if (!strcmp(token, "local_port")) {
        strcpy(arg_conf->local_port, value); 
        printf("arg_conf->local_port: %s\n", arg_conf->local_port);
        return;
    }

    if (!strcmp(token, "rx_proxy_port")) {
        strcpy(arg_conf->rx_proxy_port, value);
        printf("arg_conf->rx_proxy_port: %s\n", arg_conf->rx_proxy_port);
        return;
    }

    if (!strcmp(token, "rx_proxy_ip")) {
        strcpy(arg_conf->rx_proxy_ip, value);
        printf("arg_conf->rx_proxy_ip: %s\n", arg_conf->rx_proxy_ip);
        return;
    }
}

