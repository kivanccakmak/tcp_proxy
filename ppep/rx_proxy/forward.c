#include "forward.h"

void *wait2forward(void *args) {
    int nwrite = 0, byte = 0;
    int pack_cnt = 0;

    fqueue_t *fq = (fqueue_t *) args;
    pool *pl = (pool *) args;

    pthread_mutex_lock(&fq->lock);

    while (1) {
        pthread_cond_wait(&fq->cond, &fq->lock);
        pthread_mutex_lock(&pl->lock);
        fill_queue(pl->pq, fq);
        fq->state = SEND;
        forward_data(fq);

    }
}

/**
 * @brief initializes forward queue 
 * which passes data through end
 * destination
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 *
 * @return  
 */
fqueue_t * fqueue_init(char *dest_ip, char *dest_port)
{
    int conn_res, sockfd = 0;
    struct sockaddr_in server;

    fqueue_t *fq = (fqueue_t *) malloc(sizeof(fqueue_t));
    fq->byte_count = 0;
    fq->byte_capacity = INIT_QUEUE_SIZE;
    fq->buffer = (char **) malloc(fq->byte_capacity);

    server.sin_addr.s_addr = inet_addr(dest_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(dest_port));

    printf("queue initializes sockfd\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    printf("connecting to end destination\n");
    conn_res = connect(sockfd, (struct sockaddr*)&server,
            sizeof(server));

    if (conn_res < 0) {
        perror("connection failed. Error");
        exit(0);
    } else {
        printf("queue conencted to end destination\n");
        fq->sockfd = sockfd;
    }

    pthread_mutex_init(&fq->lock, NULL);
    pthread_cond_init(&fq->cond, NULL);

    fq->state = SLEEP;

    return fq;
}
