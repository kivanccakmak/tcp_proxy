#include "boss_server.h"

static void sigchld_handler(int s);

static struct sigaction sig_init();

#ifdef RX_PROXY
int main(int argc, char * argv[])
{
    if (argc != 4){
        printf("usage: %s SERVER_PORT DEST_IP DEST_PORT  \n", argv[0]);
        printf("ex: %s 5050 192.168.3.3 4040 \n", argv[0]);
        return 0;
    }

    char *dest_ip, *dest_port, *server_port;
    int rcv_sock, fwd_sock;
    fqueue_t *fq;
    pool_t *pl;
    queue_args_t *que_args;

    que_args = (queue_args_t *) 
        malloc(sizeof(queue_args_t));

    server_port = argv[1];
    dest_ip = argv[2];
    dest_port = argv[3];

    rcv_sock = rcv_sock_init(server_port);
    fwd_sock = fwd_sock_init(dest_ip, dest_port);

    pl = pool_init();
    fq = fqueue_init(fwd_sock);

    que_args->pl = pl;
    que_args->fq = fq;
    
    wait2forward(que_args);
    
    printf("to server listen\n");
    server_listen(rcv_sock, pl);

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

    // initialize priority queue

    pq = pqueue_init(10, cmp_pri, get_pri, set_pri,
            get_pos, set_pos);

    pl = (pool_t *) malloc(sizeof(pool_t));
    pthread_mutex_init(&pl->lock, NULL);
    pthread_cond_init(&pl->cond, NULL);
    pl->pq = pq;

    return pl;
}

/**
 * @brief initializes forward queue 
 * which passes data through end
 * destination
 *
 * @param[in] sockfd
 *
 * @return
 */
fqueue_t* fqueue_init(int sockfd)
{
    fqueue_t *fq = (fqueue_t *) malloc(sizeof(fqueue_t));
    fq->byte_count = 0;
    fq->byte_capacity = INIT_QUEUE_SIZE;

    fq->sockfd = sockfd;
    fq->state = SLEEP;

    return fq;
}

/**
 * @brief initiates server socket
 * file descriptor to accpet connections.
 * new socket file descriptors would be
 * created after accepting connections
 * from this socket
 *
 * @param[in] server_port
 *
 * @return 
 */
int rcv_sock_init(char *server_port) {
    int sockfd, rs_addr;
    int bind_val, set_val, yes = 1;
    struct addrinfo hints, *addr;

    addr = (struct addrinfo*) 
        malloc(sizeof(struct addrinfo));
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    rs_addr = getaddrinfo(NULL, server_port,
            &hints, &addr);
    if (rs_addr != 0) {
        perror("getaddrinfo:");
        exit(0);
    }

    sockfd = socket(addr->ai_family, 
            addr->ai_socktype, addr->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        exit(0);
    }

    set_val =
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int));
    printf("set_val: %d\n", set_val);
    bind_val = bind(sockfd, addr->ai_addr, addr->ai_addrlen);
    if(bind_val == -1){
        perror("server: bind");
        exit(0);
    } else {
        printf("socket binded\n");
    }

    if (addr == NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(0);
    } else {
        printf("addr set\n");
    }
    return sockfd;
} 

/**
 * @brief sets socket file descriptor 
 * to forward data through agnostic
 * end destination with forward module.
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 *
 * @return sockfd
 */
int fwd_sock_init(char *dest_ip, char *dest_port)
{
    int conn_res, sockfd = 0;
    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr(dest_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(dest_port));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    conn_res = connect(sockfd, (struct sockaddr*) &server,
            sizeof(server));

    if (conn_res < 0) {
        perror("connection failed.");
        exit(0);
    } else{
        return sockfd;
    }
}

 /**
  * @brief 
  *
  * Listens from ip_addr:port and 
  * initiates new thread whenever 
  * new TCP connection established
  * from transmitter side of proxy.
  *
  * @param[in] sockfd
  * @param[out] queue
  */
void server_listen(int rcv_sock, pool_t *pl)
{
    int newfd;
    int listen_val, thr_val;
    char tx_ip[INET6_ADDRSTRLEN];
    uint32_t tx_port;
    struct sigaction sig_sa;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    rx_args_t *rx_args;

    printf("-listen()\n");
    listen_val = listen(rcv_sock, BACKLOG);
    if (listen_val == -1){
        perror("listen");
        exit(1);
    }

    // initialize sigaction to wait connections
    sig_sa = sig_init();
    printf("server: waiting for connections\n");
    sin_size = sizeof their_addr;

    while (1) {
        newfd = accept(rcv_sock, (struct sockaddr *)&their_addr, 
                &sin_size);

        pthread_t thread_id;
        inet_ntop(their_addr.ss_family,
            get_in_ipaddr((struct sockaddr *)&their_addr),
                tx_ip, sizeof tx_ip);
        tx_port = get_in_portnum((struct sockaddr *)&their_addr);
        printf("server: got connection from %s : %d \n", 
                tx_ip, tx_port);
        
        rx_args = (rx_args_t *) malloc(sizeof(rx_args));
        rx_args->sockfd = newfd;
        rx_args->pl = pl;
        rx_args->poll_timeout = -1;

        thr_val = pthread_create(&thread_id, NULL, &rx_chain, 
                rx_args);
        pthread_join(thread_id, NULL);

        if (thr_val < 0){
            perror("could not create thread");
        }
    }
}

/**
 * @brief 
 *
 * Initialize sigaction to wait
 * incoming connections.
 *
 * @return 
 */
static struct sigaction sig_init()
{
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }
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


