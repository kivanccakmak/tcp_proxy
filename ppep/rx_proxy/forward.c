#include "forward.h"

static int fill_queue(pqueue_t *pq, fqueue_t *fq);

/**
 * @brief running queue thread, sleeps to get
 * signal, then gets packets from priority queue
 * and forwards through end-destination
 *
 * @param args
 *
 * @return 
 */
void *wait2forward(void *args) {
    int nwrite = 0, byte = 0;
    int pack_cnt = 0;

    fqueue_t *fq = (fqueue_t *) args;
    pool *pl = (pool *) args;

    pthread_mutex_lock(&fq->lock);

    while (1) {
        pthread_cond_wait(&fq->cond, &fq->lock);
        pthread_mutex_lock(&pl->lock);
        pack_cnt = fill_queue(pl->pq, fq);
        if (pack_cnt > 0) {
            fq->state = SEND;
            forward_data(fq->buffer, pack_cnt);
        } else {
            fq->state = SLEEP;
        }
    }
}

/**
 * @brief 
 *
 * @param pq
 * @param fq
 *
 * @return number of ready consecutive 
 * packets to forward 
 */
static int fill_queue(pqueue_t *pq, fqueue_t *fq)
{
    bool flag = true;
    int packets = 0;
    node_t *ns;

    ns = (node_t *) malloc(sizeof(node_t));
    ns = (node_t *) pqueue_pop(pq);

    if ( (int) ns->pri != fq->sent) {
        pqueue_insert(pq, ns);
        return packets;
    }else {
        packets += 1;
        fq->buffer[packets] = (char*) ns->raw_packet;
        while (flag == true) {
            ns = (node_t *) malloc(sizeof(node_t));
            ns = (node_t *) pqueue_pop(pq);
            if ( (int) ns->pri == fq->sent + packets) {
                fq->buffer[packets] = (char*) ns->raw_packet;
                packets += 1;
            } else {
                return packets;
            }
        }
    } 
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
    fq->buffer = (char **) malloc(fq->byte_capacity);

    fq->sockfd = sockfd;

    pthread_mutex_init(&fq->lock, NULL);
    pthread_cond_init(&fq->cond, NULL);

    fq->state = SLEEP;

    return fq;
}
