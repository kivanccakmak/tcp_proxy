#include "split.h"

#define QUEUE_NUM 0 

static int nfqueue_get_syn(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa,
        void *data);

static void* queuer_loop(void __attribute__((unused)) *unused);

static void* pass_payload(void *args);

static struct listen_args* set_listen_args(
        char *dest_ip, char *dest_port, char *local_part);

static int set_tx_sock(char *dest_ip, char *dest_port); 

static void add2buff(proxy_buff *buff, char *raw_buf); 

static struct split_args* set_split_args(char *local_port);

static void* get_payload(void *args); 

static struct cb_cntrl_args* set_controller_args(char *dest_ip, 
    char *dest_port, proxy_buff *buff); 

static void set_link(char *dest_ip, char *dest_port,
        struct link *tcp_link);

static void *run_controller(void *args);

static void *tx_chain(void *args); 

#ifdef TX_PROXY
int main(int argc, char *argv[])
{

    if (argc != 4) {
        printf("wrong usage \n");
        printf("%s local_port dest_ip dest_port\n", argv[0]);
        return 0;
    } 

    int ret;
    char *dest_ip, *dest_port, *local_port;
    struct split_args *split = NULL;
    struct cb_cntrl_args *cntrl_args = NULL;
    pthread_t queuer_id, getter_id, controller_id;

    local_port = argv[1];
    dest_ip = argv[2];
    dest_port = argv[3];

    split = set_split_args(local_port);
    cntrl_args = set_controller_args(dest_ip,
            dest_port, split->buff);

    ret = pthread_create(&queuer_id, NULL, &queuer_loop, NULL);
    if (ret) {
        printf("Failed to create queuer thread! [RET = %d]", ret);
        exit(0);
    } else{
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
    
    printf("before join commands\n");
    pthread_join(queuer_id, NULL);
    pthread_join(getter_id, NULL);
    pthread_join(controller_id, NULL);
    printf("all joined\n");
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
    int thr_res = 0;
    struct cb_cntrl_args *cntrl_args = NULL;
    struct link **tcp_links = NULL;
    struct link *temp_link = NULL;
    struct cb_tx_args **tx_args = NULL;
    char *dest_ip = (char *) malloc(sizeof(char*));;
    char *dest_port = (char *) malloc(sizeof(char*));

    cntrl_args = (struct cb_cntrl_args*)
        malloc(sizeof(struct cb_cntrl_args*));
    cntrl_args = (struct cb_cntrl_args*) args;

    tcp_links = (struct link**) 
        malloc(sizeof(struct link*) * cntrl_args->conn_number);
    tx_args = (struct cb_tx_args**)
        malloc(sizeof(struct cb_tx_args*) * cntrl_args->conn_number);

    for (i = 0; i < cntrl_args->conn_number; i++) {
        tcp_links[i] = (struct link*) malloc(sizeof(struct link));
        tx_args[i] = (struct cb_tx_args*) malloc(sizeof(struct cb_tx_args));
    }

    temp_link = (struct link*) malloc(sizeof(struct link*)); 
    dest_ip = cntrl_args->dest_ip;
    dest_port = cntrl_args->dest_port;
    printf("%s:%s\n", dest_ip,dest_port);

    printf("entering loop...\n");
    for (i = 0; i < cntrl_args->conn_number; i++) {
        printf("i: %d\n", i);
        pthread_t t_id;
        printf("rx_proxy: %s, rx_proxy_port: %s\n", dest_ip, dest_port);
        set_link(dest_ip, dest_port, tcp_links[i]);
        tx_args[i]->buff = cntrl_args->buff;
        tx_args[i]->tx_link = tcp_links[i];
        thr_res = pthread_create(&t_id, 
                NULL, &tx_chain, (void *) tx_args[i]);  
        pthread_join(t_id, NULL);
    }
    return NULL;
}


/**
 * @brief Sinlge TCP connection thread
 * that gets raw data from netfilter
 * queue and passes towards end destination
 *
 * @param args
 *
 * @return 
 */
static void *tx_chain(void *args) 
{
    printf("***\n");
    printf("in tx_chain()\n");
    printf("***\n");
    struct cb_tx_args *tx_args = NULL;
    encaps_packet_t packet;
    struct link *tx_link = NULL;
    proxy_buff *buff;
    int ind = 0, count = 0, send_res = 0;
    int fd;
    pthread_t id = pthread_self();

    tx_args = (struct cb_tx_args*)
        malloc(sizeof(struct cb_tx_args*));
    buff = (proxy_buff*) malloc(sizeof(proxy_buff));
    tx_link = (struct link*)
        malloc(sizeof(struct link*));

    tx_args = (struct cb_tx_args*) args;
    buff = tx_args->buff;
    tx_link = tx_args->tx_link;
    fd = tx_link->fd;
    while (1) {
        pthread_mutex_lock(&buff->lock);
        if (buff->set_ind >= buff->get_ind) {
            ind = buff->get_ind;
            memcpy(packet.raw_packet, buff->buffer[ind], BLOCKSIZE);
            packet.seq = (unsigned short) buff->get_ind;
            buff->get_ind++;
            pthread_mutex_unlock(&buff->lock);
            count = 0;
            while (count < (int) PACKET_SIZE) {
                printf("sending ...\n");
                send_res = send(fd, 
                    &((unsigned char*) &packet)[count],
                        PACKET_SIZE-count, 0);
                printf("send_res: %d\n", send_res);
                if (send_res > 0) {
                    count += send_res;
                } else{
                    printf("can't send\n");
                }
            }
        } else {
            pthread_mutex_unlock(&buff->lock);
        }  
        
        //check whether controller dictated to close
        //this connection
        pthread_mutex_lock(&tx_link->lock);
        if (tx_link->state == PASSIVE) {
            close(fd);
            tx_link->state = CLOSED;
            pthread_mutex_unlock(&tx_link->lock);
            pthread_exit(&id);
        }
        pthread_mutex_unlock(&tx_link->lock);
    }
    printf("out of while\n");
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
    int conn_res = 0, get_res = 0;
    socklen_t len = sizeof(server);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_addr.s_addr = inet_addr(dest_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(dest_port));

    printf("tcp link connecting ...\n");
    conn_res = connect(sockfd, (struct sockaddr*)&server,
            sizeof(server));
    printf("***\n");
    printf("conn_res: %d\n", conn_res);
    if (conn_res < 0) {
        perror("connection for tx_chain failed. Error");
        exit(0);
    } else {
        tcp_link->fd = sockfd;
    }

    printf("connected ... \n");
    get_res = 
        getsockname(sockfd, (struct sockaddr *)&server, &len);
    if (get_res == -1) {
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

static void *get_payload(void *args) 
{
    int i = 0;
    int rs_addr, numbytes = 0, recv_count = 0;
    int rv, yes = 1, set_val, bind_val, listen_val;
    struct addrinfo hints;
    struct addrinfo *addr = NULL;
    struct sockaddr_storage their_addr;
    struct pollfd pfd;
    struct split_args *split = NULL;
    char src_ip[INET6_ADDRSTRLEN];
    uint32_t src_port;
    bool exit_flag = false;
    proxy_buff *buff = NULL;
    char *local_port = NULL;
    socklen_t sin_size;
    int split_sock;
    char *raw_buf = (char*) malloc(BLOCKSIZE);
    printf("in get_payload\n");

    // get call back arguments
    split = (struct split_args*) 
        malloc(sizeof(struct split_args*));
    buff = (proxy_buff *) malloc(sizeof(proxy_buff*));
    split = (struct split_args*) args;
    local_port = split->local_port;
    buff = split->buff;

    // set local socket file descriptor 
    // to get data from nfqueue
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    printf("hints set\n");

    addr = (struct addrinfo*) 
        malloc(sizeof(struct addrinfo*));
    printf("local_port: %s\n", local_port);
    rs_addr = getaddrinfo(NULL, local_port, &hints, &addr);

    if (addr == NULL) {
        fprintf(stderr, "server: failed to bind \n");
        exit(1);
    } else 
        printf("addr set\n");

    printf("initiating socket ...\n");
    split_sock = socket(addr->ai_family, addr->ai_socktype,
            addr->ai_protocol);

    printf("after socket() \n");
    if (split_sock == -1) {
        perror("socket:");
        exit(0);
    } else
        printf("socket initiated\n");
    set_val = setsockopt(split_sock, SOL_SOCKET, 
            SO_REUSEADDR, &yes, sizeof(int));

    printf("binding to server\n");
    bind_val = bind(split_sock, addr->ai_addr, addr->ai_addrlen);
    if (bind_val == -1) {
        close(split_sock);
        perror("server: bind");
    }

    listen_val = listen(split_sock, 1);
    sin_size = sizeof(their_addr);

    while (1) {
LOOP:
        split_sock = accept(split_sock, (struct sockaddr*)&their_addr,
                &sin_size);

        pfd.fd = split_sock;
        pfd.events = POLLIN;

        inet_ntop(their_addr.ss_family,
                get_in_ipaddr((struct sockaddr*)&their_addr),
                src_ip, sizeof(src_ip));
        src_port = get_in_portnum((struct sockaddr*)&their_addr);

        printf("got connection from %s : %d \n", src_ip, src_port);
        while (1) {
            // poll() waits for file descriptor state change 
            rv = poll(&pfd, 1, -1);
            recv_count = 0;
            while ((recv_count < (int) BLOCKSIZE) 
                    & (exit_flag == false)) {
                numbytes = recv(split_sock, raw_buf+recv_count,
                        BLOCKSIZE-recv_count, 0);
                printf("numbytes: %d\n", numbytes);
                if (numbytes > 0)
                    recv_count += numbytes;
                else
                    exit_flag = true;
            }
            if (recv_count < BLOCKSIZE && recv_count > 0) {
                int count = 0;
                for (count = recv_count; count < BLOCKSIZE; count++) {
                    raw_buf[count] = '\0';
                }
                printf("raw_buf: %s\n", raw_buf);
                printf("sizeof(raw_buf): %d\n", (int) sizeof(raw_buf));
            }
            i++;
            if (recv_count > 0) {
                add2buff(buff, raw_buf);
            }
            raw_buf = (char *) malloc(BLOCKSIZE);
            if (exit_flag) {
                close(split_sock);
                goto LOOP;
            }
        }
    }
}

/**
 * @brief adds proxied payload to local 
 * buffer and updates count variables 
 * of proxy_buff pointer
 *
 * @param[out] buff
 * @param[in] raw_buf
 */
static void add2buff(proxy_buff *buff, char *raw_buf) 
{
    bool extend = false;
    int remained = 0;
    int i = 0;
    int pre_count = 0;

    pthread_mutex_lock(&buff->lock);

    remained = buff->capacity - buff->set_ind;
    extend = (remained) < (2 * (int) sizeof(char*));
    if (extend) {
        printf("in extend()\n");
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
    /*printf("===            === \n");*/
    buff->buffer[buff->set_ind] = raw_buf;
    /*printf("buff->buffer[%d]: %s\n", buff->set_ind,*/
            /*(char *) buff->buffer[buff->set_ind]);*/
    /*printf("===            === \n");*/
    buff->rx_byte += BLOCKSIZE;
    pthread_mutex_unlock(&buff->lock);
}

/**
 * @brief  
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
    } else{
        printf("accepted \n");
    }

    return 0;
}
