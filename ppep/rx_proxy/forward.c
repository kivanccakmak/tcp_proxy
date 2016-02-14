#include "forward.h"

static int fill_queue(pqueue_t *pq, fqueue_t *fq);

static void forward_data(fqueue_t* fq, int pack_cnt);

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
    int pack_cnt = 0;
    bool send_flag = false;
    queue_args_t* queue_args = NULL;
    fqueue_t *fq = NULL;
    pool_t *pl = NULL;

    queue_args = (queue_args_t*) args;
    fq = queue_args->fq;
    pl = queue_args->pl;

    printf("fq->byte: %d\n", fq->byte_capacity); 
    printf("trying to lock!\n");
    pthread_mutex_lock(&pl->lock);
    printf("======\n");
    printf("locked!\n");
    printf("======\n");
    sleep(5);

    while (1) {
        printf("queue in cond_wait()\n");
        pthread_cond_wait(&pl->cond, &pl->lock);
        sleep(1);
        printf("****\n");
        printf("queue get signal\n");
        printf("****\n");
        send_flag = true;
        while (send_flag == true) {
            pthread_mutex_lock(&pl->lock);
            pack_cnt = fill_queue(pl->pq, fq);
            pthread_mutex_unlock(&pl->lock);
            if (pack_cnt > 0) {
                fq->state = SEND;
                forward_data(fq, pack_cnt);
            } else {
                fq->state = SLEEP;
            }
            free(fq->buffer);
            pthread_mutex_lock(&pl->lock);
            pack_cnt = fill_queue(pl->pq, fq);
            if (pack_cnt == 0) {
                send_flag = false;
            }
            pthread_mutex_unlock(&pl->lock);
        }
    }
}

/**
 * @brief forward data towards agnostic
 * end destination, and update forward queue.
 *
 * @param fq
 * @param[in] pack_cnt
 */
static void forward_data(fqueue_t* fq, int pack_cnt)
{
    int byte = 0;
    int count = 0;
    int nwrite = 0;

    for (count = 0; count < pack_cnt; count++) {
        while (byte < BLOCKSIZE) {
            nwrite = write(fq->sockfd, fq->buffer[pack_cnt],
                    BLOCKSIZE - byte);
            if (nwrite < 0) {
                perror("Error");
                exit(1);
            } else if (nwrite > 0) {
                byte += nwrite;
            }
        }
        fq->sent += 1;
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
    
    // init buffer of queue
    fq->buffer = (char **) malloc(INIT_QUEUE_SIZE);

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
            if (fq->byte_capacity - fq->byte_count < 2 * BLOCKSIZE) {
                fq->byte_capacity = fq->byte_capacity * 2;
                fq->buffer = (char **) realloc(fq->buffer,
                        fq->byte_capacity);
            }
            if ( (int) ns->pri == fq->sent + packets) {
                fq->buffer[packets] = (char*) ns->raw_packet;
                packets += 1;
                fq->byte_count += (int) sizeof(ns->raw_packet);
            } else {
                pqueue_insert(pq, ns);
                flag = false;
            }
        }
        return packets;
    } 
}


