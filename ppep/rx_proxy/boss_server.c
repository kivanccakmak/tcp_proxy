#include "boss_server.h"

static void set_hints(struct addrinfo* hints);

static void sigchld_handler(int s);

static void initialize_addr(char* port, struct addrinfo *addr);

static struct sigaction sig_init();

static void server_listen(char* server_port,  
        struct packet_pool *pool, queue_t *queue);

static void set_cb_rx_args(cb_rx_args_t *rx_args,
        int sockfd, queue_t *queue, struct packet_pool *pool,
        int poll_timeout); 

#ifdef RX_PROXY
int main(int argc, char * argv[])
{
    if (argc != 4){
        printf("usage: %s SERVER_PORT DEST_IP DEST_PORT  \n", argv[0]);
        printf("ex: %s 5050 192.168.3.3 4040 \n", argv[0]);
        return 0;
    }

    char *dest_ip, *dest_port, *server_port;
    int queue_thr, pool_thr;
    struct packet_pool *pool = NULL;

    pthread_t queue_id, pool_id;
    cb_reord_args_t *reord_args = (cb_reord_args_t *)
        malloc(sizeof(cb_reord_args_t*));

    server_port = argv[1];
    dest_ip = argv[2];
    dest_port = argv[3];

    printf("initialize queue \n");
    queue_t *queue = queue_init(dest_ip, dest_port);
    queue_thr = pthread_create(&queue_id, NULL, &queue_wait,
        (void *) queue);

    pool = packet_pool_init();
    reord_args->queue = queue;
    reord_args->pool = pool;
    pool_thr = pthread_create(&pool_id, NULL, &nudge_queue,
            (void *) reord_args);

    server_listen(server_port, pool, queue);

    pthread_join(queue_id, NULL);
    pthread_join(pool_id, NULL);

    return 0;
}
#endif

 /**
  * @brief 
  *
  * Listens from ip_addr:port and 
  * initiates new thread whenever 
  * new TCP connection established
  * from transmitter side of proxy.
  *
  * @param[in] server_port
  * @param pool
  * @param queue
  */
static void server_listen(char* server_port, 
        struct packet_pool *pool, queue_t *queue){

    int sockfd, newfd;
    int yes = 1;
    int set_val, bind_val, listen_val, thr_val;
    char tx_ip[INET6_ADDRSTRLEN];
    uint32_t tx_port;
    struct addrinfo *addr = NULL;
    struct sigaction sig_sa;
    struct sockaddr_storage their_addr;
    cb_rx_args_t *cb_args_ptr;

    initialize_addr(server_port, addr);
    sockfd = socket(addr->ai_family, 
            addr->ai_socktype, addr->ai_protocol);

    set_val = 
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int));

    bind_val = bind(sockfd, addr->ai_addr, addr->ai_addrlen);
    if(bind_val == -1){
        close(sockfd);
        perror("server: bind");
    }

    if (addr == NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    listen_val = listen(sockfd, BACKLOG);
    if (listen_val == -1){
        perror("listen");
        exit(1);
    }

    // initialize sigaction to wait connections
    sig_sa = sig_init();
    printf("server: waiting for connections\n");

    socklen_t sin_size;
    sin_size = sizeof their_addr;

    while(1){
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, 
                &sin_size);

        pthread_t thread_id;

        inet_ntop(their_addr.ss_family,
            get_in_ipaddr((struct sockaddr *)&their_addr),
                tx_ip, sizeof tx_ip);
        
        tx_port = get_in_portnum((struct sockaddr *)&their_addr);

        printf("server: got connection from %s : %d \n", 
                tx_ip, tx_port);

        cb_args_ptr = (cb_rx_args_t *) 
            malloc(sizeof(cb_rx_args_t));

        set_cb_rx_args(cb_args_ptr, newfd, queue,
                pool, -1);

        thr_val = pthread_create(&thread_id, NULL, &rx_chain, 
                cb_args_ptr);
        pthread_join(thread_id, NULL);

        if (thr_val < 0){
            perror("could not create thread");
        }
    }
}

/**
 * @brief 
 *
 * Set call-back arguments of
 * rx_chain() thread.
 *
 * @param[out] rx_args
 * @param[in] sockfd
 * @param[in] queue
 * @param[in] pool
 * @param[in] poll_timeout
 */
static void set_cb_rx_args(cb_rx_args_t *rx_args,
        int sockfd, queue_t *queue, struct packet_pool *pool,
        int poll_timeout) {
    
    rx_args->sockfd = sockfd;
    rx_args->poll_timeout = poll_timeout;
    rx_args->pool = pool;
    rx_args->queue = queue;

}

/**
 * @brief 
 *
 * Initialize sigaction to wait
 * incoming connections.
 *
 * @return 
 */
static struct sigaction sig_init(){
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
 * Fill addrinfo struct.
 *
 * @param[in] port
 * @param[out] addr
 */
static void initialize_addr(char* port, struct addrinfo *addr){
    struct addrinfo hints, *servinfo;
    int rs_addr;
    
    memset(&hints, 0, sizeof hints);
    set_hints(&hints);

    rs_addr = getaddrinfo(NULL, port, &hints, &servinfo);
    // this fills addr_info in linked list struct way 
    
    if (rs_addr != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rs_addr));
    }

    addr = servinfo;
    freeaddrinfo(servinfo);
}


/**
 * @brief 
 *
 * wait for a process to change
 * state
 *
 * @param s
 */
static void sigchld_handler(int s){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

/**
 * @brief 
 *
 * @param[out] hints
 */
static void set_hints(struct addrinfo* hints){
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;
}

