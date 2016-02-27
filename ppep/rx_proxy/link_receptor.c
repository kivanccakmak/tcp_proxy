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
    pthread_t id = pthread_self();
    pool_t *pl = (pool_t *) malloc(sizeof(pool_t));

    // get struct pointers at thread initialization 
    rx_args_t *rx_args = (rx_args_t *) args;
    pl = rx_args->pl;
    sockfd = rx_args->sockfd;

    pfd.fd = sockfd;
    pfd.events = POLLIN;
    
    printf("** in link_receptor() **\n");
    while (1) {
        // wait socket file descriptor to get packet
        /*printf("receptor waits to be polled \n");*/
        rv = poll(&pfd, 1, rx_args->poll_timeout); 
        while (recv_count < PACKET_SIZE){
            numbytes = recv(sockfd, raw_buf+recv_count,
                    PACKET_SIZE, 0);
            /*printf("numbytes: %d\n", numbytes);*/
            if (numbytes > 0) {
                recv_count += numbytes;
            } else {
                goto COMPLETE;
            }
        }
        add2queue(pl, raw_buf);
        *(raw_buf + recv_count + 1) = '\0';
        raw_buf = (unsigned char*) malloc(PACKET_SIZE);
	recv_count = 0;
    }
COMPLETE:
    printf("** last packet **\n");
    printf("********************\n");
    printf("********************\n");
    printf("********************\n");
    if ((int) sizeof(raw_buf) > 0) {
        printf("bigger > 0\n");
        add2queue(pl, raw_buf);
    }
    pthread_exit(&id);
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
    /*printf("in add2queue\n");*/
    bool delay = false;
    encaps_packet_t *packet;
    packet = (encaps_packet_t *) raw_packet;

    bool nudge = false;
    node_t *ns = (node_t *) malloc(sizeof(node_t));

    /*printf("packet->raw_packet: %s\n", packet->raw_packet);*/
    /*printf("packet->seq: %d\n", packet->seq);*/
    /*printf("sizeof(packet->raw_packet): %d\n",*/
            /*(int) sizeof(packet->raw_packet));*/
    ns->pri = (-1 * packet->seq) - 1; 
    ns->raw_packet = packet->raw_packet;
    
    // now lock queue and insert data
    pthread_mutex_lock(&pl->lock);

    pqueue_insert(pl->pq, ns);
    if (pl->sent_min_seq + 1 == packet->seq) {
        nudge = true;
    } else
        nudge = false;

    if (pl->avail_min_seq + 1 == packet->seq)
        pl->avail_min_seq += 1;

    if (packet->seq - pl->avail_min_seq > DELAY_LIM)
        delay = true;

    // if expected seq_number arrived, nudge
    // forward module
    if (nudge == true) {
        /*printf("*sending cond_signal* \n");*/
        pthread_mutex_unlock(&pl->lock);
        pthread_cond_signal(&pl->cond);
        /*printf("*after sending cond_signal*\n");*/
    }else {
        pthread_mutex_unlock(&pl->lock);
    }
    if (delay)
        sleep(1);
}
