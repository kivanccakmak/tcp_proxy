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

static controller_args* set_controller_args(char *dest_ip, 
    char *dest_port, proxy_buff *buff); 

static void set_link(char *dest_ip, char *dest_port,
        struct link *tcp_link);

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
    struct controller_args *cntrl_args = NULL;
    pthread_t queuer_id, getter_id, controller_id;

    local_port = argv[1];
    dest_ip = argv[2];
    dest_port = argv[3];

    split = set_split_args(local_port);
    cntrl_args = set_controller_args(dest_ip,
            dest_port, split->buff);

    ret = pthread_create(&queuer_id, NULL, queuer_loop, NULL);
    if (ret) {
        printf("Failed to create queuer thread! [RET = %d]", ret);
    }

    ret = pthread_create(&getter_id, NULL, &get_payload,
            (void *) split);
    if (ret) {
        printf("Failed to initiate payload getter. [RET = %d]", ret);
    }

    ret = pthread_create(&controller_id, NULL, &run_controller,
            (void *) cntrl_args);

    pthread_join(queuer_id, NULL);
    pthread_join(getter_id, NULL);
    pthread_join(controller_id, NULL);
    return 0;
}


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
    struct controller_args *cntrl_args = NULL;
    struct link *tcp_link = NULL;
    struct link *temp_link = NULL;
    struct link_control *controller = NULL;
    char *dest_ip = NULL;
    char *dest_port = NULL;

    cntrl_args = (struct controller_args*) args;
    tcp_link = (struct link*) malloc(sizeof(struct link*));
    temp_link = (struct link*) malloc(sizeof(struct link*));
    dest_ip = cntrl_args->dest_ip;
    dest_port = cntrl_args->dest_port;

    while (1) {
        for (i = 0; i < cntrl_args->conn_number; i++) {
            set_link(dest_ip, dest_port, tcp_link);
            if (controller->begin == NULL) { 
                controller->begin = tcp_link;
            }else if (controller->head == NULL) { 
                tcp_link->prev = controller->begin;
                controller->begin->next = tcp_link;
                controller->head = tcp_link;
            }else {
                tcp_link->prev = temp_link;
                temp_link->next = tcp_link;
                controller->head = tcp_link;
            }
            temp_link = tcp_link;
            pthread_t tx_id;
            sleep(1);
            pthread_create(&tx_id, NULL, &tx_chain, tcp_link);  
            tcp_link = (struct link*) malloc(sizeof(struct link*));
        }
        sleep(5);


        //
    }
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

    conn_res = connect(sockfd, (struct sockaddr*)&server,
            sizeof(server));
    if (conn_res > 0) {
        perror("connection failed. Error");
        exit(0);
    } else {
        tcp_link->fd = sockfd;
    }

    get_res = 
        getsockname(sockfd, (struct sockaddr *)&server, &len);
    if (get_res == -1) {
        perror("getsockname");
        exit(0);
    } else {
        tcp_link->src_port = ntohs(server.sin_port);
    }
}

static void *tx_chain(void *args)
{
    // state from link->state
    // get fd  
    // get proxy
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
static controller_args* set_controller_args(char *dest_ip,
        char *dest_port, proxy_buff *buff) 
{
    struct controller_args *cntrl_args = NULL;
    cntrl_args = (struct controller_args*)
        malloc(sizeof(struct controller_args*));

    cntrl_args->dest_ip = dest_ip;
    cntrl_args->dest_port = dest_port;
    cntrl_args->buff = buff;
    cntrl_args->conn_number = CONN_NUMBER:
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

    struct split_args *split = 
        (struct split_args*) malloc(sizeof(struct split_args*));
    split->local_port = local_port;

    proxy_buff *buff = 
        (proxy_buff*) malloc(sizeof(proxy_buff*));

    buff->rx_byte = 0;
    buff->tx_byte = 0;
    buff->set_ind = 0;
    buff->get_ind = 0;
    buff->capacity = INITIAL_CAPACITY;
    buff->buffer = (char **) 
        malloc(sizeof(char*) * buff->capacity);

    split->buff = buff;
    return split;
}

static void *queuer_loop(void __attribute__((unused)) *unused)
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	struct nfnl_handle *nh;

	int *sockfd, fd, i, ret;
	char buf[QUEUER_BUF_SIZE];
    ssize_t rc;
    char *dest_ip, *dest_port;

    dest_ip = (char *) malloc(20);
    dest_port = (char *) malloc(20);
    sockfd = (int *) malloc(sizeof(int*));

    strcpy(dest_ip, "192.168.2.202");
    strcpy(dest_port, "5050");
    /*set_conn(dest_ip, dest_port, sockfd);*/

    h = nfq_open();
    if (!h) {
        printf("Failed to open NFQ handler! \n");
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

    printf("binding this socket to queue '%d'", QUEUE_NUM);
    qh = nfq_create_queue(h, QUEUE_NUM, &nfqueue_get_syn, NULL);
    if (!qh) {
        printf("Failed to create nfnetlink queue!\n");
        exit(0);
    }

    printf("setting copy_packet mode");
    ret = nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);
    if (ret < 0) {
        printf("Failed to setup NFQ packet_copy mode! [RET = %d]\n", ret);
        exit(0);
    }

    nh = nfq_nfnlh(h);
    fd = nfnl_fd(nh);

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
    int rs_addr, numbytes = 0, recv_count = 0;
    int rv, yes = 1, set_val, bind_val, listen_val;
    struct addrinfo hints;
    struct addrinfo *addr;
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
    char *raw_buf = (char*) malloc(BUFF_SIZE);

    // get call back arguments
    split = (struct split_args*) 
        malloc(sizeof(struct split_args*));
    buff = (proxy_buff *) malloc(sizeof(proxy_buff*));
    local_port = split->local_port;
    split = (struct split_args*) args;
    buff = split->buff;

    // set local socket file descriptor 
    // to get data from nfqueue
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rs_addr = getaddrinfo(NULL, local_port, &hints, &addr);
    split_sock = socket(addr->ai_family, addr->ai_socktype,
            addr->ai_protocol);
    set_val = setsockopt(split_sock, SOL_SOCKET, 
            SO_REUSEADDR, &yes, sizeof(int));

    if (addr == NULL) {
        fprintf(stderr, "server: failed to bind \n");
        exit(1);
    }

    bind_val = bind(split_sock, addr->ai_addr, addr->ai_addrlen);
    if (bind_val == -1) {
        close(split_sock);
        perror("server: bind");
    }

    listen_val = listen(split_sock, 1);
    sin_size = sizeof(their_addr);

    while (1) {
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
            while ((recv_count < (int) BUFF_SIZE) 
                    & (exit_flag == false)) {
                numbytes = recv(split_sock, raw_buf+recv_count,
                        BUFF_SIZE-recv_count, 0);
                if (numbytes > 0)
                    recv_count += numbytes;
                else
                    exit_flag = true;
            }
            if (exit_flag) {
                close(split_sock);
                exit(0);
            }
            add2buff(buff, raw_buf);
            raw_buf = (char *) malloc(BUFF_SIZE);
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

    pthread_mutex_lock(&buff->lock);

    remained = buff->capacity - buff->set_ind;
    extend = (remained) < (2 * (int) sizeof(char*));
    if (extend) {
        buff->capacity = buff->capacity * 2;
        buff->buffer = (char **) realloc(buff->buffer,
                sizeof(char*) * buff->capacity);
    }
    buff->buffer[buff->set_ind] = raw_buf;
    buff->set_ind++;
    buff->rx_byte += BUFF_SIZE;

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
