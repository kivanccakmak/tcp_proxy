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
    int pack_cnt = 0, conn_res = 0;
    int sockfd;
    bool send_flag = false;
    char *dest_ip, *dest_port;
    struct sockaddr_in server;
    fqueue_t *fq;
    pool_t *pl = NULL;
    queue_args_t* queue_args = NULL;

    queue_args = (queue_args_t*) args;
    dest_ip = queue_args->dest_ip;
    dest_port = queue_args->dest_port;
    
    // init receive socket
    server.sin_addr.s_addr = inet_addr(dest_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(dest_port));
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    conn_res = connect(sockfd, (struct sockaddr*) &server,
            sizeof(server));

    // init forward queue
    fq = (fqueue_t*) malloc(sizeof(fqueue_t));
    fq->byte_count = 0;
    fq->byte_capacity = INIT_QUEUE_SIZE;
    fq->sockfd = sockfd;
    fq->state = SLEEP;

    pl = queue_args->pl;

    pthread_mutex_lock(&pl->lock);

    while (1) {
        printf("queue in cond_wait()\n");
        pthread_cond_wait(&pl->cond, &pl->lock);
        printf("****\n");
        printf("queue get signal\n");
        printf("****\n");
        send_flag = true;
        while (send_flag == true) {
            pack_cnt = fill_queue(pl->pq, fq);
            printf("queue unlocks\n");
            pthread_mutex_unlock(&pl->lock);
            printf("queue unlocked\n");
            printf("pack_cnt: %d\n", pack_cnt);
            if (pack_cnt > 0) {
                fq->state = SEND;
                forward_data(fq, pack_cnt);
                pl->sent_min_seq += pack_cnt;
            } else {
                fq->state = SLEEP;
            }
            if (pl->avail_min_seq == pl->sent_min_seq) 
                send_flag = false;
            else
                send_flag = true;
            free(fq->buffer);
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
    printf("in forward_data()\n");

    for (count = 0; count < pack_cnt; count++) {
        while (byte < BLOCKSIZE) {
            nwrite = send(fq->sockfd, fq->buffer[pack_cnt],
                    BLOCKSIZE - byte, 0);
            if (nwrite < 0) {
                perror("Error in forward_data");
                exit(0);
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
    printf("in fill_queue()\n");
    bool flag = true;
    int packets = 0;
    int pack_num = 0;
    node_t *ns;
    
    // init buffer of queue
    fq->buffer = (char **) malloc(INIT_QUEUE_SIZE);

    ns = (node_t *) malloc(sizeof(node_t));
    ns = (node_t *) pqueue_pop(pq);
    pack_num = -1 * (int) ns->pri;
    printf("pack_num: %d\n", pack_num);
    printf("fq->sent: %d\n", fq->sent);

    if (pack_num != fq->sent + 1) {
        printf("won't send\n");
        pqueue_insert(pq, ns);
        return packets;
    }else {
        fq->buffer[packets] = (char*) ns->raw_packet;
        printf("-- adding --\n");
        packets += 1;
        while (flag == true) {
            ns = (node_t *) malloc(sizeof(node_t));
            ns = (node_t *) pqueue_pop(pq);
            if (fq->byte_capacity - fq->byte_count < 2 * BLOCKSIZE) {
                fq->byte_capacity = fq->byte_capacity * 2;
                fq->buffer = (char **) realloc(fq->buffer,
                        fq->byte_capacity);
            }
            pack_num = -1 * ns->pri;
            printf("*pack_num*: %d\n", pack_num);
            if ( pack_num == fq->sent + packets + 1) {
                printf("-- adding --\n");
                fq->buffer[packets] = (char*) ns->raw_packet;
                packets += 1;
                fq->byte_count += (int) sizeof(ns->raw_packet);
            } else {
                printf("-- no more add --\n");
                pqueue_insert(pq, ns);
                printf("-- blah --\n");
                flag = false;
            }
        }
        return packets;
    } 
}


