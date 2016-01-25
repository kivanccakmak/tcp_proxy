#include "link_receptor.h"

static void add2queue(pqueue_t *pq, unsigned char *raw_packet);

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
    pthread_t id = pthread_self();

    // get struct pointers at thread initialization 
    cb_rx_args_t *cb_args = (cb_rx_args_t *) args;

    /*struct packet_pool *pool = cb_args->pool;*/
    pqueue_t *pq = (pqueue_t *) malloc(sizeof(pqueue_t)); 
    pq = cb_args->pq;
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
                pthread_exit(&id);
                return NULL;
            }
        }
        add2queue(pq, raw_buf);
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
 * @param[out] pq
 * @param[in] raw_packet
 */
static void add2queue(pqueue_t *pq, unsigned char *raw_packet)
{
    encaps_packet_t *packet;
    bool nudge = false;
    node_t *ns = (node_t *) malloc(sizeof(node_t));
    packet = (encaps_packet_t *) raw_packet;
    ns->pri = packet->seq;
    ns->raw_packet = packet->raw_packet;
    
    // now lock queue and insert data
    pthread_mutex_lock(&pq->lock);

    // TODO: extend queue size if full 
    
    pqueue_insert(pq, ns);
    if (pq->min_seq + 1 == (int) ns->pri) {
        pq->min_seq = (int) ns->pri;
        nudge = true;
    } else if ((int) ns->pri < pq->min_seq) {
        pq->min_seq = (int) ns->pri;
    } 

    // if expected seq_number arrived, nudge
    // forward module
    if (nudge == true) {
        pthread_cond_signal(&pq->cond);
    }
    pthread_mutex_unlock(&pq->lock);
}

