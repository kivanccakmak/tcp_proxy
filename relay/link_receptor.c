#include "link_receptor.h"

/**
 * @brief 
 *
 * TCP data receiver thread started by boss_server. 
 * Whenever new TCP connections initiated from
 * transmitter side of proxy pair, boss_server
 * initiates this function and this function 
 * just continously receives encapsulated TCP packets.
 * Consequently, this function orders to fill 
 * packet_pool by using push2pool(). 
 *
 * @param args
 *
 * @return 
 */
void *rx_chain(void *args){

    int rv, numbytes = 0;
    int sockfd;
    unsigned long recv_count = 0;
    unsigned char *raw_buf = 
        (unsigned char*) malloc(PACKET_SIZE);
    struct pollfd pfd;

    // get struct pointers at thread initialization 
    cb_rx_args_t *cb_args = (cb_rx_args_t *) args;
    struct packet_pool *pool = cb_args->pool;
    sockfd = cb_args->sockfd;

    pfd.fd = sockfd;
    pfd.events = POLLIN;

    while (1) {
        // wait socket file descriptor to get packet
        rv = poll(&pfd, 1, cb_args->poll_timeout); 
        while (recv_count < PACKET_SIZE){
            numbytes = recv(sockfd, raw_buf+recv_count, 1, 0);
            if (numbytes > 0) {
                recv_count += numbytes;
            }
            if (numbytes == -1) {
                perror("bug\n");
                break;
            } else if (numbytes == 0) {
                return NULL;
            }
            printf("recv_count: %ld\n", recv_count);
        }
        *(raw_buf + recv_count + 1) = '\0';
        pthread_mutex_lock(&pool->lock);
        push2pool((char *) raw_buf, pool);
        raw_buf = (unsigned char*) malloc(PACKET_SIZE);
    }
    return NULL;
}

/**
 * @brief 
 *
 * Adds packet to packet_pool by considering
 * common sequence number of packet, which 
 * is defined in encaps_packet format.
 * If sequentially expected packet arrived
 * to pool, wake reorder thread up with 
 * using nudge_queue()
 *
 * @param raw_packet
 * @param pool
 */
void push2pool(char *raw_packet, 
        struct packet_pool* pool) {
    
    printf("in push2pool\n");
    printf("pool: %p\n", pool);
    bool wake_pool;
    bool full_capacity;
    int boundary_diff;
    encaps_packet_t *packet;
    packet = (encaps_packet_t *) raw_packet;

    printf("seq: %d\n", packet->seq);
    printf("raw: %s\n", packet->raw_packet);

    // Increase capacity pool->sequential numbers is FULL 
    full_capacity = ((pool->capacity - pool->count) <  2);
    if (full_capacity) {
        pool->capacity = pool->capacity * 2 + 1;
        pool->sequential_nodes = 
            (char **) realloc(pool->sequential_nodes,
                sizeof(char*) * pool->capacity);
    }

    // Increase capacity if out-boundary packet arrives 
    boundary_diff = packet->seq - pool->capacity;
    if (boundary_diff > 0) {
        pool->capacity = 2 * pool->capacity + boundary_diff;
        pool->sequential_nodes = 
            (char **) realloc(pool->sequential_nodes,
                sizeof(char*) * pool->capacity);
    }

    // put packet into pool
    pool->sequential_nodes[packet->seq] = 
        (char *) packet->raw_packet;

    printf("pool->sequential_nodes[packet->seq]: %s\n",
            pool->sequential_nodes[packet->seq]);

    // if sequentially expected packet came, make pool thread 
    wake_pool = (packet->seq == (pool->queue_seq + 1));
    printf("wake_pool: %d\n", wake_pool ? 1 : 0);
    if (wake_pool) 
        pthread_cond_signal(&pool->cond);

    pthread_mutex_unlock(&pool->lock);
    
}
