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
    pool_t *pl;
    queue_args_t *que_args;
    pthread_t que_id, serv_id;

    que_args = (queue_args_t *) 
        malloc(sizeof(queue_args_t));

    server_port = argv[1];
    dest_ip = argv[2];
    dest_port = argv[3];

    pl = pool_init();

    que_args->pl = pl;
    que_args->dest_ip = dest_ip;
    que_args->dest_port = dest_port;
    
    pthread_create(&que_id, NULL, &wait2forward,
            (void*) que_args);
    sleep(3);
    
    server_listen(server_port, pl);

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
  * @brief 
  *
  * Listens from ip_addr:port and 
  * initiates new thread whenever 
  * new TCP connection established
  * from transmitter side of proxy.
  *
  * @param[in] server_port
  * @param[out] queue
  */
void server_listen(char* server_port, pool_t *pl)
{
    int sockfd, newfd;
    int rs_addr, yes = 1, thr_val;
    char tx_ip[INET6_ADDRSTRLEN];
    uint32_t tx_port;
    struct sigaction sig_sa;
    struct sockaddr_storage their_addr;
    struct addrinfo hints, *addr;
    socklen_t sin_size;
    rx_args_t *rx_args;

    // set server socket
    addr = (struct addrinfo*) malloc(sizeof(struct addrinfo));
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

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) != 0) {
        perror("setsockopt: ");
        exit(0);
    }

    if (bind(sockfd, addr->ai_addr, addr->ai_addrlen) == -1) {
        perror("server: bind");
        exit(0);
    }

    if (addr == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(0);
    }
    
    printf("-listen()\n");
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen: ");
        exit(0);
    }

    // initialize sigaction to wait connections
    sig_sa = sig_init();
    printf("server: waiting connections from: %s\n", server_port);
    sin_size = sizeof their_addr;
    
    // wait for new TCP connections and accept them
    while (1) {
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, 
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
        rx_args->poll_timeout = 100;

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


