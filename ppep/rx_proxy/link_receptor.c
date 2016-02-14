#include "link_receptor.h"

static void add2queue(pool_t *pl, unsigned char *raw_packet);

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
    int rv, numbytes = 0;
    int sockfd;
    unsigned long recv_count = 0;
    unsigned char *raw_buf = 
        (unsigned char*) malloc(PACKET_SIZE);
    struct pollfd pfd;
    pool_t *pl = (pool_t *) malloc(sizeof(pool_t));
    pthread_t id = pthread_self();

    // get struct pointers at thread initialization 
    rx_args_t *rx_args = (rx_args_t *) args;
    pl = rx_args->pl;
    sockfd = rx_args->sockfd;

    pfd.fd = sockfd;
    pfd.events = POLLIN;
    
    printf("** in link_receptor() **\n");
    while (1) {
        // wait socket file descriptor to get packet
        printf("receptor waits to be polled \n");
        rv = poll(&pfd, 1, rx_args->poll_timeout); 

        while (recv_count < PACKET_SIZE){
            numbytes = recv(sockfd, raw_buf+recv_count, 1, 0);
            if (numbytes > 0) {
                recv_count += numbytes;
            }
            if (numbytes == -1) {
                perror("bug\n");
                break;
            } else if (numbytes == 0) {
                pthread_exit(&id);
                return NULL;
            }
        }
        add2queue(pl, raw_buf);
        *(raw_buf + recv_count + 1) = '\0';
        raw_buf = (unsigned char*) malloc(PACKET_SIZE);
	recv_count = 0;
    }
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
    printf("in add2queue\n");
    encaps_packet_t *packet;
    packet = (encaps_packet_t *) raw_packet;

    bool nudge = false;
    node_t *ns = (node_t *) malloc(sizeof(node_t));

    printf("packet->raw_packet: %s\n", packet->raw_packet);
    printf("packet->seq: %d\n", packet->seq);
    ns->pri = packet->seq;
    ns->raw_packet = packet->raw_packet;
    
    // now lock queue and insert data
    printf("receptor tries to lock\n");
    pthread_mutex_lock(&pl->lock);
    printf("receptor locked\n");

    pqueue_insert(pl->pq, ns);
    if (pl->pq->min_seq + 1 == (int) ns->pri) {
        pl->pq->min_seq = (int) ns->pri;
        nudge = true;
    } else if ((int) ns->pri < pl->pq->min_seq) {
        pl->pq->min_seq = (int) ns->pri;
    } 

    // if expected seq_number arrived, nudge
    // forward module
    if (nudge == true) {
        printf("sending cond_signal \n");
        pthread_mutex_unlock(&pl->lock);
        pthread_cond_signal(&pl->cond);
        printf("after sending cond_signal\n");
    }else {
        pthread_mutex_unlock(&pl->lock);
    }
}
