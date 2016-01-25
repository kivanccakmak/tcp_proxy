#include "queue.h"

void *wait2forward(void *args) {
    int nwrite = 0, byte = 0;
    int pack_cnt = 0;
    int sockfd;
    node_t *n;

    pqueue_t *pq = 
        (pqueue_t *) malloc(sizeof(pqueue_t));

    pq = (pqueue_t *) args;

    //TODO: get pq and sockfd from struct

    while (1) {
        pthread_cond_wait(&pq->cond, &pq->lock);
        printf("****\n");
        printf("queue get signal\n");
        printf("****\n");
        pthread_mutex_lock(&pq->lock);
        n = (node_t *) pqueue_pop(pq);
        pthread_mutex_unlock(&pq->lock);
        
        // TODO: seperate while loop
        while (byte < BLOCKSIZE) {
            nwrite = write(sockfd, n->raw_packet + byte,
                    BLOCKSIZE - byte);
            if (nwrite < 0) {
                perror("Error");
                exit(1);
            } else if (nwrite > 0) {
                byte += nwrite;
            }
        }
        // TODO: pop next packet,
        // check whether next is orderd too
        // if soo, repeat sending process
    }

}
