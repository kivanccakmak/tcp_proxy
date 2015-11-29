#include "queue.h"

/**
 * @brief queue 
 *
 * Queue thread initialized by boss_server().
 * Thread starts sleeps until link_receptor
 * wakes up by using conditional variable. 
 * Once wake up, passes buffered packets 
 * towards end destination.
 *
 * @param args  
 *
 */
void *queue_wait(void *args) {

    int nwrite = 0, byte = 0;
    int pack_cnt = 0;

    queue_t *que = (queue_t *) args;

    printf("in queue_wait()\n");
    printf("que: %p\n", que);
    printf("que->byte_capacity: %d\n", que->byte_capacity);
    printf("que->byte_count: %d\n", que->byte_count);

    // initially lock mutex
    pthread_mutex_lock(&que->lock);

    while (1) {
        printf("queue in cond_wait() \n");
        pthread_cond_wait(&que->cond, &que->lock);
        printf("***********************\n");
        printf("queue get signal \n");
        printf("***********************\n");
        printf("que->index: %d\n", que->index);
        for (pack_cnt = 0; pack_cnt <= que->index; pack_cnt++) {
            printf("pack_cnt: %d\n", pack_cnt);
            printf("message: %s\n", que->buffer[pack_cnt]);
            while (byte < BLOCKSIZE) {
                nwrite = write(que->sockfd, que->buffer[pack_cnt] + byte,
                        BLOCKSIZE - byte);
                if (nwrite < 0) {
                    perror("Error");
                    exit(1);
                } else if (nwrite > 0) {
                    byte += nwrite;
                }
            }
            sleep(1);
            que->last_seq++;
            byte = 0;
            nwrite = 0;
        }
        que->index = -1;
    }

    return NULL;
}

/**
 * @brief 
 * Queue struct initialization. 
 * Set socket file descriptor of 
 * queue to pass raw data towards
 * end-destination. Initialize
 * mutex, cond_variable and 
 * queue buffer.
 *
 * @param[in] dest_ip
 * @param[in] dest_port
 *
 * @return *queue_t 
 */
queue_t* queue_init(char *dest_ip, char *dest_port) {

    int conn_res, sockfd = 0;
    queue_t *que = (queue_t *) malloc( sizeof(queue_t) );

    // fill sockaddr_in to use in connect
    struct sockaddr_in server;

    que->byte_count = 0;
    que->byte_capacity = INIT_QUEUE_SIZE;
    que->buffer = (char **) malloc(que->byte_capacity);
    que->last_seq = NO_SEQ;
    que->index = NO_SEQ;

    server.sin_addr.s_addr = inet_addr(dest_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(dest_port) );

    printf("queue initializes socket fd \n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    printf("connectiong to end destination ... \n");
    conn_res = connect(sockfd, (struct sockaddr *)&server, 
            sizeof(server));

    if (conn_res < 0) {
        perror("connection failed. Error");
        exit(0);
    } else {
        printf("queue connected to end destination \n");
        que->sockfd = sockfd;
    }

    printf("queue mutex init \n");
    pthread_mutex_init(&que->lock, NULL);

    printf("queue cond_variable init \n");
    pthread_cond_init(&que->cond, NULL);

    return que;
}
