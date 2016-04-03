#include "link_receptor.h"

static void add2queue(pool_t *pl,
        unsigned char *raw_packet);

/**
 * @brief 
 *
 * TCP data receiver thread started by boss_server. 
 * Whenever new TCP connections initiated from
 * transmitter side of proxy pair, boss_server
 * initiates this function and this function 
 * just continously receives encapsulated TCP packets.
 * Consequently, this function orders to fill 
 * queue by using add2queue(). 
 *
 * @param args
 *
 * @return 
 */
void *rx_chain(void *args)
{
    int sockfd, numbytes = 0;
    unsigned long recv_count = 0;
    unsigned char *raw_buf = NULL; 
    struct pollfd pfd;
    pool_t *pl = (pool_t *) malloc(sizeof(pool_t));

    // get struct pointers at thread initialization 
    rx_args_t *rx_args = (rx_args_t *) args;
    pl = rx_args->pl;
    sockfd = rx_args->sockfd;

    pfd.fd = sockfd;
    pfd.events = POLLIN;
    while (true) {
        if (poll(&pfd, 1, 1) > 0) {
            recv_count = 0;
            numbytes = 0;
            raw_buf = (unsigned char*) malloc(PACKET_SIZE);
            while (recv_count < PACKET_SIZE){
                numbytes = recv(sockfd, raw_buf+recv_count,
                        PACKET_SIZE-recv_count, 0);
                if (numbytes > 0) {
                    recv_count += numbytes;
                } else if (numbytes == 0) {
                    goto COMPLETE;
                }
            }
            add2queue(pl, raw_buf);
        }
    }
COMPLETE:
    if (recv_count > 0) {
        add2queue(pl, raw_buf);
    }
    close(sockfd);
    return NULL;
}

/**
 * @brief adds packet into queue.
 * sends signal via conditional
 * variable, if next ordered packet
 * just arrived
 *
 * @param[out] pl
 * @param[in] raw_packet
 */
static void add2queue(pool_t *pl, unsigned char *raw_packet)
{
    encaps_packet_t *packet = NULL;
    packet = (encaps_packet_t *) raw_packet;

    node_t *ns = (node_t *) malloc(sizeof(node_t));
    ns->pri = packet->seq; 
    ns->raw_packet = packet->raw_packet;
    
    // lock queue and insert data
    pthread_mutex_lock(&pl->lock);
    pqueue_insert(pl->pq, ns);

    if (pl->avail_min_seq + 1 == packet->seq)
        pl->avail_min_seq += 1;

    if (pl->sent_min_seq + 1 == packet->seq) {
        pthread_mutex_unlock(&pl->lock);
        pthread_cond_signal(&pl->cond);
    } else {
        pthread_mutex_unlock(&pl->lock);
    }

}
